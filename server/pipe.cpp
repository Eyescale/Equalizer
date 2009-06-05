
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
#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "paths.h"
#include "pipeVisitor.h"
#include "window.h"

#include <eq/client/commands.h>
#include <eq/client/packets.h>
#include <eq/net/command.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{
typedef net::CommandFunc<Pipe> PipeFunc;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_PIPE_") + #attr )
std::string Pipe::_iAttributeStrings[IATTR_ALL] = 
{
    MAKE_ATTR_STRING( IATTR_HINT_THREAD ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

void Pipe::_construct()
{
    _active         = 0;
    _node           = 0;
    _tasks          = eq::TASK_NONE;
    _port           = EQ_UNDEFINED_UINT32;
    _device         = EQ_UNDEFINED_UINT32;
    _lastDrawWindow = 0;

    EQINFO << "New pipe @" << (void*)this << endl;
}

Pipe::Pipe()
{
    _construct();

    const Global* global = Global::instance();
    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = global->getPipeIAttribute(
            static_cast<IAttribute>( i ));
}

Pipe::Pipe( const Pipe& from, Node* node )
        : eq::net::Object()
{
    _construct();

    _name   = from._name;
    _port   = from._port;
    _device = from._device;
    _pvp    = from._pvp;

    node->addPipe( this );

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
    EQINFO << "Delete pipe @" << (void*)this << endl;

    if( _node )
        _node->removePipe( this );
    
    for( vector<Window*>::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;

        window->_pipe = 0;
        delete window;
    }
    _windows.clear();
}

void Pipe::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               eq::net::Session* session )
{
    eq::net::Object::attachToSession( id, instanceID, session );
    
    eq::net::CommandQueue* queue = getCommandThreadQueue();

    registerCommand( eq::CMD_PIPE_CONFIG_INIT_REPLY,
                     PipeFunc( this, &Pipe::_cmdConfigInitReply ), queue );
    registerCommand( eq::CMD_PIPE_CONFIG_EXIT_REPLY, 
                     PipeFunc( this, &Pipe::_cmdConfigExitReply ), queue );
}

void Pipe::addWindow( Window* window )
{
    EQASSERT( window->getChannels().empty( ));

    _windows.push_back( window ); 
    window->_pipe = this;
    window->notifyViewportChanged();
}

bool Pipe::removeWindow( Window* window )
{
    EQASSERT( window->getChannels().empty( ));

    vector<Window*>::iterator i = find( _windows.begin(), _windows.end(),
                                        window );
    if( i == _windows.end( ))
        return false;

    _windows.erase( i );
    window->_pipe = 0;
    return true;
}

Server* Pipe::getServer()
{ 
    EQASSERT( _node );
    return _node ? _node->getServer() : 0; 
}
const Server* Pipe::getServer() const
{ 
    EQASSERT( _node );
    return _node ? _node->getServer() : 0; 
}

Config* Pipe::getConfig()
{
    EQASSERT( _node );
    return (_node ? _node->getConfig() : 0);
}
const Config* Pipe::getConfig() const
{
    EQASSERT( _node );
    return (_node ? _node->getConfig() : 0);
}

net::CommandQueue* Pipe::getServerThreadQueue()
{ 
    EQASSERT( _node );
    return _node->getServerThreadQueue(); 
}

net::CommandQueue* Pipe::getCommandThreadQueue()
{ 
    EQASSERT( _node );
    return _node->getCommandThreadQueue(); 
}

PipePath Pipe::getPath() const
{
    EQASSERT( _node );
    PipePath path( _node->getPath( ));
    
    const PipeVector&      pipes = _node->getPipes();
    PipeVector::const_iterator i = std::find( pipes.begin(), pipes.end(),
                                              this );
    EQASSERT( i != pipes.end( ));
    path.pipeIndex = std::distance( pipes.begin(), i );
    return path;
}

Channel* Pipe::getChannel( const ChannelPath& path )
{
    EQASSERT( _windows.size() > path.windowIndex );

    if( _windows.size() <= path.windowIndex )
        return 0;

    return _windows[ path.windowIndex ]->getChannel( path );
}

namespace
{
template< class C, class V >
VisitorResult _accept( C* pipe, V& visitor )
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

VisitorResult Pipe::accept( ConstPipeVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Pipe::activate()
{   
    EQASSERT( _node );

    ++_active;
    if( _node ) 
        _node->activate();

    EQLOG( LOG_VIEW ) << "activate: " << _active << std::endl;
}

void Pipe::deactivate()
{ 
    EQASSERT( _active != 0 );
    EQASSERT( _node );

    --_active; 
    if( _node ) 
        _node->deactivate(); 

    EQLOG( LOG_VIEW ) << "deactivate: " << _active << std::endl;
};

void Pipe::addTasks( const uint32_t tasks )
{
    EQASSERT( _node );
    _tasks |= tasks;
    _node->addTasks( tasks );
}

void Pipe::send( net::ObjectPacket& packet )
{ 
    EQASSERT( _node );
    packet.objectID = getID();
    _node->send( packet ); 
}

void Pipe::_send( net::ObjectPacket& packet, const std::string& string ) 
{
    EQASSERT( _node );
    packet.objectID = getID(); 
    _node->send( packet, string ); 
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

    _error.clear();

    if( isActive() && _state != STATE_RUNNING ) // becoming active
        _configInit( initID, frameNumber );

    // Let all running windows update their running state (incl. children)
    for( WindowVector::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
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
    for( WindowVector::const_iterator i = _windows.begin();
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( !window->syncRunning( ))
        {
            _error += " window " + window->getName() + ": '" + 
                      window->getErrorMessage() + '\'';
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
    eq::NodeCreatePipePacket createPipePacket;
    createPipePacket.objectID = _node->getID();
    createPipePacket.pipeID   = getID();
    createPipePacket.threaded = getIAttribute( IATTR_HINT_THREAD );
    _node->send( createPipePacket );

    EQLOG( LOG_INIT ) << "Init pipe" << std::endl;
    eq::PipeConfigInitPacket packet;
    packet.initID = initID;
    packet.port   = _port;
    packet.device = _device;
    packet.tasks  = _tasks;
    packet.pvp    = _pvp;
    packet.frameNumber = frameNumber;
    _send( packet, _name );
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
        EQWARN << "Pipe initialization failed: " << _error << endl;

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
    eq::PipeConfigExitPacket packet;
    packet.exitThread = ( getIAttribute( IATTR_HINT_THREAD ) != eq::OFF );
    send( packet );

    EQLOG( LOG_INIT ) << "Destroy pipe" << std::endl;
    eq::NodeDestroyPipePacket destroyPipePacket;
    destroyPipePacket.objectID = _node->getID();
    destroyPipePacket.pipeID   = getID();
    _node->send( destroyPipePacket );
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
    _tasks = eq::TASK_NONE;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint32_t frameID, const uint32_t frameNumber )
{
    EQASSERT( _state == STATE_RUNNING );
    EQASSERT( _active > 0 );

    eq::PipeFrameStartClockPacket startClockPacket;
    send( startClockPacket );

    eq::PipeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;
    send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK pipe start frame " << &startPacket << endl;

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isActive( ))
            window->updateDraw( frameID, frameNumber );
    }

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isActive( ))
            window->updatePost( frameID, frameNumber );
    }

    eq::PipeFrameFinishPacket finishPacket;
    finishPacket.frameID      = frameID;
    finishPacket.frameNumber  = frameNumber;

    send( finishPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK pipe finish frame  " << &finishPacket
                           << endl;
    _lastDrawWindow = 0;
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Pipe::setPixelViewport( const eq::PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.hasArea( ))
        return;

    _pvp = pvp;
    EQINFO << "Pipe pvp set: " << _pvp << endl;
}

//===========================================================================
// command handling
//===========================================================================
eq::net::CommandResult Pipe::_cmdConfigInitReply( eq::net::Command& command ) 
{
    const eq::PipeConfigInitReplyPacket* packet = 
        command.getPacket<eq::PipeConfigInitReplyPacket>();
    EQVERB << "handle pipe configInit reply " << packet << endl;

    _error += packet->error;
    setPixelViewport( packet->pvp );

    if( packet->result )
        _state = STATE_RUNNING;
    else
        _state = STATE_INIT_FAILED;

    return eq::net::COMMAND_HANDLED;
}

eq::net::CommandResult Pipe::_cmdConfigExitReply( eq::net::Command& command ) 
{
    const eq::PipeConfigExitReplyPacket* packet = 
        command.getPacket<eq::PipeConfigExitReplyPacket>();
    EQVERB << "handle pipe configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_EXIT_SUCCESS;
    else
        _state = STATE_EXIT_FAILED;

    return eq::net::COMMAND_HANDLED;
}


std::ostream& operator << ( std::ostream& os, const Pipe* pipe )
{
    if( !pipe )
        return os;
    
    os << disableFlush << disableHeader << "pipe" << endl;
    os << "{" << endl << indent;

    const std::string& name = pipe->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << endl;

    if( pipe->getPort() != EQ_UNDEFINED_UINT32 )
        os << "port     " << pipe->getPort() << endl;
        
    if( pipe->getDevice() != EQ_UNDEFINED_UINT32 )
        os << "device   " << pipe->getDevice() << endl;
    
    const eq::PixelViewport& pvp = pipe->getPixelViewport();
    if( pvp.isValid( ))
        os << "viewport " << pvp << endl;

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
            os << endl << "attributes" << endl;
            os << "{" << endl << indent;
            attrPrinted = true;
        }
        
        os << ( i==Pipe::IATTR_HINT_THREAD ? "hint_thread       " : "ERROR" )
           << static_cast<eq::IAttrValue>( value ) << endl;
    }
    
    if( attrPrinted )
        os << exdent << "}" << endl << endl;

    os << endl;

    const WindowVector& windows = pipe->getWindows();
    for( WindowVector::const_iterator i = windows.begin();
         i != windows.end(); ++i )

        os << *i;

    os << exdent << "}" << endl << enableHeader << enableFlush;
    return os;
}

}
}
