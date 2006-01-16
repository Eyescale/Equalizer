
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "compound.h"
#include "config.h"
#include "window.h"

#include <eq/commands.h>
#include <eq/packets.h>
#include <eq/base/base.h>

using namespace eqs;
using namespace eqBase;
using namespace std;

Channel::Channel()
        : eqNet::Base( eq::CMD_CHANNEL_ALL ),
          _used(0),
          _window(NULL),
          _pendingRequestID(INVALID_ID)
{
    registerCommand( eq::CMD_CHANNEL_INIT_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqs::Channel::_cmdInitReply ));
    registerCommand( eq::CMD_CHANNEL_EXIT_REPLY, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Channel::_cmdExitReply ));
}

void Channel::refUsed()
{
    _used++;
    if( _window ) 
        _window->refUsed(); 
}

void Channel::unrefUsed()
{
    ASSERT( _used != 0 );
    _used--;
    if( _window ) 
        _window->unrefUsed(); 
}

//===========================================================================
// Operations
//===========================================================================

//---------------------------------------------------------------------------
// init
//---------------------------------------------------------------------------
void Channel::startInit()
{
    _sendInit();
}

void Channel::_sendInit()
{
    ASSERT( _pendingRequestID == INVALID_ID );

    eq::ChannelInitPacket packet( _sessionID, _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Channel::syncInit()
{
    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    return success;
}

//---------------------------------------------------------------------------
// exit
//---------------------------------------------------------------------------
void Channel::startExit()
{
    _sendExit();
}

void Channel::_sendExit()
{
    ASSERT( _pendingRequestID == INVALID_ID );

    eq::ChannelExitPacket packet( _sessionID, _id );
    _pendingRequestID = _requestHandler.registerRequest(); 
    packet.requestID  = _pendingRequestID;
    send( packet );
}

bool Channel::syncExit()
{
    ASSERT( _pendingRequestID != INVALID_ID );

    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = INVALID_ID;
    return success;
}


//---------------------------------------------------------------------------
// update
//---------------------------------------------------------------------------
void Channel::update()
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
        compound->updateChannel( this );
    }
}

//===========================================================================
// command handling
//===========================================================================
void Channel::_cmdInitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ChannelInitReplyPacket* packet = (eq::ChannelInitReplyPacket*)pkg;
    INFO << "handle channel init reply " << packet << endl;

    _near = packet->near;
    _far  = packet->far;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->result );
}

void Channel::_cmdExitReply( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ChannelExitReplyPacket* packet = (eq::ChannelExitReplyPacket*)pkg;
    INFO << "handle channel exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)true );
}



std::ostream& eqs::operator << ( std::ostream& os, const Channel* channel)
{
    if( !channel )
    {
        os << "NULL channel";
            return os;
    }
    
    os << "channel " << (void*)channel
       << ( channel->isUsed() ? " used" : " unused" );
    return os;
}
