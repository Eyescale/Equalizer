
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "commands.h"
#include "packets.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

void Node::handleCommand( eqNet::Node* node, const eqNet::NodePacket* packet )
{
    VERB << "handleCommand " << packet << endl;

    if( packet->command >= CMD_SERVER_ALL )
    {
        ERROR << "Unknown command " << packet->command << endl;
        return;
    }

    ASSERT( dynamic_cast<Server*>(node) );

    Server* server = static_cast<Server*>(node);
    server->handleCommand( packet );
}

void Node::handlePacket( eqNet::Node* node, const eqNet::Packet* packet )
{
    VERB << "handlePacket " << packet << endl;
    const uint datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQ_CONFIG:
            ASSERT( dynamic_cast<Server*>(node) );

            Server* server = static_cast<Server*>(node);
            server->handlePacket( packet );
            break;

        default:
            ERROR << "unimplemented" << endl;
            abort();
    }
}

