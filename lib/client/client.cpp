
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "client.h"

#include "commands.h"
#include "packets.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

Client::Client()
        : Node( CMD_CLIENT_ALL )
{
    //_cmdHandler[CMD_NODE_INIT] = &eq::Node::_cmdInit;
}

Client::~Client()
{
}

eqNet::CommandResult Client::handlePacket( eqNet::Node* node,
                                           const eqNet::Packet* packet )
{
    EQVERB << "handlePacket " << packet << endl;
    const uint32_t datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQ_SERVER:
            EQASSERT( dynamic_cast<Server*>(node) );

            Server* server = static_cast<Server*>(node);
            return server->handleCommand( node, packet );

        default:
            EQUNIMPLEMENTED;
            return eqNet::COMMAND_ERROR;
    }
}

