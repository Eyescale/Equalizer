
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
#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "window.h"

#include <eq/client/packets.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <eq/net/command.h>

namespace eq
{
namespace server
{

typedef fabric::Pipe< Node, Pipe, Window, PipeVisitor > Super;
typedef net::CommandFunc<Pipe> PipeFunc;

void Pipe::_construct()
{
    _active         = 0;
    _lastDrawWindow = 0;
    EQINFO << "New pipe @" << (void*)this << std::endl;
}

Pipe::Pipe( Node* parent )
        : Super( parent )
{
    _construct();

    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_ALL; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getPipeIAttribute( attr ));
    }
}

Pipe::~Pipe()
{
    EQINFO << "Delete pipe @" << (void*)this << std::endl;
}

void Pipe::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    Super::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( fabric::CMD_PIPE_CONFIG_INIT_REPLY,
                     PipeFunc( this, &Pipe::_cmdConfigInitReply ), queue );
    registerCommand( fabric::CMD_PIPE_CONFIG_EXIT_REPLY, 
                     PipeFunc( this, &Pipe::_cmdConfigExitReply ), queue );
}

ServerPtr Pipe::getServer()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getServer() : 0);
}

ConstServerPtr Pipe::getServer() const
{ 
    const Node* node = getNode();
    EQASSERT( node );
    return node ? node->getServer() : 0; 
}

Config* Pipe::getConfig()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getConfig() : 0);
}

const Config* Pipe::getConfig() const
{
    const Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getConfig() : 0);
}

net::CommandQueue* Pipe::getMainThreadQueue()
{ 
    Node* node = getNode();
    EQASSERT( node );
    return node->getMainThreadQueue(); 
}

net::CommandQueue* Pipe::getCommandThreadQueue()
{ 
    Node* node = getNode();
    EQASSERT( node );
    return node->getCommandThreadQueue(); 
}

Channel* Pipe::getChannel( const ChannelPath& path )
{
    const Windows& windows = getWindows(); 
    EQASSERT( windows.size() > path.windowIndex );

    if( windows.size() <= path.windowIndex )
        return 0;

    return windows[ path.windowIndex ]->getChannel( path );
}

void Pipe::activate()
{   
    Node* node = getNode();
    EQASSERT( node );

    ++_active;
    if( node ) 
        node->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Pipe::deactivate()
{ 
    EQASSERT( _active != 0 );

    Node* node = getNode();
    EQASSERT( node );

    --_active; 
    if( node ) 
        node->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Pipe::addTasks( const uint32_t tasks )
{
    Node* node = getNode();
    EQASSERT( node );
    setTasks( getTasks() | tasks );
    node->addTasks( tasks );
}

void Pipe::send( net::ObjectPacket& packet )
{ 
    Node* node = getNode();
    EQASSERT( node );
    packet.objectID = getID();
    node->send( packet ); 
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// update running entities (init/exit)
//---------------------------------------------------------------------------

void Pipe::updateRunning( const uint32_t initID, const uint32_t frameNumber )
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return;

    setErrorMessage( std::string( ));

    if( isActive() && _state != STATE_RUNNING ) // becoming active
        _configInit( initID, frameNumber );

    // Let all running windows update their running state (incl. children)
    const Windows& windows = getWindows(); 
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
    {
        (*i)->updateRunning( initID );
    }

    if( !isActive( )) // becoming inactive
        _configExit();
}

bool Pipe::syncRunning()
{
    if( !isActive() && _state == STATE_STOPPED ) // inactive
        return true;

    // Sync state updates
    bool result = true;
    const Windows& windows = getWindows(); 
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
    {
        Window* window = *i;
        if( !window->syncRunning( ))
        {
            setErrorMessage( getErrorMessage() + " window " + 
                             window->getName() + ": '" + 
                             window->getErrorMessage() + '\'' );
            result = false;
        }
    }

    if( isActive() && _state != STATE_RUNNING && !_syncConfigInit( ))
        // becoming active
        result = false;
    if( !isActive() && !_syncConfigExit( )) // becoming inactive
        result = false;

    EQASSERT( isMaster( ));
    EQASSERT( _state == STATE_RUNNING || _state == STATE_STOPPED ||
              _state == STATE_INIT_FAILED );
    return result;
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Pipe::_configInit( const uint32_t initID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_INITIALIZING;

    EQLOG( LOG_INIT ) << "Create pipe" << std::endl;
    NodeCreatePipePacket createPipePacket;
    createPipePacket.objectID = getNode()->getID();
    createPipePacket.pipeID   = getID();
    createPipePacket.threaded = getIAttribute( IATTR_HINT_THREAD );
    getNode()->send( createPipePacket );

    EQLOG( LOG_INIT ) << "Init pipe" << std::endl;
    PipeConfigInitPacket packet;
    packet.initID = initID;
    packet.frameNumber = frameNumber;
    send( packet );
}

bool Pipe::_syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    const bool success = ( _state == STATE_INIT_SUCCESS );
    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Pipe initialization failed: " << getErrorMessage()
               << std::endl;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Pipe::_configExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit pipe" << std::endl;
    PipeConfigExitPacket packet;
    send( packet );

    EQLOG( LOG_INIT ) << "Destroy pipe" << std::endl;
    NodeDestroyPipePacket destroyPipePacket;
    destroyPipePacket.objectID = getNode()->getID();
    destroyPipePacket.pipeID   = getID();
    getNode()->send( destroyPipePacket );
}

bool Pipe::_syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    _state = STATE_STOPPED; // EXIT_FAILED -> STOPPED transition
    setTasks( fabric::TASK_NONE );
    sync();
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint32_t frameID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    PipeFrameStartClockPacket startClockPacket;
    send( startClockPacket );

    PipeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    startPacket.version     = getVersion();
    send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK pipe start frame " << &startPacket << std::endl;

    const Windows& windows = getWindows(); 
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isActive( ))
            window->updateDraw( frameID, frameNumber );
    }
 
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isActive( ))
            window->updatePost( frameID, frameNumber );
    }

    PipeFrameFinishPacket finishPacket;
    finishPacket.frameID      = frameID;
    finishPacket.frameNumber  = frameNumber;

    send( finishPacket );
    EQLOG( LOG_TASKS ) << "TASK pipe finish frame  " << &finishPacket
                           << std::endl;
    _lastDrawWindow = 0;
}



