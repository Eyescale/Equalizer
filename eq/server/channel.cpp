
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "channel.h"

#include "channelListener.h"
#include "channelUpdateVisitor.h"
#include "compound.h"
#include "compoundVisitor.h"
#include "configVisitor.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "segment.h"
#include "view.h"
#include "window.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/statistic.h>
#include <eq/fabric/paths.h>

#include <co/objectICommand.h>

#include <lunchbox/debug.h>

#include <set>

namespace eq
{
namespace server
{
typedef co::CommandFunc<Channel> CmdFunc;
typedef fabric::Channel< Window, Channel > Super;

namespace
{
    typedef std::set< View* > ViewSet;

    class ViewFinder : public ConfigVisitor
    {
    public:
        explicit ViewFinder( const Channel* channel ) : _channel( channel ) {}
        virtual ~ViewFinder(){}

        virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( channel != _channel )
                return TRAVERSE_CONTINUE;

            channel = compound->getInheritChannel();
            if ( channel->getView( ))
                _viewSet.insert( channel->getView( ));
            return TRAVERSE_CONTINUE;
        }

        ViewSet& getViews() { return _viewSet; }

    private:
        const Channel*     _channel;
        ViewSet  _viewSet;
    };
}

Channel::Channel( Window* parent )
        : Super( parent )
        , _active( 0 )
        , _view( 0 )
        , _segment( 0 )
        , _state( STATE_STOPPED )
        , _lastDrawCompound( 0 )
        , _private( 0 )
{
    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getChannelIAttribute( attr ));
    }
    for( unsigned i = 0; i < SATTR_ALL; ++i )
    {
        const SAttribute attr = static_cast< SAttribute >( i );
        setSAttribute( attr, global->getChannelSAttribute( attr ));
    }
}

Channel::Channel( const Channel& from )
        : Super( from )
        , _active( 0 )
        , _view( 0 )
        , _segment( 0 )
        , _state( STATE_STOPPED )
        , _lastDrawCompound( 0 )
        , _private( 0 )
{
    // Don't copy view and segment. Will be re-set by segment copy ctor
}

void Channel::attach( const uint128_t& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* mainQ  = getMainThreadQueue();
    co::CommandQueue* cmdQ = getCommandThreadQueue();

    registerCommand( fabric::CMD_CHANNEL_CONFIG_INIT_REPLY,
                     CmdFunc( this, &Channel::_cmdConfigInitReply ), cmdQ );
    registerCommand( fabric::CMD_CHANNEL_CONFIG_EXIT_REPLY,
                     CmdFunc( this, &Channel::_cmdConfigExitReply ), cmdQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_FINISH_REPLY,
                     CmdFunc( this, &Channel::_cmdFrameFinishReply ), mainQ );
}

Channel::~Channel()
{
}

void Channel::postDelete()
{
    // Deregister server-queue command handler to avoid assertion in
    // command invokation after channel deletion
    registerCommand( fabric::CMD_CHANNEL_FRAME_FINISH_REPLY,
                     CmdFunc( this, &Channel::_cmdNop ), 0 );
}

Config* Channel::getConfig()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window ? window->getConfig() : 0;
}

const Config* Channel::getConfig() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return window ? window->getConfig() : 0;
}

Node* Channel::getNode()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window ? window->getNode() : 0;
}

const Node* Channel::getNode() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return window ? window->getNode() : 0;
}

Pipe* Channel::getPipe()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window ? window->getPipe() : 0;
}

const Pipe* Channel::getPipe() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return window ? window->getPipe() : 0;
}

ServerPtr Channel::getServer()
{
    Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getServer() : 0 );
}

const Canvas* Channel::getCanvas() const
{
    if( !_segment )
        return 0;
    return _segment->getCanvas();
}

const Compounds& Channel::getCompounds() const
{
    return getConfig()->getCompounds();
}

co::CommandQueue* Channel::getMainThreadQueue()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window->getMainThreadQueue();
}

co::CommandQueue* Channel::getCommandThreadQueue()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window->getCommandThreadQueue();
}

