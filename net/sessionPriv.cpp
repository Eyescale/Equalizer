
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sessionPriv.h"
#include "connection.h"
#include "connectionDescription.h"
#include "networkPriv.h"
#include "nodePriv.h"
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
          _nodeID(NODE_ID_SERVER+1),
          _server(server)
{
    _nodes[server->getID()] = server;
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
    Node* node = new Node(_nodeID++);
    _nodes[node->getID()] = node;
    return node;
}

void Session::setLocalNode( const uint nodeID )
{
    IDHash<Node*>::iterator iter = _nodes.find( nodeID );
    ASSERT( iter != _nodes.end( ));
    _localNode = (*iter).second;
}


bool Session::initNode( const uint nodeID )
{
    IDHash<Node*>::iterator iter = _nodes.find( nodeID );
    ASSERT( iter != _nodes.end( ));

    Node* node = (*iter).second; 

    SessionNewPacket sessionNewPacket;
    sessionNewPacket.id;
    sessionNewPacket.serverID;

    for( IDHash<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         iter++ )
    {
        NodeNewPacket nodeNewPacket;
        nodeNewPacket.id;
        nodeNewPacket.sessionID;
    };

    for( IDHash<Network*>::iterator iter = _networks.begin();
         iter != _networks.end(); iter++ )
    {
        NetworkNewPacket networkNewPacket;
        networkNewPacket.id;
        networkNewPacket.sessionID;
        networkNewPacket.state;
        networkNewPacket.protocol;

        NetworkAddNodePacket networkAddNodePacket;
        networkAddNodePacket.id;
        networkAddNodePacket.nodeID;
        networkAddNodePacket.connectionDescription;
    }
    //connection->send( &response, response.size );
}