//===========================================================================
// command handling
//===========================================================================
net::CommandResult Pipe::_cmdConfigInitReply( net::Command& command ) 
{
    const PipeConfigInitReplyPacket* packet = 
        command.getPacket<PipeConfigInitReplyPacket>();
    EQVERB << "handle pipe configInit reply " << packet << std::endl;

    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdConfigExitReply( net::Command& command ) 
{
    const PipeConfigExitReplyPacket* packet = 
        command.getPacket<PipeConfigExitReplyPacket>();
    EQVERB << "handle pipe configExit reply " << packet << std::endl;

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;

    return net::COMMAND_HANDLED;
}

void Pipe::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Super::deserialize( is, dirtyBits );
    EQASSERT( isMaster( ));
    setDirty( dirtyBits ); // redistribute slave changes
}

std::ostream& operator << ( std::ostream& os, const Pipe* pipe )
{
    if( !pipe )
        return os;
    
    os << base::disableFlush << base::disableHeader << "pipe" << std::endl;
    os << "{" << std::endl << base::indent;

    const std::string& name = pipe->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    if( pipe->getPort() != EQ_UNDEFINED_UINT32 )
        os << "port     " << pipe->getPort() << std::endl;
        
    if( pipe->getDevice() != EQ_UNDEFINED_UINT32 )
        os << "device   " << pipe->getDevice() << std::endl;
    
    const PixelViewport& pvp = pipe->getPixelViewport();
    if( pvp.isValid( ))
        os << "viewport " << pvp << std::endl;

    bool attrPrinted   = false;
    for( Pipe::IAttribute i = static_cast<Pipe::IAttribute>( 0 ); 
         i < Pipe::IATTR_ALL; 
         i = static_cast<Pipe::IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = pipe->getIAttribute( i );
        if( value == Global::instance()->getPipeIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << base::indent;
            attrPrinted = true;
        }
        
        os << ( i == Pipe::IATTR_HINT_THREAD ?
                "hint_thread          " :
                i == Pipe::IATTR_HINT_CUDA_GL_INTEROP ?
                "hint_cuda_GL_interop " : "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << base::exdent << "}" << std::endl;

    os << std::endl;

    const Windows& windows = pipe->getWindows();
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
        os << *i;

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}

#include "../lib/fabric/pipe.ipp"
template class eq::fabric::Pipe< eq::server::Node, eq::server::Pipe, 
                                 eq::server::Window, eq::server::PipeVisitor >;

