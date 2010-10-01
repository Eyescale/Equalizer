
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

#include <pthread.h>
#include "channel.h"

#include "channelListener.h"
#include "channelUpdateVisitor.h"
#include "compound.h"
#include "compoundVisitor.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "segment.h"
#include "view.h"
#include "window.h"

#include <eq/client/log.h>
#include <eq/client/packets.h>
#include <eq/fabric/paths.h>
#include <eq/net/command.h>
#include <eq/base/base.h>
#include <eq/base/debug.h>

#include "channel.ipp"

namespace eq
{
namespace server
{
typedef net::CommandFunc<Channel> CmdFunc;
typedef fabric::Channel< Window, Channel > Super;

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

void Channel::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    Super::attachToSession( id, instanceID, session );
    
    net::CommandQueue* serverQ  = getMainThreadQueue();
    net::CommandQueue* commandQ = getCommandThreadQueue();

    registerCommand( fabric::CMD_CHANNEL_CONFIG_INIT_REPLY, 
                     CmdFunc( this, &Channel::_cmdConfigInitReply ), commandQ );
    registerCommand( fabric::CMD_CHANNEL_CONFIG_EXIT_REPLY,
                     CmdFunc( this, &Channel::_cmdConfigExitReply ), commandQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_FINISH_REPLY,
                     CmdFunc( this, &Channel::_cmdFrameFinishReply ), serverQ );
}

Channel::~Channel()
{
    EQINFO << "Delete channel @" << (void*)this << std::endl;
}

void Channel::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );
    EQASSERT( isMaster( ));
    setDirty( dirtyBits ); // redistribute slave changes
}

void Channel::postDelete()
{
    //_state |= STATE_DELETE;
    //getConfig()->postNeedsFinish();

    // Deregister server-queue command handler to avoid assertion in
    // Session::invokeCommand after channel deletion
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

net::CommandQueue* Channel::getMainThreadQueue()
{
    Window* window = getWindow();
    EQASSERT( window );
    return window->getMainThreadQueue(); 
}

net::CommandQueue* Channel::getCommandThreadQueue()
{
    Window* window = getWindow();
    EQASSERT( window );
    return window->getCommandThreadQueue(); 
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

    _view    = view;
    _segment = segment;

    view->addChannel( this );
    segment->addDestinationChannel( this );
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
void Channel::configInit( const uint32_t initID, const uint32_t frameNumber )
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
    EQWARN << "Channel initialization failed: " << getErrorMessage()
           << std::endl;
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
void Channel::_setupRenderContext( const uint32_t frameID, 
                                   RenderContext& context )
{
    context.frameID       = frameID;
    context.pvp           = getPixelViewport();
    context.view          = _view;
    context.vp            = getViewport();
    EQASSERTINFO( getNativeContext().view == context.view, 
                  getNativeContext().view << " != " << context.view );
}

bool Channel::update( const uint32_t frameID, const uint32_t frameNumber )
{
    EQASSERT( isActive( ));
    EQASSERT( getWindow()->isActive( ));
    EQASSERT( _state == STATE_RUNNING );

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

void Channel::send( net::ObjectPacket& packet ) 
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
bool Channel::_cmdConfigInitReply( net::Command& command ) 
{
    const ChannelConfigInitReplyPacket* packet = 
        command.getPacket<ChannelConfigInitReplyPacket>();
    EQLOG( LOG_INIT ) << "handle channel configInit reply " << packet
                      << std::endl;

    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Channel::_cmdConfigExitReply( net::Command& command ) 
{
    const ChannelConfigExitReplyPacket* packet = 
        command.getPacket<ChannelConfigExitReplyPacket>();
    EQLOG( LOG_INIT ) << "handle channel configExit reply " << packet
                      << std::endl;

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return true;
}

bool Channel::_cmdFrameFinishReply( net::Command& command )
{
    const ChannelFrameFinishReplyPacket* packet = 
        command.getPacket<ChannelFrameFinishReplyPacket>();

    _fireLoadData( packet->frameNumber, packet->nStatistics,
                   packet->statistics );
    return true;
}

std::ostream& operator << ( std::ostream& os, const Channel& channel)
{
    os << base::disableFlush << base::disableHeader << "channel" << std::endl;
    os << "{" << std::endl << base::indent;

    const std::string& name = channel.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Segment* segment = channel.getSegment();
    const View*    view    = channel.getView();
    if( view && segment )
    {
        os << "path     ( ";

        const Config* config = channel.getConfig();
        const std::string& segmentName = segment->getName();
        if( !segmentName.empty() && 
            config->find< Segment >( segmentName ) == segment )
        {
            os << "segment \"" << segmentName << "\" ";
        }
        else
            os << segment->getPath() << ' ';
        
        const std::string& viewName = view->getName();
        if( !viewName.empty() && 
            config->find< View >( viewName ) == view )
        {
            os << "view \"" << viewName << '\"';
        }
        else
            os << view->getPath();
        
        os << " )" << std::endl; 
    }

    const Viewport& vp = channel.getViewport();
    const PixelViewport& pvp = channel.getPixelViewport();
    if( vp.isValid( ) && channel.hasFixedViewport( ))
    {
        if( pvp.hasArea( ))
            os << "viewport " << pvp << std::endl;
        os << "viewport " << vp << std::endl;
    }
    else if( pvp.hasArea( ))
    {
        if( vp != Viewport::FULL && vp.isValid( ))
            os << "viewport " << vp << std::endl;
        os << "viewport " << pvp << std::endl;
    }


    const uint32_t drawable = channel.getDrawable();
    if( drawable !=  Channel::FB_WINDOW )
    {
        os << "drawable [";
        
        if ((drawable &  Channel::FBO_COLOR) != 0 )
        {
           os << " FBO_COLOR";
        }
        
        if ((drawable &  Channel::FBO_DEPTH) != 0)
        {
           os << " FBO_DEPTH"; 
        } 
        if ((drawable &  Channel::FBO_STENCIL) != 0) 
        {
           os << " FBO_STENCIL";  
        }
        
        os << " ]" << std::endl;
    }
    bool attrPrinted   = false;
    
    for( Channel::IAttribute i = static_cast<Channel::IAttribute>( 0 );
         i < Channel::IATTR_LAST;
         i = static_cast<Channel::IAttribute>( static_cast<uint32_t>(i)+1 ))
    {
        const int value = channel.getIAttribute( i );
        if( value == Global::instance()->getChannelIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i==Channel::IATTR_HINT_STATISTICS ?
                "hint_statistics   " :
                i==Channel::IATTR_HINT_SENDTOKEN ?
                    "hint_sendtoken    " : "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << base::exdent << "}" << std::endl << std::endl;

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;

    return os;
}

}
}

#include "../lib/fabric/channel.ipp"
template class eq::fabric::Channel< eq::server::Window, eq::server::Channel >;
