
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

    registerCommand( eq::CMD_CHANNEL_INIT_REPLY, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Channel::_cmdInitReply ));
    registerCommand( eq::CMD_CHANNEL_EXIT_REPLY, this,
                     reinterpret_cast<CommandFcn>(
                         &eqs::Channel::_cmdExitReply ));
}

Channel::Channel()
        : eqNet::Base( eq::CMD_CHANNEL_ALL )
{
    _construct();
}

Channel::Channel( const Channel& from )
        : eqNet::Base( eq::CMD_CHANNEL_ALL )
{
    _construct();
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
    send( packet );
}

bool Channel::syncInit()
{
    const bool success = (bool)_requestHandler.waitRequest( _pendingRequestID );
    _pendingRequestID = EQ_INVALID_ID;
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

    _near = packet->_near;
    _far  = packet->_far;

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
    
    os << "channel" << endl;
    os << "{" << endl << indent;
    os << exdent << "}" << endl;

    return os;
}
