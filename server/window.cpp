
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "paths.h"
#include "pipe.h"
#include "windowVisitor.h"

#include <eq/client/packets.h>
#include <eq/net/command.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Window> WindowFunc;

void Window::_construct()
{
    _active          = 0;
    _pipe            = 0;
    _tasks           = eq::TASK_NONE;
    _fixedPVP        = false;
    _lastDrawChannel = 0;
    _maxFPS          = numeric_limits< float >::max();
    _doSwap          = false;
    _nvSwapBarrier   = 0;
    EQINFO << "New window @" << (void*)this << endl;
}

Window::Window()
{
    _construct();
    
    const Global* global = Global::instance();
    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _iAttributes[i] = global->getWindowIAttribute(
            static_cast<eq::Window::IAttribute>( i ));
}

Window::Window( const Window& from, Pipe* pipe )
        : net::Object()
{
    _construct();

    _name     = from._name;
    _pvp      = from._pvp;
    _vp       = from._vp;
    _fixedPVP = from._fixedPVP;

    pipe->addWindow( this );

    for( int i=0; i<eq::Window::IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    const ChannelVector& channels = from.getChannels();
    for( ChannelVector::const_iterator i = channels.begin();
         i != channels.end(); ++i )
    {
        new Channel( **i, this );
    }            
}

Window::~Window()
{
    EQINFO << "Delete window @" << (void*)this << endl;

    if( _pipe )
        _pipe->removeWindow( this );
    
    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;

        channel->_window = 0;
        delete channel;
    }
    _channels.clear();
}

void Window::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* commandQueue = getCommandThreadQueue();

    registerCommand( eq::CMD_WINDOW_CONFIG_INIT_REPLY, 
                     WindowFunc( this, &Window::_cmdConfigInitReply),
                     commandQueue );
    registerCommand( eq::CMD_WINDOW_CONFIG_EXIT_REPLY, 
                     WindowFunc( this, &Window::_cmdConfigExitReply),
                     commandQueue );
    registerCommand( eq::CMD_WINDOW_SET_PVP, 
                     WindowFunc( this, &Window::_cmdSetPixelViewport ),
                     commandQueue );
                         
}

void Window::addChannel( Channel* channel )
{
    EQASSERT( find( _channels.begin(), _channels.end(), channel ) == 
              _channels.end( ));

    _channels.push_back( channel ); 
    channel->_window = this;
    channel->notifyViewportChanged();
}

bool Window::removeChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _channels.begin(), _channels.end(),
                                        channel );
    if( i == _channels.end( ))
        return false;

    _channels.erase( i );
    channel->_window = 0;
    return true;
}

Node* Window::getNode()
{
    EQASSERT( _pipe );
    return (_pipe ? _pipe->getNode() : 0); 
}
const Node* Window::getNode() const
{
    EQASSERT( _pipe );
    return (_pipe ? _pipe->getNode() : 0); 
}

Config* Window::getConfig()
{
    EQASSERT( _pipe );
    return (_pipe ? _pipe->getConfig() : 0); 
}
const Config* Window::getConfig() const 
{
    EQASSERT( _pipe );
    return (_pipe ? _pipe->getConfig() : 0); 
}
        
net::CommandQueue* Window::getServerThreadQueue()
{
    EQASSERT( _pipe );
    return _pipe->getServerThreadQueue(); 
}

net::CommandQueue* Window::getCommandThreadQueue()
{ 
    EQASSERT( _pipe );
    return _pipe->getCommandThreadQueue(); 
}

WindowPath Window::getPath() const
{
    EQASSERT( _pipe );
    WindowPath path( _pipe->getPath( ));
    
    const WindowVector&    windows = _pipe->getWindows();
    WindowVector::const_iterator i = std::find( windows.begin(), windows.end(),
                                                this );
    EQASSERT( i != windows.end( ));
    path.windowIndex = std::distance( windows.begin(), i );
    return path;
}

