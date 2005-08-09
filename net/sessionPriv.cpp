
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sessionPriv.h"
#include "connection.h"
#include "connectionDescription.h"
#include "networkPriv.h"
#include "nodePriv.h"
#include "nodeList.h"
#include "server.h"
#include "serverPriv.h"
#include "sessionPrivPackets.h"
#include "util.h"

#include <eq/base/log.h>

#include <alloca.h>

using namespace eqNet::priv;
using namespace std;

Session::Session(const uint id, Server* server )
        : eqNet::Session(id),
          _networkID(1),
          _nodeID(1),
          _server(server),
          _localNode(NULL)
{
    for( int i=0; i<CMD_SESSION_ALL; i++ )
        _cmdHandler[i] = &eqNet::priv::Session::_cmdUnknown;

    _cmdHandler[CMD_SESSION_NEW_NODE]    = &eqNet::priv::Session::_cmdNewNode;
    _cmdHandler[CMD_SESSION_NEW_NETWORK] = &eqNet::priv::Session::_cmdNewNetwork;

    INFO << "New session" << this;
}

Session* Session::create( const char* serverAddress )
{
    return Server::createSession( serverAddress );
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

void Session::setLocalNode( const uint nodeID )
{
    _localNode = _nodes[nodeID];
    ASSERT( _localNode );
}

void Session::handlePacket( SessionPacket* packet)
{
    INFO << "handle " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_SESSION:
            ASSERT( packet->command < CMD_SESSION_ALL );
            (this->*_cmdHandler[packet->command])( packet );
            break;

        case DATATYPE_NETWORK:
        {
            NetworkPacket* networkPacket = (NetworkPacket*)(packet);
            Network* network = _networks[networkPacket->networkID];
            ASSERT( network );

            network->handlePacket( networkPacket );
        } break;

        case DATATYPE_NODE:
        {
            NodePacket* nodePacket = (NodePacket*)(packet);
            Node* node = _nodes[nodePacket->nodeID];
            ASSERT( node );

            //node->handlePacket( nodePacket );
        } break;

        default:
            WARN << "Unhandled packet " << packet << endl;
    }
}

void Session::_cmdNewNode( Packet* pkg )
{
    SessionNewNodePacket* packet  = (SessionNewNodePacket*)pkg;
    INFO << "Cmd new node: " << packet << endl;
    
   if( packet->nodeID == INVALID_ID )
        packet->nodeID = _nodeID++;

    Node* node = new Node( packet->nodeID );
    _nodes[packet->nodeID] = node;
    packet->result = packet->nodeID;
}

void Session::_cmdNewNetwork( Packet* pkg )
{
    SessionNewNetworkPacket* packet  = (SessionNewNetworkPacket*)pkg;
    INFO << "Cmd new network: " << packet << endl;
    
    if( packet->networkID == INVALID_ID )
        packet->networkID = _networkID++;

    Network* network = Network::create( packet->networkID, this,
                                        packet->protocol );
    _networks[packet->networkID] = network;
    packet->result = packet->networkID;
}


void Session::pack( const NodeList& nodes )
{
    ASSERT( _server );

    for( IDHash<Node*>::iterator iter = _nodes.begin(); iter != _nodes.end();
         iter++ )
    {
        SessionNewNodePacket newNodePacket;
        newNodePacket.sessionID = getID();
        newNodePacket.nodeID    = (*iter).first;
        nodes.send( newNodePacket );
    };

    for( IDHash<Network*>::iterator iter = _networks.begin();
         iter != _networks.end(); iter++ )
    {
        Network* network = (*iter).second;

        SessionNewNetworkPacket newNetworkPacket;
        newNetworkPacket.sessionID = getID();
        newNetworkPacket.networkID = network->getID();
        newNetworkPacket.protocol  = network->getProtocol();
        nodes.send( newNetworkPacket );

        network->pack( nodes );
    }
}

