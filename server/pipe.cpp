
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>
#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "window.h"

#include <eq/net/command.h>
#include <eq/client/commands.h>

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
    MAKE_ATTR_STRING( IATTR_HINT_THREAD )
};

void Pipe::_construct()
{
    _used           = 0;
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

Pipe::Pipe( const Pipe& from )
        : eq::net::Object()
{
    _construct();

    _name   = from._name;
    _port   = from._port;
    _device = from._device;
    _pvp    = from._pvp;

    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

    const WindowVector& windows = from.getWindows();
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        addWindow( new Window( **i ));
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
    _windows.push_back( window ); 
    window->_pipe = this;
    window->notifyViewportChanged();
}

bool Pipe::removeWindow( Window* window )
{
    vector<Window*>::iterator i = find( _windows.begin(), _windows.end(),
                                        window );
    if( i == _windows.end( ))
        return false;

    _windows.erase( i );
    window->_pipe = 0;
    return true;
}

VisitorResult Pipe::accept( PipeVisitor* visitor )
{ 
    VisitorResult result = visitor->visitPre( this );
    if( result != TRAVERSE_CONTINUE )
        return result;

    for( WindowVector::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        switch( window->accept( visitor ))
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

    switch( visitor->visitPost( this ))
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

void Pipe::refUsed()
{
    _used++;
    if( _node ) 
        _node->refUsed(); 
}
void Pipe::unrefUsed()
{
    _used--;
    if( _node ) 
        _node->unrefUsed(); 
}

void Pipe::addTasks( const uint32_t tasks )
{
    EQASSERT( _node );
    _tasks |= tasks;
    _node->addTasks( tasks );
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// configInit
//---------------------------------------------------------------------------
void Pipe::startConfigInit( const uint32_t initID )
{
    EQASSERT( _state == STATE_STOPPED );
    _state         = STATE_INITIALIZING;

    _sendConfigInit( initID );

    Config*                    config = getConfig();
    eq::PipeCreateWindowPacket createWindowPacket;

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;

        if( window->isUsed( ))
        {
            config->registerObject( window );
            createWindowPacket.windowID = window->getID();
            _send( createWindowPacket );

            window->startConfigInit( initID );
        }
    }
}

void Pipe::_sendConfigInit( const uint32_t initID )
{
    eq::PipeConfigInitPacket packet;
    packet.initID = initID;
    packet.port   = _port;
    packet.device = _device;
    packet.tasks  = _tasks;
    packet.pvp    = _pvp;

    _send( packet, _name );
    EQLOG( eq::LOG_TASKS ) << "TASK pipe configInit  " << &packet << endl;
}

bool Pipe::syncConfigInit()
{
    bool      success  = true;

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isUsed( ))
            if( !window->syncConfigInit( ))
            {
                _error += " window " + window->getName() + ": '" +
                          window->getErrorMessage() + '\'';
                success = false;
            }
    }

    EQASSERT( _state == STATE_INITIALIZING || _state == STATE_RUNNING ||
              _state == STATE_INIT_FAILED );
    _state.waitNE( STATE_INITIALIZING );
    if( _state == STATE_INIT_FAILED )
        success = false;

    if( !success )
        EQWARN << "Pipe initialisation failed: " << _error << endl;
    return success;
}

//---------------------------------------------------------------------------
// configExit
//---------------------------------------------------------------------------
void Pipe::startConfigExit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_INIT_FAILED );
    _state = STATE_STOPPING;
    _tasks = eq::TASK_NONE;

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->getState() == Window::STATE_STOPPED )
            continue;

        window->startConfigExit();
    }

    _sendConfigExit();
}

void Pipe::_sendConfigExit()
{
    eq::PipeConfigExitPacket packet;
    _send( packet );
}

bool Pipe::syncConfigExit()
{
    EQASSERT( _state == STATE_STOPPING || _state == STATE_STOPPED || 
              _state == STATE_STOP_FAILED );
    
    _state.waitNE( STATE_STOPPING );
    bool success = ( _state == STATE_STOPPED );
    EQASSERT( success || _state == STATE_STOP_FAILED );
    _state = STATE_STOPPED; /// STOP_FAILED -> STOPPED transition

    Config* config = getConfig();
    eq::PipeDestroyWindowPacket destroyWindowPacket;

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->getID() == EQ_ID_INVALID )
            continue;

        if( !window->syncConfigExit( ))
            success = false;

        destroyWindowPacket.windowID = window->getID();
        _send( destroyWindowPacket );
        config->deregisterObject( window );
    }

    if( getIAttribute( IATTR_HINT_THREAD ))
    {   // Note: thread is started by NodeCreatePipePacket
        eq::PipeStopThreadPacket packet;
        _send( packet );
    }

    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint32_t frameID, const uint32_t frameNumber )
{
    eq::PipeFrameStartClockPacket startClockPacket;
    _send( startClockPacket );

    if( !_lastDrawWindow ) // happens when all used channels skip a frame
    {
        _lastDrawWindow = _windows[0];
        
        eq::PipeFrameNoDrawPacket packet;
        packet.frameID     = frameID;
        packet.frameNumber = frameNumber;
        _send( packet );
        EQLOG( eq::LOG_TASKS ) << "TASK pipe no draw " << &packet << endl;
    }

    eq::PipeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;

    _send( startPacket );
    EQLOG( eq::LOG_TASKS ) << "TASK pipe start frame " << &startPacket << endl;

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isUsed( ))
            window->updateDraw( frameID, frameNumber );
    }

    for( vector< Window* >::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->isUsed( ))
            window->updatePost( frameID, frameNumber );
    }

    eq::PipeFrameFinishPacket finishPacket;
    finishPacket.frameID      = frameID;
    finishPacket.frameNumber  = frameNumber;

    _send( finishPacket );
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
    EQINFO << "handle pipe configInit reply " << packet << endl;

    _error = packet->error;
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
    EQINFO << "handle pipe configExit reply " << packet << endl;

    if( packet->result )
        _state = STATE_STOPPED;
    else
        _state = STATE_STOP_FAILED;

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
