
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "window.h"

#include <eq/client/commands.h>

using namespace eqs;
using namespace std;

void Pipe::_construct()
{
    _used             = 0;
    _node             = NULL;
    _pendingRequestID = EQ_INVALID_ID;
    _display          = EQ_UNDEFINED;
    _screen           = EQ_UNDEFINED;

    registerCommand( eq::CMD_PIPE_INIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Pipe::_cmdInitReply ));
    registerCommand( eq::CMD_PIPE_EXIT_REPLY, this,reinterpret_cast<CommandFcn>(
                         &eqs::Pipe::_cmdExitReply ));
    registerCommand( eq::CMD_PIPE_FRAME_SYNC, this,reinterpret_cast<CommandFcn>(
                         &eqs::Pipe::_cmdFrameSync ));
}

Pipe::Pipe()
        : eqNet::Base( eq::CMD_PIPE_ALL )
{
    _construct();
}

Pipe::Pipe( const Pipe& from )
        : eqNet::Base( eq::CMD_PIPE_ALL )
{
    _construct();

    const uint32_t nWindows = from.nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window      = from.getWindow(i);
        Window* windowClone = new Window( *window );
            
        addWindow( windowClone );
    }    
}


void Pipe::addWindow( Window* window )
{
    _windows.push_back( window ); 
    window->_pipe = this; 

    Config* config = getConfig();
    if( !config )
        return;

    config->registerObject( window );
    const int nChannels = window->nChannels();
    for( int i = 0; i<nChannels; ++i )
    {
        Channel* channel = window->getChannel( i );
        config->registerObject( channel );
    }
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

    eq::PipeCreateWindowPacket createWindowPacket( getSession()->getID(),
                                                   getID( ));
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
        {
            createWindowPacket.windowID = window->getID();
            _send( createWindowPacket );
            window->startInit( initID );
        }
    }
}

void Pipe::_sendInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_INVALID_ID );

    eq::PipeInitPacket packet( getSession()->getID(), getID( ));
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
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

    EQASSERT( _pendingRequestID != EQ_INVALID_ID );

    if( !(bool)_requestHandler.waitRequest( _pendingRequestID ))
        success = false;
    _pendingRequestID = EQ_INVALID_ID;

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
    EQASSERT( _pendingRequestID == EQ_INVALID_ID );

    eq::PipeExitPacket packet( getSession()->getID(), getID( ));
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
}

bool Pipe::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_INVALID_ID );

    bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_INVALID_ID;

    eq::PipeDestroyWindowPacket destroyWindowPacket( getSession()->getID(), 
                                                     getID( ));
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

//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Pipe::update( const uint32_t frameID )
{
    const uint32_t nWindows = this->nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window = getWindow( i );
        if( window->isUsed( ))
            window->update( frameID );
    }

    eq::PipeFrameSyncPacket packet( getSession()->getID(), getID( ));
    _send( packet );
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Pipe::_cmdInitReply( eqNet::Node* node, 
                                          const eqNet::Packet* pkg )
{
    eq::PipeInitReplyPacket* packet = (eq::PipeInitReplyPacket*)pkg;
    EQINFO << "handle pipe init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdExitReply( eqNet::Node* node, 
                                          const eqNet::Packet* pkg )
{
    eq::PipeExitReplyPacket* packet = (eq::PipeExitReplyPacket*)pkg;
    EQINFO << "handle pipe exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}


eqNet::CommandResult Pipe::_cmdFrameSync( eqNet::Node* node,
                                          const eqNet::Packet* pkg )
{
    EQVERB << "handle frame sync " << pkg << endl;
    _frameSync.post();
    return eqNet::COMMAND_HANDLED;
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
