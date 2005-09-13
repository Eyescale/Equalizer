
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "commands.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

Node* Node::_localNode = new Node();

void Node::handleCommand( eqNet::Node* node, const eqNet::NodePacket* packet )
{
    INFO << "handle " << packet << endl;

    if( packet->command >= CMD_SERVER_ALL )
    {
        ERROR << "Unknown command " << packet->command << endl;
        return;
    }

    Server* server = dynamic_cast<Server*>(node);
    if( !server )
    {
        ERROR << "Received server message from non-server node." << endl;
        return;
    }

    server->handleCommand( packet );
}

