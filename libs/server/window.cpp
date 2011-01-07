
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

#include "window.h"

#include "global.h"
#include "channel.h"
#include "config.h"
#include "compound.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "swapBarrier.h"
  
#include <eq/pipePackets.h>
#include <eq/windowPackets.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <co/command.h>

namespace eq
{
namespace server
{
typedef fabric::Window< Pipe, Window, Channel > Super;
typedef co::CommandFunc<Window> WindowFunc;

Window::Window( Pipe* parent ) 
        : Super( parent )
        , _active( 0 )
        , _state( STATE_STOPPED )
        , _maxFPS( std::numeric_limits< float >::max( ))
        , _nvSwapBarrier( 0 )
        , _nvNetBarrier( 0 )
        , _lastDrawChannel( 0 )
        , _swapFinish( false )
        , _swap( false )
{
    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getWindowIAttribute( attr ));
    }
}

Window::~Window()
{
}

void Window::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( fabric::CMD_WINDOW_CONFIG_INIT_REPLY, 
                     WindowFunc( this, &Window::_cmdConfigInitReply ), queue );
    registerCommand( fabric::CMD_WINDOW_CONFIG_EXIT_REPLY, 
                     WindowFunc( this, &Window::_cmdConfigExitReply ), queue );
}

void Window::removeChild( const co::base::UUID& id )
{
    EQASSERT( getConfig()->isRunning( ));

    Channel* channel = _findChannel( id );
    EQASSERT( channel );
    if( channel )
        channel->postDelete();
}

void Window::postDelete()
{
    _state = State( _state.get() | STATE_DELETE );
    getConfig()->postNeedsFinish();

    const Channels& channels = getChannels(); 
    for( Channels::const_iterator i = channels.begin(); i!=channels.end(); ++i )
    {
        (*i)->postDelete();
    }    
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

ServerPtr Window::getServer() 
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getServer() : 0 );
}

co::CommandQueue* Window::getMainThreadQueue()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getMainThreadQueue(); 
}

co::CommandQueue* Window::getCommandThreadQueue()
{ 
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getCommandThreadQueue(); 
}

