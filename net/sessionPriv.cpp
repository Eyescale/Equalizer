
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sessionPriv.h"

#include "connection.h"
#include "networkPriv.h"
#include "nodePriv.h"
#include "server.h"
#include "serverPriv.h"

#include <eq/base/log.h>
#include <eq/net/connectionDescription.h>

#include <alloca.h>

using namespace eqNet::priv;
using namespace std;

Session::Session(const uint id, Server* server, const bool onServer )
        : eqNet::Session(id),
          _networkID(1),
          _nodeID(NODE_ID_SERVER+1),
          _localNode(NULL)
{
    _nodes[server->getID()] = server;

    if( onServer )
        _localNode = server;
    else
        _localNode = newNode();
}

Session* Session::create( const char* serverAddress )
{
    Server* server = Server::connect( serverAddress );
    if( !server )
        return NULL;

    return server->getSession(0);
}

Network* Session::newNetwork( const NetworkProtocol protocol )
{
    Network* network = Network::create( _networkID++, PROTO_TCPIP );
    _networks[network->getID()] = network;
    return network;
}

bool Session::deleteNetwork( Network* network )
{
    const uint networkID = network->getID();

    IDHash<Network*>::iterator iter = _networks.find( networkID );
    if( iter == _networks.end() || (*iter).second != network )
        return false;

    int nErased = _networks.erase( networkID );
    ASSERT( nErased == 1 );
    delete network;
    return true;
}

Node* Session::newNode()
{
    Node* node = new Node(_nodeID++);
    _nodes[node->getID()] = node;
    return node;
}

