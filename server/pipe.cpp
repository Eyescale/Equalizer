
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "config.h"
#include "node.h"
#include "window.h"

#include <eq/commands.h>

using namespace eqs;
using namespace std;

Pipe::Pipe()
        : eqNet::Base( eq::CMD_PIPE_ALL ),
          _used( 0 ),
          _node( NULL ),
          _pendingRequestID(INVALID_ID),
          _display(EQ_UNDEFINED_UINT32),
          _screen(EQ_UNDEFINED_UINT32)
{
    registerCommand( eq::CMD_PIPE_INIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Pipe::_cmdInitReply ));
    registerCommand( eq::CMD_PIPE_EXIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Pipe::_cmdExitReply ));
}

void Pipe::addWindow( Window* window )
{
    _windows.push_back( window ); 
    window->_pipe = this; 
    getConfig()->registerObject( window );
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
void Pipe::startInit()
{
    _sendInit();

    eq::PipeCreateWindowPacket createWindowPacket( _sessionID, _id );
    
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
        {
            createWindowPacket.windowID = window->getID();
            _send( createWindowPacket );
            window->startInit();
        }
    }
}

void Pipe::_sendInit()
{
    ASSERT( _pendingRequestID == INVALID_ID );

    eq::PipeInitPacket packet( _sessionID, _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.display    = _display;
    packet.screen     = _screen;
    _send( packet );
}

bool Pipe::syncInit()
{
    bool success = true;
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
            if( !window->syncInit( ))
                success = false;
    }

    ASSERT( _pendingRequestID != INVALID_ID );

    if( !(bool)_requestHandler.waitRequest( _pendingRequestID ))
        success = false;
    _pendingRequestID = INVALID_ID;

    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Pipe::startExit()
{
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
            window->startExit();
    }

    _sendExit();
}

void Pipe::_sendExit()
{
    ASSERT( _pendingRequestID == INVALID_ID );

    eq::PipeExitPacket packet( _sessionID, _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
}

bool Pipe::syncExit()
{
    ASSERT( _pendingRequestID != INVALID_ID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;

    eq::PipeDestroyWindowPacket destroyWindowPacket( _sessionID, _id );

    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
        {
            if( !window->syncExit( ))
                success = false;

            destroyWindowPacket.windowID = window->getID();
            _send( destroyWindowPacket );
        }
    }

    return success;
}
//===========================================================================
// command handling
//===========================================================================
void Pipe::_cmdInitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::PipeInitReplyPacket* packet = (eq::PipeInitReplyPacket*)pkg;
    INFO << "handle pipe init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
}

void Pipe::_cmdExitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::PipeExitReplyPacket* packet = (eq::PipeExitReplyPacket*)pkg;
    INFO << "handle pipe exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
}



std::ostream& eqs::operator << ( std::ostream& os, const Pipe* pipe )
{
    if( !pipe )
    {
        os << "NULL pipe";
        return os;
    }
    
    const uint32_t nWindows = pipe->nWindows();
    os << "pipe " << (void*)pipe << ( pipe->isUsed() ? " used " : " unused " )
       << nWindows << " windows";
    
    for( uint32_t i=0; i<nWindows; i++ )
        os << std::endl << "    " << pipe->getWindow(i);
    
    return os;
}
