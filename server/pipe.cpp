
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "window.h"

#include <eq/client/commands.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Pipe::_construct()
{
    _used             = 0;
    _node             = NULL;
    _pendingRequestID = EQ_ID_INVALID;
    _display          = EQ_UNDEFINED_UINT32;
    _screen           = EQ_UNDEFINED_UINT32;
    _state            = STATE_STOPPED;

    registerCommand( eq::CMD_PIPE_INIT_REPLY,
                     eqNet::PacketFunc<Pipe>( this, &Pipe::_cmdInitReply ));
    registerCommand( eq::CMD_PIPE_EXIT_REPLY, 
                     eqNet::PacketFunc<Pipe>( this, &Pipe::_cmdExitReply ));
    registerCommand( eq::CMD_PIPE_FRAME_SYNC, 
                     eqNet::PacketFunc<Pipe>( this, &Pipe::_cmdFrameSync ));
    EQINFO << "New pipe @" << (void*)this << endl;
}

Pipe::Pipe()
        : eqNet::Object( eq::Object::TYPE_PIPE )
{
    _construct();
}

Pipe::Pipe( const Pipe& from )
        : eqNet::Object( eq::Object::TYPE_PIPE )
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


void Pipe::addWindow( Window* window )
{
    _windows.push_back( window ); 
    window->_pipe = this; 
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
    bool success = true;
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
            if( !window->syncInit( ))
                success = false;
    }

    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    if( !(bool)_requestHandler.waitRequest( _pendingRequestID ))
        success = false;
    _pendingRequestID = EQ_ID_INVALID;

    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Pipe initialisation failed" << endl;
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
    updatePacket.frameID = frameID;
    _send( updatePacket );

    const uint32_t nWindows = this->nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window = getWindow( i );
        if( window->isUsed( ))
            window->update( frameID );
    }

    eq::PipeFrameSyncPacket syncPacket;
    syncPacket.frameID = frameID;
    _send( syncPacket );
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
