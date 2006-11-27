
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "compound.h"
#include "config.h"
#include "window.h"

#include <eq/base/base.h>
#include <eq/net/command.h>
#include <eq/client/commands.h>
#include <eq/client/packets.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

void Channel::_construct()
{
    _used             = 0;
    _window           = NULL;
    _pendingRequestID = EQ_ID_INVALID;
    _state            = STATE_STOPPED;
    _fixedPVP         = false;

    registerCommand( eq::CMD_CHANNEL_INIT_REPLY, 
                  eqNet::CommandFunc<Channel>( this, &Channel::_cmdInitReply ));
    registerCommand( eq::CMD_CHANNEL_EXIT_REPLY,
                  eqNet::CommandFunc<Channel>( this, &Channel::_cmdExitReply ));
    registerCommand( eq::CMD_CHANNEL_SET_NEARFAR,
                     eqNet::CommandFunc<Channel>( this, &Channel::_cmdPush ));
    registerCommand( eq::REQ_CHANNEL_SET_NEARFAR,
                 eqNet::CommandFunc<Channel>( this, &Channel::_reqSetNearFar ));

    ref(); // We don't use RefPtr so far
    EQINFO << "New channel @" << (void*)this << endl;
}

Channel::Channel()
        : eqNet::Object( eq::Object::TYPE_CHANNEL )
{
    _construct();
}

Channel::Channel( const Channel& from )
        : eqNet::Object( eq::Object::TYPE_CHANNEL )
{
    _construct();
    _name     = from._name;
    _vp       = from._vp;
    _pvp      = from._pvp;
    _fixedPVP = from._fixedPVP;
}

Channel::~Channel()
{
    EQINFO << "Delete channel @" << (void*)this << endl;

    if( _window )
        _window->removeChannel( this );
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
    if( !pvp.hasArea( ))
        return;
    
    _fixedPVP = true;

    if( pvp == _pvp )
        return;

    _pvp = pvp;
    _vp.invalidate();
    notifyWindowPVPChanged();
}

void Channel::setViewport( const eq::Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
     
    _fixedPVP = false;

    if( vp == _vp )
        return;

    _vp = vp;
    _pvp.invalidate();
    notifyWindowPVPChanged();
}

void Channel::notifyWindowPVPChanged()
{
    if( !_window )
        return;

    const eq::PixelViewport& windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    if( _fixedPVP )
    {
        _vp = _pvp / windowPVP;
    }
    else
    {
        eq::PixelViewport relWindowPVP = windowPVP;
        relWindowPVP.x = 0;
        relWindowPVP.y = 0;
        _pvp = relWindowPVP * _vp;
    }
    EQINFO << "Channel viewport update: " << _pvp << ":" << _vp << endl;
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
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::ChannelInitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    packet.initID     = initID;
    packet.pvp        = _pvp; 
    packet.vp         = _vp;
    
    Object::send( _getNetNode(), packet, _name );
    _state = STATE_INITIALIZING;
}

bool Channel::syncInit()
{
    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_ID_INVALID;

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
    EQASSERT( _pendingRequestID == EQ_ID_INVALID );

    eq::ChannelExitPacket packet;
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    Object::send( _getNetNode(), packet );
}

bool Channel::syncExit()
{
    EQASSERT( _pendingRequestID != EQ_ID_INVALID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_ID_INVALID;

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
eqNet::CommandResult Channel::_cmdInitReply( eqNet::Command& command ) 
{
    const eq::ChannelInitReplyPacket* packet = 
        command.getPacket<eq::ChannelInitReplyPacket>();
    EQINFO << "handle channel init reply " << packet << endl;

    _near = packet->near;
    _far  = packet->far;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_cmdExitReply( eqNet::Command& command ) 
{
    const eq::ChannelExitReplyPacket* packet = 
        command.getPacket<eq::ChannelExitReplyPacket>();
    EQINFO << "handle channel exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Channel::_reqSetNearFar( eqNet::Command& command )
{
    const eq::ChannelSetNearFarPacket* packet = 
        command.getPacket<eq::ChannelSetNearFarPacket>();
    _near = packet->near;
    _far  = packet->far;
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
