
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
#include "window.h"

#include <eq/client/commands.h>
#include <eq/client/packets.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/paths.h>
#include <eq/net/command.h>

namespace eq
{
namespace server
{

typedef fabric::Pipe< Node, Pipe, Window > Super;
typedef net::CommandFunc<Pipe> PipeFunc;

void Pipe::_construct()
{
    _active         = 0;
    _lastDrawWindow = 0;

    EQINFO << "New pipe @" << (void*)this << std::endl;
}

Pipe::Pipe( Node* parent ) : Super( parent )
{
    _construct();

    parent->addPipe( this );
    const Global* global = Global::instance();
    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = global->getPipeIAttribute(
            static_cast<IAttribute>( i ));
}

Pipe::Pipe( const Pipe& from, Node* parent ) : Super( parent )
{
    _construct();

    setName( from.getName() );
    setPort( from.getPort() );
    setDevice( from.getDevice() );
    _pvp    = from._pvp;

    parent->addPipe( this );

    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    const WindowVector& windows = from.getWindows();
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        new Window( **i, this );
    }
}

Pipe::~Pipe()
{
    EQINFO << "Delete pipe @" << (void*)this << std::endl;
    Node* node = getNode();
    if( node )
        node->removePipe( this );
    
    WindowVector& windows = _getWindows(); 
    while( !windows.empty() )
    {
        Window* window = windows.back();
        _removeWindow( window );
        delete window;
    }
    windows.clear();
}

void Pipe::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( CMD_PIPE_CONFIG_INIT_REPLY,
                     PipeFunc( this, &Pipe::_cmdConfigInitReply ), queue );
    registerCommand( CMD_PIPE_CONFIG_EXIT_REPLY, 
                     PipeFunc( this, &Pipe::_cmdConfigExitReply ), queue );
}

void Pipe::addWindow( Window* window )
{
    EQASSERT( window->getChannels().empty( ));
    _addWindow( window );
    window->notifyViewportChanged();
}

bool Pipe::removeWindow( Window* window )
{
    EQASSERT( window->getChannels().empty( ));

    return _removeWindow( window );
}

ServerPtr Pipe::getServer()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getServer() : 0);
}

const ServerPtr Pipe::getServer() const
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

net::CommandQueue* Pipe::getServerThreadQueue()
{ 
    Node* node = getNode();
    EQASSERT( node );
    return node->getServerThreadQueue(); 
}

net::CommandQueue* Pipe::getCommandThreadQueue()
{ 
    Node* node = getNode();
    EQASSERT( node );
    return node->getCommandThreadQueue(); 
}

Channel* Pipe::getChannel( const ChannelPath& path )
{
    WindowVector& windows = _getWindows(); 
    EQASSERT( windows.size() > path.windowIndex );

    if( windows.size() <= path.windowIndex )
        return 0;

    return windows[ path.windowIndex ]->getChannel( path );
}

