
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "window.h"

#include "global.h"
#include "channel.h"
#include "config.h"
#include "compound.h"
#include "log.h"
#include "node.h"
#include "pipe.h"
#include "swapBarrier.h"
  
#include <eq/client/channel.h>
#include <eq/client/packets.h>
#include <eq/client/pipe.h>

#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <eq/net/command.h>

namespace eq
{
namespace server
{
typedef fabric::Window< Pipe, Window, Channel > Super;
typedef net::CommandFunc<Window> WindowFunc;

void Window::_construct()
{
    _active          = 0;
    _lastDrawChannel = 0;
    _maxFPS          = std::numeric_limits< float >::max();
    _swapFinish      = false;
    _swap            = false;
    _nvSwapBarrier   = 0;
    _nvNetBarrier    = 0;
    EQINFO << "New window @" << (void*)this << std::endl;
}

Window::Window( Pipe* parent ) 
        : Super( parent )
{
    _construct();
    
    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getWindowIAttribute( attr ));
    }
}

Window::Window( const Window& from, Pipe* parent )
        : Super( from, parent )
{
    _construct();
 
    const ChannelVector& channels = from.getChannels();
    for( ChannelVector::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        new Channel( **i, this );
    }            
}

Window::~Window()
{
    EQINFO << "Delete window @" << (void*)this << std::endl;
    
    ChannelVector& channels = _getChannels(); 
    while( !channels.empty( ))
    {
        Channel* channel = channels.back();

        EQASSERT( channel->getWindow() == this );
        _removeChannel( channel );
        delete channel;
    }
    EQASSERT( channels.empty( ));
}

void Window::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( CMD_WINDOW_CONFIG_INIT_REPLY, 
                     WindowFunc( this, &Window::_cmdConfigInitReply),
                     commandQueue );
    registerCommand( CMD_WINDOW_CONFIG_EXIT_REPLY, 
                     WindowFunc( this, &Window::_cmdConfigExitReply),
                     commandQueue );
}

const Node* Window::getNode() const 
{
    const Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}

Node* Window::getNode()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}

const Config* Window::getConfig() const
{
    const Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0);
}
Config* Window::getConfig() 
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0);
}

net::CommandQueue* Window::getServerThreadQueue()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getServerThreadQueue(); 
}

net::CommandQueue* Window::getCommandThreadQueue()
{ 
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getCommandThreadQueue(); 
}

Channel* Window::getChannel( const ChannelPath& path )
{
    ChannelVector& channels = _getChannels(); 
    EQASSERT( channels.size() > path.channelIndex );

    if( channels.size() <= path.channelIndex )
        return 0;

    return channels[ path.channelIndex ];
}

void Window::activate()
{   
    Pipe* pipe = getPipe();
    EQASSERT( pipe );

    ++_active;
    if( pipe ) 
        pipe->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Window::deactivate()
{ 
    EQASSERT( _active != 0 );
    Pipe* pipe = getPipe();
    EQASSERT( pipe );

    --_active; 
    if( pipe ) 
        pipe->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Window::addTasks( const uint32_t tasks )
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    _setTasks( getTasks() | tasks );
    pipe->addTasks( tasks );
}

//---------------------------------------------------------------------------
// swap barrier operations
//---------------------------------------------------------------------------
void Window::_resetSwapBarriers()
{ 
    Node* node = getNode();
    EQASSERT( node );

    for( std::vector< net::Barrier* >::iterator i = _masterSwapBarriers.begin();
         i != _masterSwapBarriers.end(); ++i )
            
        node->releaseBarrier( *i );

    _nvNetBarrier = 0;
    _masterSwapBarriers.clear();
    _swapBarriers.clear();
}

net::Barrier* Window::joinSwapBarrier( net::Barrier* barrier )
{
    _swapFinish = true;

    if( !barrier )
    {
        Node* node = getNode();
        barrier = node->getBarrier();
        barrier->increase();

        _masterSwapBarriers.push_back( barrier );
        _swapBarriers.push_back( barrier );
        return barrier;
    }

    EQASSERT( getPipe() );
    const WindowVector& windows = getPipe()->getWindows();
    bool beforeSelf = true;

    // Check if another window in the same thread is using the swap barrier
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        Window* window = *i;

        if( window == this ) // skip self
        {
            beforeSelf = false;
            continue;
        }

        net::BarrierVector& barriers = window->_swapBarriers;
        net::BarrierVector::iterator j =
            std::find( barriers.begin(), barriers.end(), barrier );
        if( j == barriers.end( ))
            continue;

        if( beforeSelf ) // some earlier window will do the barrier for us
            return barrier;
            
        // else we will do the barrier, remove from later window
        barriers.erase( j );
        _swapBarriers.push_back( barrier );
        return barrier;
    }

    // No other window on this pipe does the barrier yet
    barrier->increase();
    _swapBarriers.push_back( barrier );
    return barrier;
}

net::Barrier* Window::joinNVSwapBarrier( const SwapBarrier* swapBarrier,
                                         net::Barrier* netBarrier )
{ 
    EQASSERTINFO( !_nvSwapBarrier, 
                  "Only one NV_swap_group barrier per window allowed" );

    _nvSwapBarrier = swapBarrier;
    _nvNetBarrier = netBarrier;
    // no _swapFinish = true since NV_swap_group takes care of this

    if( !_nvNetBarrier )
    {
        Node* node = getNode();
        _nvNetBarrier = node->getBarrier();
        _masterSwapBarriers.push_back( _nvNetBarrier );
    }

    _nvNetBarrier->increase();
    _swapBarriers.push_back( _nvNetBarrier );
    return _nvNetBarrier;
}


void Window::send( net::ObjectPacket& packet ) 
{
    packet.objectID = getID(); 
    getNode()->send( packet ); 
}

void Window::_send( net::ObjectPacket& packet, const std::string& string ) 
{
    packet.objectID = getID(); 
    getNode()->send( packet, string ); 
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// update running entities (init/exit)
//---------------------------------------------------------------------------

void Window::updateRunning( const uint32_t initID )
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return;

    if( isActive() && _state != STATE_RUNNING ) // becoming active
        _configInit( initID );

    // Let all running channels update their running state (incl. children)
    ChannelVector& channels = _getChannels();
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        (*i)->updateRunning( initID );
    }

    if( !isActive( )) // becoming inactive
        _configExit();
}

bool Window::syncRunning()
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return true;

    // Sync state updates
    bool success = true;
    ChannelVector& channels = _getChannels(); 
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        Channel* channel = *i;
        if( !channel->syncRunning( ))
        {
            setErrorMessage( getErrorMessage() + "channel " + 
                             channel->getName() + ": '" + 
                             channel->getErrorMessage() + '\'' );
            success = false;
        }
    }

    if( isActive() && _state != STATE_RUNNING && !_syncConfigInit( ))
        // becoming active
        success = false;

    if( !isActive() && !_syncConfigExit( ))
        // becoming inactive
        success = false;

    if( getID() != EQ_ID_INVALID ) // TODO: remove (see TODO below)
        commit();

    EQASSERT( _state == STATE_STOPPED || _state == STATE_RUNNING || 
              _state == STATE_INIT_FAILED );
    return success;
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Window::_configInit( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state         = STATE_INITIALIZING;

    if( getID() == EQ_ID_INVALID ) // TODO: do at Server::chooseConfig time
        getConfig()->registerObject( this );
    else
        commit();

    EQLOG( LOG_INIT ) << "Create Window" << std::endl;
    PipeCreateWindowPacket createWindowPacket;
    createWindowPacket.windowID = getID();
    getPipe()->send( createWindowPacket );

    WindowConfigInitPacket packet;
    packet.initID = initID;
    
    EQLOG( LOG_INIT ) << "Init Window" << std::endl;
    _send( packet, getName() );
    EQLOG( LOG_TASKS ) << "TASK window configInit  " << &packet << std::endl;
}

bool Window::_syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    const bool success = ( _state == STATE_INIT_SUCCESS );
    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Window initialization failed: " 
               << getErrorMessage() << std::endl;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Window::_configExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit Window" << std::endl;
    WindowConfigExitPacket packet;
    send( packet );

    EQLOG( LOG_INIT ) << "Destroy Window" << std::endl;
    PipeDestroyWindowPacket destroyWindowPacket;
    destroyWindowPacket.windowID = getID();
    getPipe()->send( destroyWindowPacket );
}

bool Window::_syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    _state = STATE_STOPPED; // EXIT_FAILED -> STOPPED transition
    _nvSwapBarrier = 0;
    _setTasks( fabric::TASK_NONE );
    sync();
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Window::updateDraw( const uint32_t frameID, const uint32_t frameNumber )
{
    sync();
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    _swap = false;

    WindowFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    startPacket.version     = commit();
    send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK window start frame  " << &startPacket 
                           << std::endl;

    ChannelVector& channels = _getChannels(); 
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->isActive( ))
            _swap |= channel->update( frameID, frameNumber );
    }

    if( _swapFinish )
    {
        WindowFinishPacket packet;
        send( packet );
        EQLOG( LOG_TASKS ) << "TASK finish  " << &packet << std::endl;
        _swapFinish = false;
    }
}

void Window::updatePost( const uint32_t frameID, 
                         const uint32_t frameNumber )
{
    _updateSwap( frameNumber );

    WindowFrameFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    send( finishPacket );
    EQLOG( LOG_TASKS ) << "TASK window finish frame  " << &finishPacket
                           << std::endl;
    _lastDrawChannel = 0;
}

void Window::_updateSwap( const uint32_t frameNumber )
{
    if( _maxFPS < std::numeric_limits< float >::max( ))
    {
        WindowThrottleFramerate packetThrottle;
        packetThrottle.minFrameTime = 1000.0f / _maxFPS;
        
        send( packetThrottle );
        EQLOG( LOG_TASKS ) << "TASK Throttle framerate  " 
                               << &packetThrottle << std::endl;

        _maxFPS = std::numeric_limits< float >::max();
    }
    
    for( std::vector<net::Barrier*>::iterator i = _swapBarriers.begin();
         i != _swapBarriers.end(); ++i )
    {
        const net::Barrier*   barrier = *i;

        if( barrier->getHeight() <= 1 )
        {
            EQINFO << "Ignoring swap barrier of height " << barrier->getHeight()
                   << std::endl;
            continue;
        }

        WindowBarrierPacket packet;
        packet.barrier = barrier;
        send( packet );

        EQLOG( LOG_TASKS ) << "TASK barrier  " << &packet << std::endl;
    }

    if( _nvNetBarrier )
    {
        if( _nvNetBarrier->getHeight() <= 1 )
        {
            EQWARN << "Ignoring NV swap barrier of height "
                   << _nvNetBarrier->getHeight() << std::endl;
        }
        else
        {
            EQASSERT( _nvSwapBarrier );
            EQASSERT( _nvSwapBarrier->isNvSwapBarrier( ));
            // Entering the NV_swap_group. The _nvNetBarrier is also part of
            // _swapBarriers, which means that the pre-join was already sync'ed
            // with a barrier.

            // Now enter the swap group and post-sync with the barrier again.
            WindowNVBarrierPacket packet;
            packet.barrier = _nvSwapBarrier->getNVSwapBarrier();
            packet.group   = _nvSwapBarrier->getNVSwapGroup();
            packet.netBarrier = _nvNetBarrier;
            send( packet );
        }
    }

    _resetSwapBarriers();

    if( _swap )
    {
        WindowSwapPacket packet;

        send( packet );
        EQLOG( LOG_TASKS ) << "TASK swap  " << &packet << std::endl;
    }
}

//===========================================================================
// command handling
//===========================================================================
net::CommandResult Window::_cmdConfigInitReply( net::Command& command )
{
    const WindowConfigInitReplyPacket* packet =
        command.getPacket<WindowConfigInitReplyPacket>();
    EQVERB << "handle window configInit reply " << packet << std::endl;

    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigExitReply( net::Command& command )
{
    const WindowConfigExitReplyPacket* packet =
        command.getPacket<WindowConfigExitReplyPacket>();
    EQVERB << "handle window configExit reply " << packet << std::endl;

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return net::COMMAND_HANDLED;
}

void Window::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );
    EQASSERT( isMaster( ));
    setDirty( dirtyBits ); // redistribute slave changes
}

std::ostream& operator << ( std::ostream& os, const Window* window )
{
    if( !window )
        return os;
    
    os << base::disableFlush << base::disableHeader << "window" << std::endl;
    os << "{" << std::endl << base::indent;

    const std::string& name = window->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Viewport& vp  = window->getViewport();
    if( vp.isValid( ) && !window->hasFixedViewport() )
    {
        if( vp != Viewport::FULL )
            os << "viewport " << vp << std::endl;
    }
    else
    {
        const PixelViewport& pvp = window->getPixelViewport();
        if( pvp.isValid( ))
            os << "viewport " << pvp << std::endl;
    }

    bool attrPrinted   = false;
    
    for( Window::IAttribute i = static_cast<Window::IAttribute>( 0 );
         i < Window::IATTR_ALL; 
         i = static_cast<Window::IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = window->getIAttribute( i );
        if( value == Global::instance()->getWindowIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i== Window::IATTR_HINT_STEREO ?
                    "hint_stereo        " :
                i== Window::IATTR_HINT_DOUBLEBUFFER ?
                    "hint_doublebuffer  " :
                i== Window::IATTR_HINT_FULLSCREEN ?
                    "hint_fullscreen    " :
                i== Window::IATTR_HINT_DECORATION ?
                    "hint_decoration    " :
                i== Window::IATTR_HINT_SWAPSYNC ?
                    "hint_swapsync      " :
                i== Window::IATTR_HINT_DRAWABLE ?
                    "hint_drawable      " :
                i== Window::IATTR_HINT_STATISTICS ?
                    "hint_statistics    " :
                i== Window::IATTR_PLANES_COLOR ? 
                    "planes_color       " :
                i== Window::IATTR_PLANES_ALPHA ?
                    "planes_alpha       " :
                i== Window::IATTR_PLANES_DEPTH ?
                    "planes_depth       " :
                i== Window::IATTR_PLANES_STENCIL ?
                    "planes_stencil     " :
                i== Window::IATTR_PLANES_ACCUM ?
                    "planes_accum       " :
                i== Window::IATTR_PLANES_ACCUM_ALPHA ?
                    "planes_accum_alpha " :
                i== Window::IATTR_PLANES_SAMPLES ?
                    "planes_samples     " : "ERROR" )
           << static_cast<IAttrValue>( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << base::exdent << "}" << std::endl << std::endl;

    const ChannelVector& channels = window->getChannels();
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        const Channel* channel = *i;
        if( channel->getView() && channel->getSegment( ))
            continue; // don't print generated channels for now

        os << *channel;
    }

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}

#include "../lib/fabric/window.cpp"
template class eq::fabric::Window< eq::server::Pipe, eq::server::Window, 
                                   eq::server::Channel >;

