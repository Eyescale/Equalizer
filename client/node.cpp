
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "commands.h"
#include "packets.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

Node::Node()
{
    for( int i=0; i<CMD_NODE_ALL; i++ )
        _cmdHandler[i] =  &eq::Node::_cmdUnknown;

     _cmdHandler[CMD_NODE_INIT] = &eq::Node::_cmdInit;
}

Node::~Node()
{
}

void Node::handlePacket( eqNet::Node* node, const eqNet::Packet* packet )
{
    VERB << "handlePacket " << packet << endl;
    const uint datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQ_NODE:
            _handleCommand( packet );
            break;

        case DATATYPE_EQ_SERVER:
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

void Node::_handleCommand( const eqNet::Packet* packet )
{
    VERB << "handleCommand " << packet << endl;
    ASSERT( packet->datatype == DATATYPE_EQ_NODE );
    ASSERT( packet->command <  CMD_NODE_ALL );

    (this->*_cmdHandler[packet->command])(packet);
}

void Node::_cmdUnknown( const eqNet::Packet* pkg )
{
    ERROR << "Unknown packet " << pkg << endl;
}