bool Channel::supportsView( const View* view ) const
{
    if( !view )
        return true;

    const uint64_t minimum = view->getMinimumCapabilities();
    const uint64_t supported = getCapabilities();
    return( (supported & minimum) == minimum );
}

void Channel::activate()
{
    Window* window = getWindow();
    LBASSERT( window );

    ++_active;
    window->activate();

    LBLOG( LOG_VIEW ) << "activate: " << _active << " " << (void*)this
                      << std::endl;
}

void Channel::deactivate()
{
    Window* window = getWindow();
    LBASSERT( _active != 0 );
    LBASSERT( window );

    --_active;
    window->deactivate();

    LBLOG( LOG_VIEW ) << "deactivate: " << _active << " " << (void*)this
                      << std::endl;
}

void Channel::setOutput( View* view, Segment* segment )
{
    if( _view == view && _segment == segment )
        return;

    LBASSERT( !_view && !_segment );
    LBASSERT( view && segment );

    _view = view;
    _segment = segment;

    view->addChannel( this );
    segment->addDestinationChannel( this );

    co::ObjectVersion viewVersion( view );
    if( view && view->isDirty( ))
        ++viewVersion.version;

    setViewVersion( viewVersion );
}

void Channel::unsetOutput()
{
    LBASSERT( _view && _segment );

    LBCHECK( _view->removeChannel( this ));
    LBCHECK( _segment->removeDestinationChannel( this ));

    _view    = 0;
    _segment = 0;
}

const Layout* Channel::getLayout() const
{
    LBASSERT( _view );
    return _view ? _view->getLayout() : 0;
}

void Channel::addTasks( const uint32_t tasks )
{
    Window* window = getWindow();
    LBASSERT( window );
    setTasks( getTasks() | tasks );
    window->addTasks( tasks );
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Channel::configInit( const uint128_t& initID, const uint32_t /*frame*/ )
{
    LBASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    LBLOG( LOG_INIT ) << "Init channel" << std::endl;
    getWindow()->send( fabric::CMD_WINDOW_CREATE_CHANNEL ) << getID();
    send( fabric::CMD_CHANNEL_CONFIG_INIT ) << initID;
}

bool Channel::syncConfigInit()
{
    LBASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_INIT_SUCCESS )
    {
        _state = STATE_RUNNING;
        return true;
    }

    configExit();
    return false;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Channel::configExit()
{
    LBASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    LBLOG( LOG_INIT ) << "Exit channel" << std::endl;
    send( fabric::CMD_CHANNEL_CONFIG_EXIT );
}

bool Channel::syncConfigExit()
{
    LBASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS ||
              _state == STATE_EXIT_FAILED );

    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    LBASSERT( success || _state == STATE_EXIT_FAILED );

    _state = isActive() ? STATE_FAILED : STATE_STOPPED;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Channel::_setupRenderContext( const uint128_t& frameID,
                                   RenderContext& context )
{
    context.frameID = frameID;
    context.pvp = getPixelViewport();
    context.view = _view;
    context.vp = getViewport();
    LBASSERTINFO( getNativeContext().view == context.view,
                  getNativeContext().view << " != " << context.view << " " <<
                  getName( ));
}

bool Channel::update( const uint128_t& frameID, const uint32_t frameNumber )
{
    if( !isRunning( ))
        return false; // not updated

    LBASSERT( isActive( ))
    LBASSERT( getWindow()->isActive( ));

    RenderContext context;
    _setupRenderContext( frameID, context );
    send( fabric::CMD_CHANNEL_FRAME_START )
            << context << getVersion() << frameNumber;
    LBLOG( LOG_TASKS ) << "TASK channel " << getName() << " start frame  "
                       << frameNumber << std::endl;

    bool updated = false;
    const Compounds& compounds = getCompounds();
    for( Compounds::const_iterator i = compounds.begin();
         i != compounds.end(); ++i )
    {
        const Compound* compound = *i;
        ChannelUpdateVisitor visitor( this, frameID, frameNumber );

        visitor.setEye( EYE_CYCLOP );
        compound->accept( visitor );

        visitor.setEye( EYE_LEFT );
        compound->accept( visitor );

        visitor.setEye( EYE_RIGHT );
        compound->accept( visitor );

        updated |= visitor.isUpdated();
    }

    send( fabric::CMD_CHANNEL_FRAME_FINISH ) << context << frameNumber;
    LBLOG( LOG_TASKS ) << "TASK channel " << getName() << " finish frame  "
                           << frameNumber << std::endl;
    _lastDrawCompound = 0;

    return updated;
}

co::ObjectOCommand Channel::send( const uint32_t cmd )
{
    return getNode()->send( cmd, getID( ));
}

//---------------------------------------------------------------------------
// Listener interface
//---------------------------------------------------------------------------
void Channel::addListener( ChannelListener* listener )
{
    LB_TS_SCOPED( _serverThread );
    LBASSERT( std::find( _listeners.begin(), _listeners.end(), listener ) ==
              _listeners.end( ));

    _listeners.push_back( listener );
}

void Channel::removeListener(  ChannelListener* listener )
{
    ChannelListeners::iterator i = find( _listeners.begin(), _listeners.end(),
                                          listener );
    if( i != _listeners.end( ))
        _listeners.erase( i );
}

void Channel::_fireLoadData( const uint32_t frameNumber,
                             const fabric::Statistics& statistics,
                             const Viewport& region )
{
    LB_TS_SCOPED( _serverThread );
    for( ChannelListener* listener : _listeners )
        listener->notifyLoadData( this, frameNumber, statistics, region );
}

//===========================================================================
// command handling
//===========================================================================
bool Channel::_cmdConfigInitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const bool result = command.read< bool >();

    LBLOG( LOG_INIT ) << "handle channel configInit reply " << command
                      << " result " << result << std::endl;

    _state = result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Channel::_cmdConfigExitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBLOG( LOG_INIT ) << "handle channel configExit reply " << command
                      << std::endl;

    _state = command.read< bool >() ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return true;
}

