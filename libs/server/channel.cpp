
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/channelPackets.h>
#include <eq/log.h>
#include <eq/windowPackets.h>
#include <eq/fabric/paths.h>
#include <co/command.h>
#include <co/base/os.h>
#include <co/base/debug.h>

#include "channel.ipp"

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
        ViewFinder( const Channel* channel ) : _channel( channel ) {}
        virtual ~ViewFinder(){}

        virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( channel != _channel )
                return TRAVERSE_CONTINUE;

            channel = compound->getInheritChannel();
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
{
    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getChannelIAttribute( attr ));
    }
}

Channel::Channel( const Channel& from )
        : Super( from )
        , _active( 0 )
        , _view( 0 )
        , _segment( 0 )
        , _state( STATE_STOPPED )
        , _lastDrawCompound( 0 )
{
    // Don't copy view and segment. Will be re-set by segment copy ctor
}

void Channel::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    
    co::CommandQueue* serverQ  = getMainThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();

    registerCommand( fabric::CMD_CHANNEL_CONFIG_INIT_REPLY, 
                     CmdFunc( this, &Channel::_cmdConfigInitReply ), commandQ );
    registerCommand( fabric::CMD_CHANNEL_CONFIG_EXIT_REPLY,
                     CmdFunc( this, &Channel::_cmdConfigExitReply ), commandQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_FINISH_REPLY,
                     CmdFunc( this, &Channel::_cmdFrameFinishReply ), serverQ );
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
    EQASSERT( window );
    return window ? window->getConfig() : 0; 
}

const Config* Channel::getConfig() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    return window ? window->getConfig() : 0;
}

Node* Channel::getNode() 
{ 
    Window* window = getWindow();
    EQASSERT( window );
    return window ? window->getNode() : 0;
}

const Node* Channel::getNode() const
{ 
    const Window* window = getWindow();
    EQASSERT( window );
    return window ? window->getNode() : 0;
}

Pipe* Channel::getPipe() 
{ 
    Window* window = getWindow();
    EQASSERT( window );
    return window ? window->getPipe() : 0;
}

const Pipe* Channel::getPipe() const
{ 
    const Window* window = getWindow();
    EQASSERT( window );
    return window ? window->getPipe() : 0;
}

ServerPtr Channel::getServer()
{
    Window* window = getWindow();
    EQASSERT( window );
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
    EQASSERT( window );
    return window->getMainThreadQueue(); 
}

co::CommandQueue* Channel::getCommandThreadQueue()
{
    Window* window = getWindow();
    EQASSERT( window );
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
    EQASSERT( window );

    ++_active;
    window->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Channel::deactivate()
{ 
    Window* window = getWindow();
    EQASSERT( _active != 0 );
    EQASSERT( window );

    --_active; 
    window->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
}

void Channel::setOutput( View* view, Segment* segment )
{
    EQASSERT( !_view && !_segment );
    EQASSERT( view && segment );

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
    EQASSERT( _view && _segment );

    EQCHECK( _view->removeChannel( this ));
    EQCHECK( _segment->removeDestinationChannel( this ));

    _view    = 0;
    _segment = 0;
}

const Layout* Channel::getLayout() const
{
    EQASSERT( _view );
    return _view ? _view->getLayout() : 0;
}

void Channel::addTasks( const uint32_t tasks )
{
    Window* window = getWindow();
    EQASSERT( window );
    setTasks( getTasks() | tasks );
    window->addTasks( tasks );
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Channel::configInit( const uint128_t& initID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    WindowCreateChannelPacket createChannelPacket;
    createChannelPacket.channelID = getID();
    getWindow()->send( createChannelPacket );

    ChannelConfigInitPacket packet;
    packet.initID = initID;
    
    EQLOG( LOG_INIT ) << "Init channel" << std::endl;
    send( packet );
}

bool Channel::syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_INIT_SUCCESS )
    {
        _state = STATE_RUNNING;
        return true;
    }

    sync();
    EQWARN << "Channel initialization failed: " << getError() << std::endl;
    configExit();
    return false;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Channel::configExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit channel" << std::endl;
    ChannelConfigExitPacket packet;
    send( packet );
}

bool Channel::syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

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
    EQASSERTINFO( getNativeContext().view == context.view, 
                  getNativeContext().view << " != " << context.view << " " <<
                  getName( ));
}

bool Channel::update( const uint128_t& frameID, const uint32_t frameNumber )
{
    EQASSERT( isActive( ));
    EQASSERT( isRunning( ));
    EQASSERT( getWindow()->isActive( ));

    ChannelFrameStartPacket startPacket;
    startPacket.frameNumber = frameNumber;
    startPacket.version     = getVersion();
    _setupRenderContext( frameID, startPacket.context );

    send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK channel " << getName() << " start frame  " 
                           << &startPacket << std::endl;

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

    ChannelFrameFinishPacket finishPacket;
    finishPacket.frameNumber = frameNumber;
    finishPacket.context = startPacket.context;

    send( finishPacket );
    EQLOG( LOG_TASKS ) << "TASK channel " << getName() << " finish frame  "
                           << &finishPacket << std::endl;
    _lastDrawCompound = 0;

    return updated;
}

void Channel::send( co::ObjectPacket& packet ) 
{ 
    packet.objectID = getID();
    getNode()->send( packet );
}

//---------------------------------------------------------------------------
// Listener interface
//---------------------------------------------------------------------------
void Channel::addListener( ChannelListener* listener )
{
    EQ_TS_SCOPED( _serverThread );
    EQASSERT( std::find( _listeners.begin(), _listeners.end(), listener ) ==
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
                             const uint32_t nStatistics,
                             const Statistic* statistics )
{
    EQ_TS_SCOPED( _serverThread );

    for( ChannelListeners::const_iterator i = _listeners.begin(); 
         i != _listeners.end(); ++i )
    {
        (*i)->notifyLoadData( this, frameNumber, nStatistics, statistics );
    }
}

//===========================================================================
// command handling
//===========================================================================
bool Channel::_cmdConfigInitReply( co::Command& command ) 
{
    const ChannelConfigInitReplyPacket* packet = 
        command.getPacket<ChannelConfigInitReplyPacket>();
    EQLOG( LOG_INIT ) << "handle channel configInit reply " << packet
                      << std::endl;

    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Channel::_cmdConfigExitReply( co::Command& command ) 
{
    const ChannelConfigExitReplyPacket* packet = 
        command.getPacket<ChannelConfigExitReplyPacket>();
    EQLOG( LOG_INIT ) << "handle channel configExit reply " << packet
                      << std::endl;

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return true;
}

bool Channel::_cmdFrameFinishReply( co::Command& command )
{
    const ChannelFrameFinishReplyPacket* packet = 
        command.getPacket<ChannelFrameFinishReplyPacket>();

    _fireLoadData( packet->frameNumber, packet->nStatistics,
                   packet->statistics );
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
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i==IATTR_HINT_STATISTICS ?
                "hint_statistics   " :
                i==IATTR_HINT_SENDTOKEN ?
                    "hint_sendtoken    " : "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << co::base::exdent << "}" << std::endl << std::endl;
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