Channel* Window::getChannel( const ChannelPath& path )
{
    EQASSERT( _channels.size() > path.channelIndex );

    if( _channels.size() <= path.channelIndex )
        return 0;

    return _channels[ path.channelIndex ];
}

namespace
{
template< class C, class V >
VisitorResult _accept( C* window, V& visitor )
{ 
    VisitorResult result = visitor.visitPre( window );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const ChannelVector& channels = window->getChannels();
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( window ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

VisitorResult Window::accept( WindowVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Window::accept( ConstWindowVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Window::activate()
{   
    EQASSERT( _pipe );

    ++_active;
    if( _pipe ) 
        _pipe->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Window::deactivate()
{ 
    EQASSERT( _active != 0 );
    EQASSERT( _pipe );

    --_active; 
    if( _pipe ) 
        _pipe->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Window::addTasks( const uint32_t tasks )
{
    EQASSERT( _pipe );
    _tasks |= tasks;
    _pipe->addTasks( tasks );
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Window::setPixelViewport( const eq::PixelViewport& pvp )
{
    if( !pvp.isValid( ))
        return;

    _fixedPVP = true;

    if( pvp == _pvp )
        return;

    _pvp      = pvp;
    _vp.invalidate();
    notifyViewportChanged();
}

void Window::setViewport( const eq::Viewport& vp )
{
    if( !vp.hasArea( ))
        return;

    _fixedPVP = false;

    if( vp == _vp )
        return;
    
    _vp = vp;
    _pvp.invalidate();
    notifyViewportChanged();
}

void Window::notifyViewportChanged()
{
    if( _pipe )
    {
        eq::PixelViewport pipePVP = _pipe->getPixelViewport();
        if( pipePVP.hasArea( ))
        {
            if( _fixedPVP ) // update viewport
                _vp = _pvp.getSubVP( pipePVP );
            else            // update pixel viewport
                _pvp = pipePVP.getSubPVP( _vp );
        }
    }
    EQINFO << "Window viewport update: " << _pvp << ":" << _vp << endl;

    for( ChannelVector::iterator i = _channels.begin();
         i != _channels.end(); ++i )

        (*i)->notifyViewportChanged();
}

//---------------------------------------------------------------------------
// swap barrier operations
//---------------------------------------------------------------------------
void Window::_resetSwapBarriers()
{ 
    Node* node = getNode();
    EQASSERT( node );

    for( vector<net::Barrier*>::iterator i = _masterSwapBarriers.begin();
         i != _masterSwapBarriers.end(); ++i )
            
        node->releaseBarrier( *i );

    _masterSwapBarriers.clear();
    _swapBarriers.clear();
}

net::Barrier* Window::newSwapBarrier()
{
    Node*         node    = getNode();
    net::Barrier* barrier = node->getBarrier();
    _masterSwapBarriers.push_back( barrier );

    joinSwapBarrier( barrier );
    return barrier;
}

void Window::joinSwapBarrier( net::Barrier* barrier )
{ 
    barrier->increase();
    _swapBarriers.push_back( barrier );

}

void Window::joinNVSwapBarrier( const SwapBarrier* barrier )
{ 
    if ( _nvSwapBarrier )
        EQWARN << "Only one NV_swap_group barrier per window allowed, "
               << "overwriting previous one" << endl;

    _nvSwapBarrier = barrier;
}

void Window::leaveNVSwapBarrier( const SwapBarrier* barrier )
{ 
    if ( _nvSwapBarrier == barrier )
        _nvSwapBarrier = 0;
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
    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
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
    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( !channel->syncRunning( ))
        {
            _error += "channel " + channel->getName() + ": '" + 
                      channel->getErrorMessage() + '\'';
            success = false;
        }
    }

    if( isActive() && _state != STATE_RUNNING && !_syncConfigInit( ))
        // becoming active
        success = false;

    if( !isActive() && !_syncConfigExit( ))
        // becoming inactive
        success = false;

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

    getConfig()->registerObject( this );

    EQLOG( LOG_INIT ) << "Create Window" << std::endl;
    eq::PipeCreateWindowPacket createWindowPacket;
    createWindowPacket.windowID = getID();
    _pipe->send( createWindowPacket );

    eq::WindowConfigInitPacket packet;
    packet.initID = initID;
    packet.tasks  = _tasks;

    if( _fixedPVP )
        packet.pvp    = _pvp; 
    else
        packet.vp     = _vp;

    memcpy( packet.iAttributes, _iAttributes, 
            eq::Window::IATTR_ALL * sizeof( int32_t ));
    
    EQASSERT( !_nvSwapBarrier || _nvSwapBarrier->isNvSwapBarrier() )
    if ( _nvSwapBarrier && _nvSwapBarrier->isNvSwapBarrier() )
    {
        packet.nvSwapBarrier = _nvSwapBarrier->getNVSwapBarrier();
        packet.nvSwapGroup   = _nvSwapBarrier->getNVSwapGroup();
    }
    else
    {
        packet.nvSwapBarrier = 0;
        packet.nvSwapGroup   = 0;
    }

    EQLOG( LOG_INIT ) << "Init Window" << std::endl;
    _send( packet, _name );
    EQLOG( eq::LOG_TASKS ) << "TASK window configInit  " << &packet << endl;
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
        EQWARN << "Window initialization failed: " << _error << endl;

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
    eq::WindowConfigExitPacket packet;
    send( packet );

    EQLOG( LOG_INIT ) << "Destroy Window" << std::endl;
    eq::PipeDestroyWindowPacket destroyWindowPacket;
    destroyWindowPacket.windowID = getID();
    _pipe->send( destroyWindowPacket );
}

bool Window::_syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    getConfig()->deregisterObject( this );

    _state = STATE_STOPPED; // EXIT_FAILED -> STOPPED transition
    _tasks = eq::TASK_NONE;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Window::updateDraw( const uint32_t frameID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    _doSwap = false;

    eq::WindowFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK window start frame  " << &startPacket 
                           << endl;

    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->isActive( ))
            _doSwap |= channel->update( frameID, frameNumber );
    }
}

void Window::updatePost( const uint32_t frameID, 
                         const uint32_t frameNumber )
{
    _updateSwap( frameNumber );

    eq::WindowFrameFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK window finish frame  " << &finishPacket
                           << endl;
    _lastDrawChannel = 0;
}

void Window::_updateSwap( const uint32_t frameNumber )
{
    bool doFinish = true;

    if( _maxFPS < numeric_limits< float >::max( ))
    {
        eq::WindowThrottleFramerate packetThrottle;
        packetThrottle.minFrameTime = 1000.0f / _maxFPS;
        
        send( packetThrottle );
        EQLOG( eq::LOG_TASKS ) << "TASK Throttle framerate  " 
                               << &packetThrottle << endl;

        _maxFPS = numeric_limits< float >::max();
    }
    
    for( vector<net::Barrier*>::iterator i = _swapBarriers.begin();
         i != _swapBarriers.end(); ++i )
    {
        const net::Barrier*   barrier = *i;
        if( barrier->getHeight() <= 1 )
        {
            EQWARN << "Ignoring swap barrier of height " << barrier->getHeight()
                   << endl;
            continue;
        }

        if( doFinish )
        {
            eq::WindowFinishPacket packet;
            send( packet );
            EQLOG( eq::LOG_TASKS ) << "TASK finish  " << &packet << endl;
            doFinish = false;
        }

        eq::WindowBarrierPacket packet;

        packet.barrierID      = barrier->getID();
        packet.barrierVersion = barrier->getVersion();
        send( packet );
        EQLOG( eq::LOG_TASKS ) << "TASK barrier  " << &packet << endl;
    }

    _resetSwapBarriers();

    if( _doSwap )
    {
        eq::WindowSwapPacket packet;

        send( packet );
        EQLOG( eq::LOG_TASKS ) << "TASK swap  " << &packet << endl;
    }
}

//===========================================================================
// command handling
//===========================================================================
net::CommandResult Window::_cmdConfigInitReply( net::Command& command )
{
    const eq::WindowConfigInitReplyPacket* packet =
        command.getPacket<eq::WindowConfigInitReplyPacket>();
    EQVERB << "handle window configInit reply " << packet << endl;

    if( packet->pvp.isValid( ))
        setPixelViewport( packet->pvp );

    _error = packet->error;

    if( packet->result )
    {
        _drawableConfig = packet->drawableConfig;
        _state = STATE_INIT_SUCCESS;
    }
    else
        _state = STATE_INIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigExitReply( net::Command& command )
{
    const eq::WindowConfigExitReplyPacket* packet =
        command.getPacket<eq::WindowConfigExitReplyPacket>();
    EQVERB << "handle window configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_EXIT_SUCCESS;
    else
        _state = STATE_EXIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdSetPixelViewport( net::Command& command)
{
    const eq::WindowSetPVPPacket* packet = 
        command.getPacket<eq::WindowSetPVPPacket>();
    EQVERB << "handle window set pvp " << packet << endl;

    setPixelViewport( packet->pvp );
    return net::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Window* window )
{
    if( !window )
        return os;
    
    os << disableFlush << disableHeader << "window" << endl;
    os << "{" << endl << indent; 

    const std::string& name = window->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

    const eq::Viewport& vp  = window->getViewport();
    if( vp.isValid( ) && !window->_fixedPVP )
    {
        if( vp != eq::Viewport::FULL )
            os << "viewport " << vp << endl;
    }
    else
    {
        const eq::PixelViewport& pvp = window->getPixelViewport();
        if( pvp.isValid( ))
            os << "viewport " << pvp << endl;
    }

    bool attrPrinted   = false;
    
    for( eq::Window::IAttribute i = static_cast<eq::Window::IAttribute>( 0 );
         i<eq::Window::IATTR_ALL; 
         i = static_cast<eq::Window::IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = window->getIAttribute( i );
        if( value == Global::instance()->getWindowIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << endl << "attributes" << endl;
            os << "{" << endl << indent;
            attrPrinted = true;
        }
        
        os << ( i==eq::Window::IATTR_HINT_STEREO ?
                    "hint_stereo        " :
                i==eq::Window::IATTR_HINT_DOUBLEBUFFER ?
                    "hint_doublebuffer  " :
                i==eq::Window::IATTR_HINT_FULLSCREEN ?
                    "hint_fullscreen    " :
                i==eq::Window::IATTR_HINT_DECORATION ?
                    "hint_decoration    " :
                i==eq::Window::IATTR_HINT_SWAPSYNC ?
                    "hint_swapsync      " :
                i==eq::Window::IATTR_HINT_DRAWABLE ?
                    "hint_drawable      " :
                i==eq::Window::IATTR_HINT_STATISTICS ?
                    "hint_statistics    " :
                i==eq::Window::IATTR_PLANES_COLOR ? 
                    "planes_color       " :
                i==eq::Window::IATTR_PLANES_ALPHA ?
                    "planes_alpha       " :
                i==eq::Window::IATTR_PLANES_DEPTH ?
                    "planes_depth       " :
                i==eq::Window::IATTR_PLANES_STENCIL ?
                    "planes_stencil     " :
                i==eq::Window::IATTR_PLANES_ACCUM ?
                    "planes_accum       " :
                i==eq::Window::IATTR_PLANES_ACCUM_ALPHA ?
                    "planes_accum_alpha " :
                i==eq::Window::IATTR_PLANES_SAMPLES ?
                    "planes_samples     " : "ERROR" )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }
    
    if( attrPrinted )
        os << exdent << "}" << endl << endl;

    const ChannelVector& channels = window->getChannels();
    for( ChannelVector::const_iterator i = channels.begin(); 
         i != channels.end(); ++i )
    {
        const Channel* channel = *i;
//        if( channel->getView() && channel->getSegment( ))
//            continue; // don't print generated channels for now

        os << channel;
    }

    os << exdent << "}" << endl << enableHeader << enableFlush;
    return os;
}

}
}
