
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

#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "window.h"

#include <eq/nodePackets.h>
#include <eq/pipePackets.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <co/command.h>

namespace eq
{
namespace server
{

typedef fabric::Pipe< Node, Pipe, Window, PipeVisitor > Super;
typedef co::CommandFunc<Pipe> PipeFunc;


Pipe::Pipe( Node* parent )
        : Super( parent )
        , _active( 0 )
        , _state( STATE_STOPPED )
        , _lastDrawWindow( 0 )
{
    const Global* global = Global::instance();
    for( unsigned i = 0; i < IATTR_LAST; ++i )
    {
        const IAttribute attr = static_cast< IAttribute >( i );
        setIAttribute( attr, global->getPipeIAttribute( attr ));
    }
}

Pipe::~Pipe()
{
}

void Pipe::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    
    co::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( fabric::CMD_PIPE_CONFIG_INIT_REPLY,
                     PipeFunc( this, &Pipe::_cmdConfigInitReply ), queue );
    registerCommand( fabric::CMD_PIPE_CONFIG_EXIT_REPLY, 
                     PipeFunc( this, &Pipe::_cmdConfigExitReply ), queue );
}

void Pipe::removeChild( const co::base::UUID& id )
{
    EQASSERT( getConfig()->isRunning( ));

    Window* window = _findWindow( id );
    EQASSERT( window );
    if( window )
        window->postDelete();
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

co::CommandQueue* Pipe::getMainThreadQueue()
{ 
    Node* node = getNode();
    EQASSERT( node );
    return node->getMainThreadQueue(); 
}

co::CommandQueue* Pipe::getCommandThreadQueue()
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

void Pipe::send( co::ObjectPacket& packet )
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
// init
//---------------------------------------------------------------------------
void Pipe::configInit( const uint128_t& initID, const uint32_t frameNumber )
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

bool Pipe::syncConfigInit()
{
    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_INIT_SUCCESS ||
              _state == STATE_INIT_FAILED );

    _state.waitNE( STATE_INITIALIZING );

    if( _state == STATE_INIT_SUCCESS )
    {
        _state = STATE_RUNNING;
        return true;
    }

    EQWARN << "Pipe initialization failed: " << getError() << std::endl;
    configExit();
    return false;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Pipe::configExit()
{
    if( _state == STATE_EXITING )
        return;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_EXITING;

    EQLOG( LOG_INIT ) << "Exit pipe" << std::endl;
    PipeConfigExitPacket packet;
    send( packet );
}

bool Pipe::syncConfigExit()
{
    EQASSERT( _state == STATE_EXITING || _state == STATE_EXIT_SUCCESS || 
              _state == STATE_EXIT_FAILED );
    
    _state.waitNE( STATE_EXITING );
    const bool success = ( _state == STATE_EXIT_SUCCESS );
    EQASSERT( success || _state == STATE_EXIT_FAILED );

    _state = isActive() ? STATE_FAILED : STATE_STOPPED;
    setTasks( fabric::TASK_NONE );
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint128_t& frameID, const uint32_t frameNumber )
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
        if( window->isActive() && window->isRunning( ))
            window->updateDraw( frameID, frameNumber );
    }
 
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isActive() && window->isRunning( ))
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
bool Pipe::_cmdConfigInitReply( co::Command& command ) 
{
    const PipeConfigInitReplyPacket* packet = 
        command.getPacket<PipeConfigInitReplyPacket>();
    EQVERB << "handle pipe configInit reply " << packet << std::endl;

    _state = packet->result ? STATE_INIT_SUCCESS : STATE_INIT_FAILED;
    return true;
}

bool Pipe::_cmdConfigExitReply( co::Command& command ) 
{
    const PipeConfigExitReplyPacket* packet = 
        command.getPacket<PipeConfigExitReplyPacket>();
    EQVERB << "handle pipe configExit reply " << packet << std::endl;

    _state = packet->result ? STATE_EXIT_SUCCESS : STATE_EXIT_FAILED;
    return true;
}

void Pipe::output( std::ostream& os ) const
{
    bool attrPrinted   = false;
    for( IAttribute i = static_cast<IAttribute>( 0 ); 
         i < IATTR_LAST;
         i = static_cast<IAttribute>( static_cast<uint32_t>( i )+1))
    {
        const int value = getIAttribute( i );
        if( value == Global::instance()->getPipeIAttribute( i ))
            continue;

        if( !attrPrinted )
        {
            os << std::endl << "attributes" << std::endl;
            os << "{" << std::endl << co::base::indent;
            attrPrinted = true;
        }
        
        os << ( i == IATTR_HINT_THREAD ? "hint_thread          " :
                i == IATTR_HINT_CUDA_GL_INTEROP ? "hint_cuda_GL_interop " :
                "ERROR" )
           << static_cast< fabric::IAttribute >( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << co::base::exdent << "}" << std::endl;
}

}
}

#include "../fabric/pipe.ipp"
template class eq::fabric::Pipe< eq::server::Node, eq::server::Pipe, 
                                 eq::server::Window, eq::server::PipeVisitor >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */
