
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/fabric/commands.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/leafVisitor.h>
#include <eq/fabric/paths.h>
#include <co/objectICommand.h>

#include <boost/foreach.hpp>

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
    for( unsigned i = 0; i < WindowSettings::IATTR_ALL; ++i )
    {
        const WindowSettings::IAttribute attr = static_cast< WindowSettings::IAttribute >( i );
        setIAttribute( attr, global->getWindowIAttribute( attr ));
    }
}

Window::~Window()
{
}

void Window::attach( const uint128_t& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* cmdQ = getCommandThreadQueue();
    registerCommand( fabric::CMD_OBJECT_SYNC,
                     WindowFunc( this, &Window::_cmdSync ), cmdQ );
    registerCommand( fabric::CMD_WINDOW_CONFIG_INIT_REPLY,
                     WindowFunc( this, &Window::_cmdConfigInitReply ), cmdQ );
    registerCommand( fabric::CMD_WINDOW_CONFIG_EXIT_REPLY,
                     WindowFunc( this, &Window::_cmdConfigExitReply ), cmdQ );
}

void Window::removeChild( const uint128_t& id )
{
    LBASSERT( getConfig()->isRunning( ));

    Channel* channel = _findChannel( id );
    LBASSERT( channel );
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
    LBASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}

Node* Window::getNode()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}

const Config* Window::getConfig() const
{
    const Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0);
}

Config* Window::getConfig()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0);
}

ServerPtr Window::getServer()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getServer() : 0 );
}

co::CommandQueue* Window::getMainThreadQueue()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return pipe->getMainThreadQueue();
}

co::CommandQueue* Window::getCommandThreadQueue()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return pipe->getCommandThreadQueue();
}

Channel* Window::getChannel( const ChannelPath& path )
{
    const Channels& channels = getChannels();
    LBASSERT( channels.size() > path.channelIndex );

    if( channels.size() <= path.channelIndex )
        return 0;

    return channels[ path.channelIndex ];
}

void Window::activate()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );

    ++_active;
    pipe->activate();

    LBLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Window::deactivate()
{
    LBASSERT( _active != 0 );
    Pipe* pipe = getPipe();
    LBASSERT( pipe );

    --_active;
    pipe->deactivate();

    LBLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
}

void Window::addTasks( const uint32_t tasks )
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    setTasks( getTasks() | tasks );
    pipe->addTasks( tasks );
}

//---------------------------------------------------------------------------
// swap barrier operations
//---------------------------------------------------------------------------
void Window::_resetSwapBarriers()
{
    Node* node = getNode();
    LBASSERT( node );

    BOOST_FOREACH( co::Barrier* barrier, _masterBarriers )
    {
        node->releaseBarrier( barrier );
    }

    _nvNetBarrier = 0;
    _masterBarriers.clear();
    _barriers.clear();
}

co::Barrier* Window::joinSwapBarrier( co::Barrier* barrier )
{
    _swapFinish = true;

    if( !barrier )
    {
        Node* node = getNode();
        barrier = node->getBarrier();
        barrier->increase();

        _masterBarriers.push_back( barrier );
        _barriers.push_back( barrier );
        return barrier;
    }

    co::BarriersCIter i = lunchbox::find( _barriers, barrier );
    if( i != _barriers.end( )) // Issue #39: window already has this barrier
        return barrier;

    LBASSERT( getPipe() );
    const Windows& windows = getPipe()->getWindows();
    bool beforeSelf = true;

    // Check if another window in the same thread is using the swap barrier
    for( WindowsCIter j = windows.begin(); j != windows.end(); ++j )
    {
        Window* window = *j;
        if( window == this ) // skip self
        {
            beforeSelf = false;
            continue;
        }

        co::Barriers& barriers = window->_barriers;
        co::BarriersIter k = lunchbox::find( barriers, barrier );
        if( k == barriers.end( ))
            continue;

        if( beforeSelf ) // some earlier window will do the barrier for us
            return barrier;

        // else we will do the barrier, remove from later window
        barriers.erase( k );
        _barriers.push_back( barrier );
        return barrier;
    }

    // No other window on this pipe does the barrier yet
    barrier->increase();
    _barriers.push_back( barrier );
    return barrier;
}