namespace
{
template< class C >
VisitorResult _accept( C* pipe, PipeVisitor& visitor )
{ 
    VisitorResult result = visitor.visitPre( pipe );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const WindowVector& windows = pipe->getWindows();
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
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

    switch( visitor.visitPost( pipe ))
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

VisitorResult Pipe::accept( PipeVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Pipe::accept( PipeVisitor& visitor ) const
{
    return _accept( this, visitor );
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
    _setTasks( getTasks() | tasks );
    node->addTasks( tasks );
}

void Pipe::send( net::ObjectPacket& packet )
{ 
    Node* node = getNode();
    EQASSERT( node );
    packet.objectID = getID();
    node->send( packet ); 
}

void Pipe::_send( net::ObjectPacket& packet, const std::string& string ) 
{
    Node* node = getNode();
    EQASSERT( node );
    packet.objectID = getID(); 
    node->send( packet, string ); 
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
    WindowVector& windows = _getWindows(); 
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
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
    bool success = true;
    WindowVector& windows = _getWindows(); 
    for( WindowVector::const_iterator i = windows.begin();
         i != windows.end(); ++i )
    {
        Window* window = *i;
        if( !window->syncRunning( ))
        {
            setErrorMessage( getErrorMessage() + " window " + 
                             window->getName() + ": '" + 
                             window->getErrorMessage() + '\'' );
            success = false;
        }
    }

    if( isActive() && _state != STATE_RUNNING && !_syncConfigInit( ))
        // becoming active
        success = false;

    if( !isActive() && !_syncConfigExit( ))
        // becoming inactive
        success = false;

    EQASSERT( _state == STATE_RUNNING || _state == STATE_STOPPED ||
              _state == STATE_INIT_FAILED );
    return success;
}

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Pipe::_configInit( const uint32_t initID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_STOPPED );
    _state         = STATE_INITIALIZING;

    getConfig()->registerObject( this );

    EQLOG( LOG_INIT ) << "Create pipe" << std::endl;
    NodeCreatePipePacket createPipePacket;
    createPipePacket.objectID = getNode()->getID();
    createPipePacket.pipeID   = getID();
    createPipePacket.threaded = getIAttribute( IATTR_HINT_THREAD );
    getNode()->send( createPipePacket );

    EQLOG( LOG_INIT ) << "Init pipe" << std::endl;
    PipeConfigInitPacket packet;
    packet.initID = initID;
    packet.port   = getPort();
    packet.device = getDevice();
    packet.tasks  = getTasks();
    packet.pvp    = _pvp;
    packet.frameNumber = frameNumber;
    packet.cudaGLInterop = getIAttribute( IATTR_HINT_CUDA_GL_INTEROP );
    _send( packet, getName() );
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
        EQWARN << "Pipe initialization failed: " << getErrorMessage() << std::endl;

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
    packet.exitThread = ( getIAttribute( IATTR_HINT_THREAD ) != OFF );
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

    getConfig()->deregisterObject( this );

    _state = STATE_STOPPED; // EXIT_FAILED -> STOPPED transition
    _setTasks( fabric::TASK_NONE );
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
    send( startPacket );
    EQLOG( LOG_TASKS ) << "TASK pipe start frame " << &startPacket << std::endl;

    WindowVector& windows = _getWindows(); 
    for( WindowVector::const_iterator i = windows.begin();
         i != windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isActive( ))
            window->updateDraw( frameID, frameNumber );
    }
    
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
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

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Pipe::setPixelViewport( const PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.hasArea( ))
        return;

    _pvp = pvp;

    const WindowVector& windows = getWindows();
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        (*i)->notifyViewportChanged();
    }
    EQINFO << "Pipe pvp set: " << _pvp << std::endl;
}

//===========================================================================
// command handling
//===========================================================================
net::CommandResult Pipe::_cmdConfigInitReply( net::Command& command ) 
{
    const PipeConfigInitReplyPacket* packet = 
        command.getPacket<PipeConfigInitReplyPacket>();
    EQVERB << "handle pipe configInit reply " << packet << std::endl;

    setErrorMessage( getErrorMessage() + packet->error );
    setPixelViewport( packet->pvp );

    if( packet->result )
        _state = STATE_RUNNING;
    else
        _state = STATE_INIT_FAILED;

    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdConfigExitReply( net::Command& command ) 
{
    const PipeConfigExitReplyPacket* packet = 
        command.getPacket<PipeConfigExitReplyPacket>();
    EQVERB << "handle pipe configExit reply " << packet << std::endl;

    if( packet->result )
        _state = STATE_EXIT_SUCCESS;
    else
        _state = STATE_EXIT_FAILED;

    return net::COMMAND_HANDLED;
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
           << static_cast<IAttrValue>( value ) << std::endl;
    }
    
    if( attrPrinted )
        os << base::exdent << "}" << std::endl;

    os << std::endl;

    const WindowVector& windows = pipe->getWindows();
    for( WindowVector::const_iterator i = windows.begin();
         i != windows.end(); ++i )

        os << *i;

    os << base::exdent << "}" << std::endl << base::enableHeader
       << base::enableFlush;
    return os;
}

}
}

#include "../lib/fabric/pipe.cpp"
template class eq::fabric::Pipe< eq::server::Node, eq::server::Pipe, 
                                 eq::server::Window >;

