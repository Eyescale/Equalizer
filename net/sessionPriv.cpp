
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sessionPriv.h"
#include "connection.h"
#include "connectionDescription.h"
#include "networkPriv.h"
#include "nodePriv.h"
#include "nodeList.h"
#include "packet.h"
#include "server.h"
#include "serverPriv.h"
#include "util.h"

#include <eq/base/log.h>

#include <alloca.h>

using namespace eqNet::priv;
using namespace std;

Session::Session(const uint id, Server* server )
        : eqNet::Session(id),
          _networkID(1),
          _server(server),
          _serverID(server->getID()),
          _nodeID(server->getID()+1),
          _localNode(NULL),
          _localNodeID(INVALID_ID)
{
    _nodes[server->getID()] = server;
    INFO << "New session" << this;
}

Session* Session::create( const char* serverAddress )
{
    return Server::createSession( serverAddress );
}

Network* Session::newNetwork( const NetworkProtocol protocol )
{
    Network* network = Network::create( _networkID++, this, PROTO_TCPIP );
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
    Node* node = new Node( _nodeID++ );
    _nodes[node->getID()] = node;
    return node;
}

void Session::setLocalNode( const uint nodeID )
{
    IDHash<Node*>::iterator iter = _nodes.find( nodeID );
    ASSERT( iter != _nodes.end( ));
    _localNode = (*iter).second;
}


void Session::pack( const NodeList& nodes, const bool initial )
{
    //if( initial )

    SessionNewPacket sessionNewPacket;
    const uint       sessionID = getID();
    sessionNewPacket.id = sessionID;
    sessionNewPacket.serverID = _server->getID();
    nodes.send( _localNode, sessionNewPacket );
    
    for( IDHash<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         iter++ )
    {
        NodeNewPacket nodeNewPacket;
        nodeNewPacket.id        = (*iter).first;
        nodeNewPacket.sessionID = sessionID;
        nodes.send( _localNode, nodeNewPacket );
    };

    for( IDHash<Network*>::iterator iter = _networks.begin();
         iter != _networks.end(); iter++ )
    {
        const Network* network   = (*iter).second;
        //network->pack( connections, fullUpdate );
    }
}

