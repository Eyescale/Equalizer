
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
          _nodeID(server->getID()+1),
          _server(server),
          _serverID(server->getID()),
          _localNode(NULL),
          _localNodeID(INVALID_ID)
{
    _nodes[server->getID()] = server;

    for( int i=0; i<CMD_SESSION_ALL; i++ )
        _cmdHandler[i] = &eqNet::priv::Session::_handleUnknown;

    _cmdHandler[CMD_NODE_NEW]    = &eqNet::priv::Session::_handleNodeNew;
    _cmdHandler[CMD_NETWORK_NEW] = &eqNet::priv::Session::_handleNetworkNew;

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

void Session::handlePacket( Connection* connection, const SessionPacket* packet)
{
    INFO << "handle " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_SESSION:
            ASSERT( packet->command < CMD_SESSION_ALL );
            (this->*_cmdHandler[packet->command])( connection, packet );
            break;

        case DATATYPE_NETWORK:
        {
            const NetworkPacket* networkPacket = (NetworkPacket*)(packet);
            Network* network = _networks[networkPacket->networkID];
            ASSERT( network );

            network->handlePacket( connection, networkPacket );
        } break;

        case DATATYPE_NODE:
        {
            const NodePacket* nodePacket = (NodePacket*)(packet);
            Node* node = _nodes[nodePacket->nodeID];
            ASSERT( node );

            //node->handlePacket( connection, nodePacket );
        } break;

        default:
            WARN << "Unhandled packet " << packet << endl;
    }
}

void Session::_handleNodeNew( Connection* connection, const Packet* pkg )
{
    INFO << "Handle node new" << endl;
    const NodeNewPacket* packet  = (NodeNewPacket*)pkg;
    
    Node* node = new Node( packet->nodeID );
    _nodes[packet->nodeID] = node;
}

void Session::_handleNetworkNew( Connection* connection, const Packet* pkg )
{
    INFO << "Handle network new" << endl;
    const NetworkNewPacket* packet  = (NetworkNewPacket*)pkg;
    
    Network* network = Network::create( packet->networkID, this,
                                        packet->protocol );
    _networks[packet->networkID] = network;
}


void Session::pack( const NodeList& nodes, const bool initial )
{
    //if( initial )
    ASSERT( _server );

    SessionNewPacket sessionNewPacket;
    const uint       serverID  = _server->getID();
    const uint       sessionID = getID();
    sessionNewPacket.serverID  = serverID;
    sessionNewPacket.sessionID = sessionID;
    nodes.send( _localNode, sessionNewPacket );
    
    for( IDHash<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         iter++ )
    {
        NodeNewPacket nodeNewPacket;
        nodeNewPacket.serverID  = serverID;
        nodeNewPacket.sessionID = sessionID;
        nodeNewPacket.nodeID    = (*iter).first;
        nodes.send( _localNode, nodeNewPacket );
    };

    for( IDHash<Network*>::iterator iter = _networks.begin();
         iter != _networks.end(); iter++ )
    {
        const Network* network = (*iter).second;
        //network->pack( connections, fullUpdate );
    }
}