bool Channel::_cmdFrameFinishReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const Viewport& region = command.read< Viewport >();
    const uint32_t frameNumber = command.read< uint32_t >();
    const Statistics& statistics = command.read< Statistics >();

    _fireLoadData( frameNumber, statistics, region );
    return true;
}

bool Channel::omitOutput() const
{
    // don't print generated channels for now
    return _view || _segment;
}

void Channel::output( std::ostream& os ) const
{
    bool attrPrinted   = false;

    for( IAttribute i = static_cast<IAttribute>( 0 );
         i < IATTR_LAST;
         i = static_cast<IAttribute>( static_cast<uint32_t>(i)+1 ))
    {
        const int value = getIAttribute( i );
        if( value == Global::instance()->getChannelIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl
               << "{" << std::endl << lunchbox::indent;
            attrPrinted = true;
        }

        os << ( i==IATTR_HINT_STATISTICS ? "hint_statistics   " :
                i==IATTR_HINT_SENDTOKEN ?  "hint_sendtoken    " :
                                           "ERROR " )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    for( SAttribute i = static_cast<SAttribute>( 0 );
         i < SATTR_LAST;
         i = static_cast<SAttribute>( static_cast<uint32_t>(i)+1 ))
    {
        const std::string& value = getSAttribute( i );
        if( value == Global::instance()->getChannelSAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl
               << "{" << std::endl << lunchbox::indent;
            attrPrinted = true;
        }

        os << ( i == SATTR_DUMP_IMAGE ? "dump_image        " : "ERROR " )
           << "\"" << value << "\"" << std::endl;
    }

    if( attrPrinted )
        os << lunchbox::exdent << "}" << std::endl << std::endl;
}

void Channel::updateCapabilities()
{
    ViewFinder visitor( this );
    getConfig()->accept( visitor );
    ViewSet& views = visitor.getViews();

    for( ViewSet::const_iterator i = views.begin(); i != views.end(); ++i )
        (*i)->updateCapabilities();
}

}
}

#include "../fabric/channel.ipp"
template class eq::fabric::Channel< eq::server::Window, eq::server::Channel >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */
