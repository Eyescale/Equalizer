
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "packets.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

Config::Config()
        : Session( CMD_CONFIG_ALL )
{
    registerCommand( CMD_CONFIG_INIT_REPLY, this, reinterpret_cast<CommandFcn>( 
                         &eq::Config::_cmdInitReply ));
    registerCommand( CMD_CONFIG_EXIT_REPLY, this, reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdExitReply ));
    registerCommand( CMD_CONFIG_FRAME_BEGIN_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdFrameBeginReply ));
    registerCommand( CMD_CONFIG_FRAME_END_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdFrameEndReply ));
}

bool Config::init()
{
    ConfigInitPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    _server->send( packet );
    return ( _requestHandler.waitRequest( packet.requestID ) != 0 );
}

bool Config::exit()
{
    ConfigExitPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    _server->send( packet );
    return ( _requestHandler.waitRequest( packet.requestID ) != 0 );
}

uint32_t Config::frameBegin()
{
    ConfigFrameBeginPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    _server->send( packet );
    return (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));
}

uint32_t Config::frameEnd()
{
    ConfigFrameEndPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    _server->send( packet );
    return (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Config::_cmdInitReply( eqNet::Node* node,
                                            const eqNet::Packet* pkg )
{
    ConfigInitReplyPacket* packet = (ConfigInitReplyPacket*)pkg;
    EQINFO << "handle init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}
eqNet::CommandResult Config::_cmdExitReply( eqNet::Node* node,
                                            const eqNet::Packet* pkg )
{
    ConfigExitReplyPacket* packet = (ConfigExitReplyPacket*)pkg;
    EQINFO << "handle exit reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFrameBeginReply(eqNet::Node* node,
                                                 const eqNet::Packet* pkg )
{
    ConfigFrameBeginReplyPacket* packet = (ConfigFrameBeginReplyPacket*)pkg;
    EQVERB << "handle frame begin reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFrameEndReply( eqNet::Node* node,
                                                const eqNet::Packet* pkg )
{
    ConfigFrameEndReplyPacket* packet = (ConfigFrameEndReplyPacket*)pkg;
    EQVERB << "handle frame end reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}
