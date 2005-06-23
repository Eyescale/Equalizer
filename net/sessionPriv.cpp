
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "sessionPriv.h"

#include "connection.h"
#include "networkPriv.h"
#include "nodePriv.h"
#include "server.h"

#include <eq/base/log.h>
#include <eq/net/connectionDescription.h>

#include <alloca.h>

using namespace eqNet::priv;
using namespace std;

Session::Session(const uint id)
        : Base( id ),
          eqNet::Session(),
          _networkID(1),
          _nodeID(NODE_ID_SERVER+1)
{
}

Session* Session::create( const char* server )
{
    Session *session = new Session(INVALID_ID);
    if( session->_create( server ))
        return session;

    delete session;
    return NULL;
}

bool Session::_create( const char* serverAddress )
{
    Server* server = _openServer( serverAddress );
    
    if( server == NULL )
    {
        WARN << "Could not contact server" << endl;
        //_id = INVALID_ID;
        return false;
    }

    
//     Node* server = new Node();
//     _nodes.push_back( server );

//     Node* node = new Node();
//     _nodes.push_back( node );

//     Network* network = new Network(Network::PROTO_TCPIP);
    return false;
}

Server* Session::_openServer( const char* serverAddress )
{
#if 0
    Network* network = Network::create( INVALID_ID, PROTO_TCPIP );
    Node*    server  = new Node( INVALID_ID );
    Node*    local   = new Node( INVALID_ID-1 );
    ConnectionDescription serverConnection;
    ConnectionDescription localConnection;
    
    if( serverAddress )
        serverConnection.TCPIP.address = serverAddress;
    else
    {
        // If the server address is <code>NULL</code>, the environment
        // variable EQSERVER is used to determine the server address.
        const char* env = getenv( "EQSERVER" );
        if( env )
            serverConnection.parameters.TCPIP.address = env;
        else
        {
            // If the environment variable is not set, the local server on the
            // default port is contacted.
            char *address = (char *)alloca( 16 );
            sprintf( address, "localhost:%d", DEFAULT_PORT );
            serverConnection.parameters.TCPIP.address = address;
        }
    }

    network->addNode( server->getID(), serverConnection );
    network->addNode( local->getID(), localConnection );
#else
    Network* network = Network::create( INVALID_ID, PROTO_PIPE );
    Node*    server  = new Node( INVALID_ID );
    Node*    local   = new Node( INVALID_ID-1 );
    ConnectionDescription serverConnection;
    ConnectionDescription localConnection;
    
    serverConnection.parameters.PIPE.entryFunc = "Server::run";

    network->addNode( server->getID(), serverConnection );
    network->addNode( local->getID(), localConnection );

    INFO << server;
#endif
    
    if( !network->init() || !network->start() )
    {
        // TODO delete network;
    }
        

    return NULL;
}
        

Network* Session::addNetwork( const NetworkProtocol protocol )
{
    Network* network = Network::create( _networkID++, PROTO_TCPIP );
    _networks[network->getID()] = network;
    return network;
}

Node* Session::addNode()
{
    Node* node = new Node(_nodeID++);
    _nodes[node->getID()] = node;
    return node;
}

void Session::addNode(Node* node)
{
    ASSERT( node->getID() != INVALID_ID );
    _nodes[node->getID()] = node;
}
