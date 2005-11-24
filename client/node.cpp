
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "commands.h"
#include "packets.h"
#include "server.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

static bool _firstNode = true;

Node::Node()
        : _id(INVALID_ID),
          _config(NULL)
{
    for( int i=0; i<CMD_NODE_ALL; i++ )
        _cmdHandler[i] =  &eq::Node::_cmdUnknown;

     _cmdHandler[CMD_NODE_INIT] = &eq::Node::_cmdInit;
}

Node::~Node()
{
}

eqBase::RefPtr<eqNet::Node> Node::createNode()
{ 
    // The first node in a render node's life cycle is the server. There must be
    // a cleaner way to instantiate it...
    if( _firstNode ) 
    {
        _firstNode = false;
        _server    = new Server;
        return _server.get();
    }

    return new Node; 
}

void Node::handlePacket( eqNet::Node* node, const eqNet::Packet* packet )
{
    VERB << "handlePacket " << packet << endl;
    const uint datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQ_NODE:
            ASSERT( packet->command <  CMD_NODE_ALL );

            (this->*_cmdHandler[packet->command])( node, packet );
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

void Node::_cmdInit( eqNet::Node* node, const eqNet::Packet* pkg )
{
    NodeInitPacket* packet = (NodeInitPacket*)pkg;
    ERROR << "handle node init " << packet << endl;

    _id     = packet->nodeID;
    _config = new Config( packet->configID, _server.get() );
    
    NodeInitReplyPacket reply( packet );
    reply.result = init();
    node->send( reply );
}