co::Barrier* Window::joinNVSwapBarrier( SwapBarrierConstPtr swapBarrier,
                                        co::Barrier* netBarrier )
{
    LBASSERTINFO( !_nvSwapBarrier,
                  "Only one NV_swap_group barrier per window allowed" );

    _nvSwapBarrier = swapBarrier;
    _nvNetBarrier = netBarrier;
    // no _swapFinish = true since NV_swap_group takes care of this

    if( !_nvNetBarrier )
    {
        Node* node = getNode();
        _nvNetBarrier = node->getBarrier();
        _masterBarriers.push_back( _nvNetBarrier );
    }

    _nvNetBarrier->increase();
    _barriers.push_back( _nvNetBarrier );
    return _nvNetBarrier;
}


co::ObjectOCommand Window::send( const uint32_t cmd )
{
    return getNode()->send( cmd, getID( ));
}

//===========================================================================
// Operations
//===========================================================================
//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Window::configInit( const uint128_t& initID, const uint32_t /*frame*/ )
{
    LBASSERT( !needsDelete( ));
    LBASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    LBLOG( LOG_INIT ) << "Create Window" << std::endl;
    getPipe()->send( fabric::CMD_PIPE_CREATE_WINDOW ) << getID();

    LBLOG( LOG_INIT ) << "Init Window" << std::endl;
    send( fabric::CMD_WINDOW_CONFIG_INIT ) << initID;
    LBLOG( LOG_TASKS ) << "TASK window configInit  " << " id " << getID()
                       << std::endl;
}

bool Window::syncConfigInit()
{
    LBASSERT( !needsDelete( ));
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
void Window::configExit()
{
    if( _state & STATE_EXITING )
        return;

    LBASSERT( isRunning() || _state == STATE_INIT_FAILED );
    _state =
        State( needsDelete() ? STATE_EXITING | STATE_DELETE : STATE_EXITING );

    LBLOG( LOG_INIT ) << "Exit Window" << std::endl;
    send( fabric::CMD_WINDOW_CONFIG_EXIT );
}

bool Window::syncConfigExit()
{
    _state.waitNE( STATE_EXITING, State( STATE_EXITING | STATE_DELETE ));
    const bool success = ( _state & STATE_EXIT_SUCCESS );
    LBASSERT( success || _state & STATE_EXIT_FAILED );

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
    if( !isRunning( ))
        return;

    LBASSERT( isActive( ))

    send( fabric::CMD_WINDOW_FRAME_START )
            << getVersion() << frameID << frameNumber;
    LBLOG( LOG_TASKS ) << "TASK window start frame " << frameNumber
                       << " id " << frameID << std::endl;

    const Channels& channels = getChannels();
    _swap = false;

    for( ChannelsCIter i = channels.begin(); i != channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->update( frameID, frameNumber ))
            _swap = true;
    }

    if( _lastDrawChannel )
        _lastDrawChannel = 0;
    else // no FrameDrawFinish sent
    {
        send( fabric::CMD_WINDOW_FRAME_DRAW_FINISH ) << frameID << frameNumber;
        LBLOG( LOG_TASKS ) << "TASK window draw finish " << getName()
                           << " frame " << frameNumber
                           << " id " << frameID << std::endl;
    }

    if( _swapFinish )
    {
        send( fabric::CMD_WINDOW_FLUSH );
        LBLOG( LOG_TASKS ) << "TASK flush " << std::endl;
    }
}

void Window::updatePost( const uint128_t& frameID,
                         const uint32_t frameNumber )
{
    if( !isRunning( ))
        return;

    LBASSERT( isActive( ))
    _updateSwap( frameNumber );

    send( fabric::CMD_WINDOW_FRAME_FINISH ) << frameID << frameNumber;
    LBLOG( LOG_TASKS ) << "TASK window finish frame " << frameNumber
                       << " id " << frameID << std::endl;
}

