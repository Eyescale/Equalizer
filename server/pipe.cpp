
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "channel.h"
#include "config.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "window.h"

#include <eq/net/command.h>
#include <eq/client/commands.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_PIPE_") + #attr )
std::string Pipe::_iAttributeStrings[IATTR_ALL] = 
{
    MAKE_ATTR_STRING( IATTR_HINT_THREAD )
};

void Pipe::_construct()
{
    _used             = 0;
    _node             = 0;
    _pendingRequestID = EQ_ID_INVALID;
    _display          = EQ_UNDEFINED_UINT32;
    _screen           = EQ_UNDEFINED_UINT32;
    _state            = STATE_STOPPED;

    registerCommand( eq::CMD_PIPE_CONFIG_INIT_REPLY,
                  eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdConfigInitReply ));
    registerCommand( eq::CMD_PIPE_CONFIG_EXIT_REPLY, 
                  eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdConfigExitReply ));

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
{
    _construct();
    _display = from._display;
    _screen  = from._screen;
    _pvp     = from._pvp;

    for( int i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = from._iAttributes[i];

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

        window->_pipe = NULL;
        delete window;
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
// configInit
//---------------------------------------------------------------------------
void Pipe::startConfigInit( const uint32_t initID )
{
    _sendConfigInit( initID );

    Config*                    config = getConfig();
    eq::PipeCreateWindowPacket createWindowPacket;

    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
        {
            config->registerObject( window );
            createWindowPacket.windowID = window->getID();
            _send( createWindowPacket );

            window->startConfigInit( initID );
        }
    }
    _state = STATE_INITIALISING;
}

void Pipe::_sendConfigInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::PipeConfigInitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    packet.display    = _display;
    packet.screen     = _screen;
    packet.pvp        = _pvp;
    packet.threaded   = getIAttribute( IATTR_HINT_THREAD );

    _send( packet );
}

bool Pipe::syncConfigInit()
{
    bool      success  = true;
    string    error;
    const int nWindows = _windows.size();

    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->isUsed( ))
            if( !window->syncConfigInit( ))
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
// configExit
//---------------------------------------------------------------------------
void Pipe::startConfigExit()
{
    _state = STATE_STOPPING;
    const int nWindows = _windows.size();
    for( int i=0; i<nWindows; ++i )
    {
        Window* window = _windows[i];
        if( window->getState() == Window::STATE_STOPPED )
            continue;

        window->startConfigExit();
    }

    _sendConfigExit();
}

void Pipe::_sendConfigExit()
{
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::PipeConfigExitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    _send( packet );
}

bool Pipe::syncConfigExit()
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

        if( !window->syncConfigExit( ))
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
void Pipe::update( const uint32_t frameID, const uint32_t frameNumber )
{
    eq::PipeFrameStartPacket startPacket;
    startPacket.frameID     = frameID;
    startPacket.frameNumber = frameNumber;

    _send( startPacket );

    const uint32_t nWindows = this->nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window = getWindow( i );
        if( window->isUsed( ))
            window->updateDraw( frameID, frameNumber );
    }

    for( uint32_t i=0; i<nWindows; i++ )
    {
        Window* window = getWindow( i );
        if( window->isUsed( ))
            window->updatePost( frameID, frameNumber );
    }

    eq::PipeFrameFinishPacket finishPacket;
    finishPacket.frameID     = frameID;
    finishPacket.frameNumber = frameNumber;
    _send( finishPacket );
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
eqNet::CommandResult Pipe::_cmdConfigInitReply( eqNet::Command& command ) 
{
    const eq::PipeConfigInitReplyPacket* packet = 
        command.getPacket<eq::PipeConfigInitReplyPacket>();
    EQINFO << "handle pipe configInit reply " << packet << endl;

    _error = packet->error;
    setPixelViewport( packet->pvp );

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdConfigExitReply( eqNet::Command& command ) 
{
    const eq::PipeConfigExitReplyPacket* packet = 
        command.getPacket<eq::PipeConfigExitReplyPacket>();
    EQINFO << "handle pipe configExit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
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
    const uint32_t nWindows = pipe->nWindows();
    for( uint32_t i=0; i<nWindows; i++ )
        os << pipe->getWindow(i);
    
    os << exdent << "}" << endl << enableHeader << enableFlush;
    return os;
}
