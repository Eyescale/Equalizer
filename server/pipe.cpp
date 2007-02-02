
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "log.h"
#include "node.h"
#include "window.h"

#include <eq/net/command.h>
#include <eq/client/commands.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Pipe::_construct()
{
    _used             = 0;
    _node             = 0;
    _currentWindow    = 0;
    _pendingRequestID = EQ_ID_INVALID;
    _display          = EQ_UNDEFINED_UINT32;
    _screen           = EQ_UNDEFINED_UINT32;
    _state            = STATE_STOPPED;

    registerCommand( eq::CMD_PIPE_INIT_REPLY,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdInitReply ));
    registerCommand( eq::CMD_PIPE_EXIT_REPLY, 
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdExitReply ));
    registerCommand( eq::CMD_PIPE_FRAME_SYNC_REPLY, 
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdFrameSync ));

    ref(); // We don't use RefPtr so far
    EQINFO << "New pipe @" << (void*)this << endl;
}

Pipe::Pipe()
{
    _construct();
}

Pipe::Pipe( const Pipe& from )
{
    _construct();
    _display = from._display;
    _screen  = from._screen;
    _pvp     = from._pvp;

    const uint32_t nWindows = from.nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window      = from.getWindow(i);
        Window* windowClone = new Window( *window );
            
        addWindow( windowClone );
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
        EQASSERT( window->getRefCount() == 1 );

        window->_pipe = NULL;
        window->unref(); // a.k.a delete
    }
    _windows.clear();
}

void Pipe::addWindow( Window* window )
{
    _windows.push_back( window ); 
    window->_pipe = this; 
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

bool Pipe::testMakeCurrentWindow( const Window* window )
{
    if( _currentWindow == window )
        return false;

    _currentWindow = window;
    return true;
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

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Pipe::startInit( const uint32_t initID )
{
    _sendInit( initID );

    Config*                    config = getConfig();
    eq::PipeCreateWindowPacket createWindowPacket;

    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
        {
            config->registerObject( window, (eqNet::Node*)getServer( ));
            createWindowPacket.windowID = window->getID();
            _send( createWindowPacket );

            window->startInit( initID );
        }
    }
    _state = STATE_INITIALISING;
}

void Pipe::_sendInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::PipeInitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    packet.display    = _display;
    packet.screen     = _screen;
    packet.pvp        = _pvp;

    _send( packet );
}

bool Pipe::syncInit()
{
    bool      success  = true;
    string    error;
    const int nWindows = _windows.size();

    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
            if( !window->syncInit( ))
            {
                error += (' ' + window->getErrorMessage( ));
                success = false;
            }
    }

    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    if( !(bool)_requestHandler.waitRequest( _pendingRequestID ))
        success = false;

    _pendingRequestID = EQ_ID_INVALID;
    _error += (' ' + error);

    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Pipe initialisation failed: " << _error << endl;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Pipe::startExit()
{
    _state = STATE_STOPPING;
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->getState() == Window::STATE_STOPPED )
            continue;

        window->startExit();
    }

    _sendExit();
}

void Pipe::_sendExit()
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::PipeExitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
}

bool Pipe::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_ID_INVALID;

    Config* config = getConfig();
    eq::PipeDestroyWindowPacket destroyWindowPacket;

    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->getState() != Window::STATE_STOPPING )
            continue;

        if( !window->syncExit( ))
            success = false;

        destroyWindowPacket.windowID = window->getID();
        _send( destroyWindowPacket );
        config->deregisterObject( window );
    }

    _state = STATE_STOPPED;
    return success;
}

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint32_t frameID )
{
    eq::PipeUpdatePacket updatePacket;
    updatePacket.frameID     = frameID;

    _send( updatePacket );

    const uint32_t nWindows = this->nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window = getWindow( i );
        if( window->isUsed( ))
            window->update( frameID );
    }

    eq::PipeFrameSyncPacket syncPacket;
    syncPacket.frameID     = frameID;
    syncPacket.frameNumber = getConfig()->getFrameNumber();
    _send( syncPacket );
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

    for( std::vector<Window*>::iterator iter = _windows.begin(); 
         iter != _windows.end(); ++iter )

        (*iter)->notifyViewportChanged();
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Pipe::_cmdInitReply( eqNet::Command& command ) 
{
    const eq::PipeInitReplyPacket* packet = 
        command.getPacket<eq::PipeInitReplyPacket>();
    EQINFO << "handle pipe init reply " << packet << endl;

    _error = packet->error;
    setPixelViewport( packet->pvp );

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdExitReply( eqNet::Command& command ) 
{
    const eq::PipeExitReplyPacket* packet = 
        command.getPacket<eq::PipeExitReplyPacket>();
    EQINFO << "handle pipe exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}


eqNet::CommandResult Pipe::_cmdFrameSync( eqNet::Command& command )
{
    const eq::PipeFrameSyncReplyPacket* packet = 
        command.getPacket<eq::PipeFrameSyncReplyPacket>();
    EQVERB << "handle frame sync " << packet << endl;
    
    // Move me
    for( uint32_t i =0; i<packet->nStatEvents; ++i )
    {
        const eq::StatEvent& event = packet->statEvents[i];
        EQLOG( LOG_STATS ) << event << endl;
    }

    _frameFinished = packet->frameNumber;
    return eqNet::COMMAND_HANDLED;
}


std::ostream& eqs::operator << ( std::ostream& os, const Pipe* pipe )
{
    if( !pipe )
        return os;
    
    os << disableFlush << disableHeader << "pipe" << endl;
    os << "{" << endl << indent;

    if( pipe->getDisplay() != EQ_UNDEFINED_UINT32 )
        os << "display  " << pipe->getDisplay() << endl;
        
    if( pipe->getScreen() != EQ_UNDEFINED_UINT32 )
        os << "screen   " << pipe->getScreen() << endl;
    
    const eq::PixelViewport& pvp = pipe->getPixelViewport();
    if( pvp.isValid( ))
        os << "viewport " << pvp << endl;

    os << endl;
    const uint32_t nWindows = pipe->nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
        os << pipe->getWindow(i);
    
    os << exdent << "}" << endl << enableHeader << enableFlush;
    return os;
}
