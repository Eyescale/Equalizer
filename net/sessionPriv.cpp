
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
          eqNet::Session()
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
    priv::Network* network = priv::Network::create( INVALID_ID, PROTO_TCPIP );
    priv::Node*    server  = new priv::Node( INVALID_ID );
    priv::Node*    local   = new priv::Node( INVALID_ID-1 );
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
            serverConnection.TCPIP.address = env;
        else
        {
            // If the environment variable is not set, the local server on the
            // default port is contacted.
            char *address = (char *)alloca( 16 );
            sprintf( address, "localhost:%d", DEFAULT_PORT );
            serverConnection.TCPIP.address = address;
        }
    }

    network->addNode( server->getID(), serverConnection );
    network->addNode( local->getID(), localConnection );
#else
    priv::Network* network = priv::Network::create( INVALID_ID, PROTO_PIPE );
    priv::Node*    server  = new priv::Node( INVALID_ID );
    priv::Node*    local   = new priv::Node( INVALID_ID-1 );
    ConnectionDescription serverConnection;
    ConnectionDescription localConnection;
    
    serverConnection.PIPE.entryFunc = "Server::run";

    network->addNode( server->getID(), serverConnection );
    network->addNode( local->getID(), localConnection );
    
#endif
    
    if( !network->init() || !network->start() )
    {
        // TODO delete network;
    }
        

    return NULL;
}
        