void Window::_updateSwap( const uint32_t frameNumber )
{
    if( _swapFinish )
    {
        send( fabric::CMD_WINDOW_FINISH );
        LBLOG( LOG_TASKS ) << "TASK finish " << frameNumber << std::endl;
        _swapFinish = false;
    }

    if( _maxFPS < std::numeric_limits< float >::max( ))
    {
        const float minFrameTime = 1000.0f / _maxFPS;
        send( fabric::CMD_WINDOW_THROTTLE_FRAMERATE ) << minFrameTime;
        LBLOG( LOG_TASKS ) << "TASK Throttle framerate  "
                               << minFrameTime << std::endl;

        _maxFPS = std::numeric_limits< float >::max();
    }

    for( co::BarriersCIter i = _barriers.begin(); i != _barriers.end(); ++i )
    {
        const co::Barrier* barrier = *i;
        if( barrier->getHeight() <= 1 )
        {
            LBVERB << "Ignoring swap barrier of height " << barrier->getHeight()
                   << std::endl;
            continue;
        }

        send( fabric::CMD_WINDOW_BARRIER ) << co::ObjectVersion( barrier );
        LBLOG( LOG_TASKS ) << "TASK barrier  barrier "
                           << co::ObjectVersion( barrier ) << std::endl;
    }

    if( _nvNetBarrier )
    {
        if( _nvNetBarrier->getHeight() <= 1 )
        {
            LBWARN << "Ignoring NV swap barrier of height "
                   << _nvNetBarrier->getHeight() << std::endl;
        }
        else
        {
            LBASSERT( _nvSwapBarrier );
            LBASSERT( _nvSwapBarrier->isNvSwapBarrier( ));
            // Entering the NV_swap_group. The _nvNetBarrier is also part of
            // _barriers, which means that the pre-join was already sync'ed
            // with a barrier.

            // Now enter the swap group and post-sync with the barrier again.
            send( fabric::CMD_WINDOW_NV_BARRIER )
                    << co::ObjectVersion( _nvNetBarrier )
                    << _nvSwapBarrier->getNVSwapGroup()
                    << _nvSwapBarrier->getNVSwapBarrier();
        }
    }

    _resetSwapBarriers();

    if( _swap )
    {
        send( fabric::CMD_WINDOW_SWAP );
        LBLOG( LOG_TASKS ) << "TASK swap  " << frameNumber << std::endl;
    }
}

//===========================================================================
// command handling
//===========================================================================
bool Window::_cmdConfigInitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBVERB << "handle window configInit reply " << command << std::endl;

    LBASSERT( !needsDelete( ));
    _state = command.read< bool >() ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Window::_cmdConfigExitReply( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBVERB << "handle window configExit reply " << command << std::endl;

    if( command.read< bool >( ))
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

    for( WindowSettings::IAttribute i = static_cast<WindowSettings::IAttribute>( 0 );
         i < WindowSettings::IATTR_LAST;
         i = static_cast<WindowSettings::IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = getIAttribute( i );
        if( value == Global::instance()->getWindowIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << lunchbox::indent;
            attrPrinted = true;
        }

        os << ( i == WindowSettings::IATTR_HINT_CORE_PROFILE ?
                    "hint_core_profile  " :
                i == WindowSettings::IATTR_HINT_OPENGL_MAJOR ?
                    "hint_opengl_major  " :
                i == WindowSettings::IATTR_HINT_OPENGL_MINOR ?
                    "hint_opengl_minor  " :
                i == WindowSettings::IATTR_HINT_STEREO ?
                    "hint_stereo        " :
                i == WindowSettings::IATTR_HINT_DOUBLEBUFFER ?
                    "hint_doublebuffer  " :
                i == WindowSettings::IATTR_HINT_FULLSCREEN ?
                    "hint_fullscreen    " :
                i == WindowSettings::IATTR_HINT_DECORATION ?
                    "hint_decoration    " :
                i == WindowSettings::IATTR_HINT_SWAPSYNC ?
                    "hint_swapsync      " :
                i == WindowSettings::IATTR_HINT_DRAWABLE ?
                    "hint_drawable      " :
                i == WindowSettings::IATTR_HINT_STATISTICS ?
                    "hint_statistics    " :
                i == WindowSettings::IATTR_HINT_SCREENSAVER ?
                    "hint_screensaver   " :
                i == WindowSettings::IATTR_HINT_GRAB_POINTER ?
                    "hint_grab_pointer  " :
                i == WindowSettings::IATTR_PLANES_COLOR ?
                    "planes_color       " :
                i == WindowSettings::IATTR_PLANES_ALPHA ?
                    "planes_alpha       " :
                i == WindowSettings::IATTR_PLANES_DEPTH ?
                    "planes_depth       " :
                i == WindowSettings::IATTR_PLANES_STENCIL ?
                    "planes_stencil     " :
                i == WindowSettings::IATTR_PLANES_ACCUM ?
                    "planes_accum       " :
                i == WindowSettings::IATTR_PLANES_ACCUM_ALPHA ?
                    "planes_accum_alpha " :
                i == WindowSettings::IATTR_PLANES_SAMPLES ?
                    "planes_samples     " : "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }

    if( attrPrinted )
        os << lunchbox::exdent << "}" << std::endl << std::endl;
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