Channel* Window::getChannel( const ChannelPath& path )
{
    const Channels& channels = getChannels(); 
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
    pipe->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Window::deactivate()
{ 
    EQASSERT( _active != 0 );
    Pipe* pipe = getPipe();
    EQASSERT( pipe );

    --_active; 
    pipe->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Window::addTasks( const uint32_t tasks )
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    setTasks( getTasks() | tasks );
    pipe->addTasks( tasks );
}

//---------------------------------------------------------------------------
// swap barrier operations
//---------------------------------------------------------------------------
void Window::_resetSwapBarriers()
{ 
    Node* node = getNode();
    EQASSERT( node );

    for( std::vector< co::Barrier* >::iterator i = _masterSwapBarriers.begin();
         i != _masterSwapBarriers.end(); ++i )
    {
        node->releaseBarrier( *i );
    }

    _nvNetBarrier = 0;
    _masterSwapBarriers.clear();
    _swapBarriers.clear();
}

co::Barrier* Window::joinSwapBarrier( co::Barrier* barrier )
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
    const Windows& windows = getPipe()->getWindows();
    bool beforeSelf = true;

    // Check if another window in the same thread is using the swap barrier
    for( Windows::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        Window* window = *i;

        if( window == this ) // skip self
        {
            beforeSelf = false;
            continue;
        }

        co::Barriers& barriers = window->_swapBarriers;
        co::Barriers::iterator j = stde::find( barriers, barrier );
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

co::Barrier* Window::joinNVSwapBarrier( const SwapBarrier* swapBarrier,
                                         co::Barrier* netBarrier )
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


void Window::send( co::ObjectPacket& packet ) 
{
    packet.objectID = getID(); 
    getNode()->send( packet ); 
}

//===========================================================================
// Operations
//===========================================================================
//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Window::configInit( const uint128_t& initID, const uint32_t frameNumber )
{
    EQASSERT( !needsDelete( ));
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    EQLOG( LOG_INIT ) << "Create Window" << std::endl;
    PipeCreateWindowPacket createWindowPacket;
    createWindowPacket.windowID = getID();
    getPipe()->send( createWindowPacket );

    WindowConfigInitPacket packet;
    packet.initID = initID;
    
    EQLOG( LOG_INIT ) << "Init Window" << std::endl;
    send( packet );
    EQLOG( LOG_TASKS ) << "TASK window configInit  " << &packet << std::endl;
}

bool Window::syncConfigInit()
{
    EQASSERT( !needsDelete( ));
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_INIT_SUCCESS )
    {
        _state = STATE_RUNNING;
        return true;
    }

    EQWARN << "Window initialization failed: " << getError() <<std::endl;
    configExit();
    return false;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Window::configExit()
{
    if( _state & STATE_EXITING )
        return;

    EQASSERT( isRunning() || _state == STATE_INIT_FAILED );
    _state =
        State( needsDelete() ? STATE_EXITING | STATE_DELETE : STATE_EXITING );

    EQLOG( LOG_INIT ) << "Exit Window" << std::endl;
    WindowConfigExitPacket packet;
    send( packet );
}

bool Window::syncConfigExit()
{
    _state.waitNE( STATE_EXITING, State( STATE_EXITING | STATE_DELETE ));
    const bool success = ( _state & STATE_EXIT_SUCCESS );
    EQASSERT( success || _state & STATE_EXIT_FAILED );

    // EXIT_FAILED -> STOPPED transition
    uint32_t state = needsDelete() ? STATE_DELETE : 0;
    state |= ( isActive() ? STATE_FAILED : STATE_STOPPED );
    _state = State( state );
    _nvSwapBarrier = 0;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Window::updateDraw( const uint128_t& frameID, const uint32_t frameNumber )
{
    EQASSERT( isRunning( ));
    EQASSERT( isActive( ));

    _swap = false;

    WindowFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    startPacket.version     = getVersion();
    send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK window start frame  " << &startPacket 
                           << std::endl;

    const Channels& channels = getChannels(); 
    for( Channels::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->isActive() && channel->isRunning( ))
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

void Window::updatePost( const uint128_t& frameID, 
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
    
    for( std::vector<co::Barrier*>::iterator i = _swapBarriers.begin();
         i != _swapBarriers.end(); ++i )
    {
        const co::Barrier* barrier = *i;

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
bool Window::_cmdConfigInitReply( co::Command& command )
{
    const WindowConfigInitReplyPacket* packet =
        command.getPacket<WindowConfigInitReplyPacket>();
    EQVERB << "handle window configInit reply " << packet << std::endl;

    EQASSERT( !needsDelete( ));
    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Window::_cmdConfigExitReply( co::Command& command )
{
    const WindowConfigExitReplyPacket* packet =
        command.getPacket<WindowConfigExitReplyPacket>();
    EQVERB << "handle window configExit reply " << packet << std::endl;

    if( packet->result )
        _state = State( needsDelete() ? 
                        STATE_EXIT_SUCCESS|STATE_DELETE : STATE_EXIT_SUCCESS );
    else
        _state = State( needsDelete() ? 
                        STATE_EXIT_FAILED | STATE_DELETE : STATE_EXIT_FAILED );
    return true;
}

void Window::output( std::ostream& os ) const
{
    bool attrPrinted   = false;
    
    for( IAttribute i = static_cast<IAttribute>( 0 );
         i < IATTR_LAST;
         i = static_cast<IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = getIAttribute( i );
        if( value == Global::instance()->getWindowIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i== IATTR_HINT_STEREO ?
                    "hint_stereo        " :
                i== IATTR_HINT_DOUBLEBUFFER ?
                    "hint_doublebuffer  " :
                i== IATTR_HINT_FULLSCREEN ?
                    "hint_fullscreen    " :
                i== IATTR_HINT_DECORATION ?
                    "hint_decoration    " :
                i== IATTR_HINT_SWAPSYNC ?
                    "hint_swapsync      " :
                i== IATTR_HINT_DRAWABLE ?
                    "hint_drawable      " :
                i== IATTR_HINT_STATISTICS ?
                    "hint_statistics    " :
                i== IATTR_HINT_SCREENSAVER ?
                    "hint_screensaver   " :
                i== IATTR_PLANES_COLOR ? 
                    "planes_color       " :
                i== IATTR_PLANES_ALPHA ?
                    "planes_alpha       " :
                i== IATTR_PLANES_DEPTH ?
                    "planes_depth       " :
                i== IATTR_PLANES_STENCIL ?
                    "planes_stencil     " :
                i== IATTR_PLANES_ACCUM ?
                    "planes_accum       " :
                i== IATTR_PLANES_ACCUM_ALPHA ?
                    "planes_accum_alpha " :
                i== IATTR_PLANES_SAMPLES ?
                    "planes_samples     " : "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << co::base::exdent << "}" << std::endl << std::endl;
}

}
}

#include "../fabric/window.ipp"
template class eq::fabric::Window< eq::server::Pipe, eq::server::Window, 
                                   eq::server::Channel >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */
