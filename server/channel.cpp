
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "compound.h"
#include "config.h"
#include "window.h"

#include <eq/client/commands.h>
#include <eq/client/packets.h>
#include <eq/base/base.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Channel::_construct()
{
    _used             = 0;
    _window           = NULL;
    _pendingRequestID = EQ_INVALID_ID;
    _state            = STATE_STOPPED;

    registerCommand( eq::CMD_CHANNEL_INIT_REPLY, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Channel::_cmdInitReply ));
    registerCommand( eq::CMD_CHANNEL_EXIT_REPLY, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Channel::_cmdExitReply ));
}

Channel::Channel()
        : eqNet::Object( eq::Object::TYPE_CHANNEL, eq::CMD_CHANNEL_CUSTOM )
{
    _construct();
}

Channel::Channel( const Channel& from )
        : eqNet::Object( eq::Object::TYPE_CHANNEL, eq::CMD_CHANNEL_CUSTOM )
{
    _construct();
    _name = from._name;
    _vp   = from._vp;
    _pvp  = from._pvp;
}

void Channel::refUsed()
{
    _used++;
    if( _window ) 
        _window->refUsed(); 
}

void Channel::unrefUsed()
{
    EQASSERT( _used != 0 );
    _used--;
    if( _window ) 
        _window->unrefUsed(); 
}

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Channel::setPixelViewport( const eq::PixelViewport& pvp )
{
    if( !pvp.isValid( ))
        return;

    _pvp = pvp;
    _vp.invalidate();

    if( !_window )
        return;
    
    const eq::PixelViewport& windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
        _vp = pvp / windowPVP;
}

void Channel::setViewport( const eq::Viewport& vp )
{
    if( !vp.isValid( ))
        return;
    
    _vp = vp;
    _pvp.invalidate();

    if( !_window )
        return;

    eq::PixelViewport windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
    {
        windowPVP.x = 0;
        windowPVP.y = 0;
        _pvp = windowPVP * vp;
    }
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Channel::startInit( const uint32_t initID )
{
    _sendInit( initID );
}

void Channel::_sendInit( const uint32_t initID )
{
    EQASSERT( _pendingRequestID == EQ_INVALID_ID );

    eq::ChannelInitPacket packet( getSession()->getID(), getID( ));
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    packet.pvp        = _pvp; 
    packet.vp         = _vp;
    
    send( packet, getName( ));
    _state = STATE_INITIALISING;
}

bool Channel::syncInit()
{
    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_INVALID_ID;

    if( success )
        _state = STATE_RUNNING;
    else
        EQWARN << "Channel initialisation failed" << endl;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Channel::startExit()
{
    _state = STATE_STOPPING;
    _sendExit();
}

void Channel::_sendExit()
{
    EQASSERT( _pendingRequestID == EQ_INVALID_ID );

    eq::ChannelExitPacket packet( getSession()->getID(), getID( ));
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Channel::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_INVALID_ID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_INVALID_ID;

    _state = STATE_STOPPED;
    return success;
}


//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Channel::update( const uint32_t frameID )
{
    _pvp = _window->getPixelViewport();

    _pvp.x = 0;
    _pvp.y = 0;
    _pvp.applyViewport( _vp );

    Config*        config     = getConfig();
    const uint32_t nCompounds = config->nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound = config->getCompound( i );
        compound->updateChannel( this, frameID );
    }
}

//===========================================================================
// command handling
//===========================================================================
eqNet::CommandResult Channel::_cmdInitReply( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    eq::ChannelInitReplyPacket* packet = (eq::ChannelInitReplyPacket*)pkg;
    EQINFO << "handle channel init reply " << packet << endl;

    _near = packet->near;
    _far  = packet->far;
    setPixelViewport( packet->pvp );

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdExitReply( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    eq::ChannelExitReplyPacket* packet = (eq::ChannelExitReplyPacket*)pkg;
    EQINFO << "handle channel exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}


std::ostream& eqs::operator << ( std::ostream& os, const Channel* channel)
{
    if( !channel )
        return os;
    
    os << disableFlush << disableHeader << "channel" << endl;
    os << "{" << endl << indent;
    
    const std::string& name = channel->getName();
    if( name.empty( ))
        os << "name \"channel_" << (void*)channel << "\"" << endl;
    else
        os << "name \"" << name << "\"" << endl;

    const eq::Viewport& vp  = channel->getViewport();
    if( vp.isValid( ))
    {
        if( !vp.isFullScreen( ))
            os << "viewport " << vp << endl;
    }
    else
    {
        const eq::PixelViewport& pvp = channel->getPixelViewport();
        if( pvp.isValid( ))
            os << "viewport " << pvp << endl;
    }

    os << exdent << "}" << endl << enableHeader << enableFlush;

    return os;
}
