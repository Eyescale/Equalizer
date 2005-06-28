
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "serverPriv.h"

#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "networkPriv.h"
#include "pipeNetwork.h"
#include "sessionPriv.h"

#include <eq/base/log.h>

#include <alloca.h>
#include <unistd.h>
#include <sys/param.h>

using namespace eqNet::priv;
using namespace std;

Server::Server()
        : Node(NODE_ID_SERVER),
          _sessionID(1),
          _state(STATE_STOPPED)
{}

Session* Server::getSession( const uint index )
{
    size_t i=0;
    for( IDHash<Session*>::iterator iter = _sessions.begin();
         iter != _sessions.end(); iter++, i++ )
    {
        if( i==index )
            return (*iter).second;
    }
    return NULL;
}

//----------------------------------------------------------------------
// internal server (forked) init
//----------------------------------------------------------------------
int Server::run( PipeConnection* connection )
{
    Server* server = new Server();
    if( !server->start( connection ))
    {
        ERROR << "Server startup failed" << endl;
        return EXIT_FAILURE;
    }
    return server->_run();
}

bool Server::start( PipeConnection* connection )
{
    if( _state != STATE_STOPPED )
        return false;

    Session* session = new Session( _sessionID++, this, true );
    Network* network = session->newNetwork( eqNet::PROTO_PIPE );
    Node*    client  = session->newNode();

    ConnectionDescription serverDescription;
    ConnectionDescription clientDescription;

    network->addNode( this->getID(), serverDescription );
    network->addNode( client->getID(), clientDescription );
    
    ConnectionNetwork *connectionNet = static_cast<ConnectionNetwork*>(network);
    connectionNet->setStarted( client->getID(), (Connection*)connection );
    
    _sessions[session->getID()] = session;
    _state = STATE_STARTED;
    
    INFO << endl << this;
    return true;
}

//----------------------------------------------------------------------
// standalone server init
//----------------------------------------------------------------------
int Server::run( const char* address )
{
    Server* server = new Server();
    if( !server->start( address ))
    {
        ERROR << "Server startup failed" << endl;
        return EXIT_FAILURE;
    }
    return server->_run();
}

bool Server::start( const char* address )
{
    if( _state != STATE_STOPPED )
        return false;

    Session* session = new Session( _sessionID++, this, true );
    Network* network = session->newNetwork( eqNet::PROTO_TCPIP );

    Connection*           connection = Connection::create(PROTO_TCPIP);
    ConnectionDescription connDesc;
    connDesc.parameters.TCPIP.address = "localhost:4242";

    network->addNode( this->getID(), connDesc );

    if( !network->init() || !network->start())
    {
        delete session;
        return false;
    }

    _sessions[session->getID()]=session;
    _state = STATE_STARTED;
    
    INFO << endl << this;
    return true;
}

//----------------------------------------------------------------------
// connect to a server
//----------------------------------------------------------------------
Server* Server::connect( const char* address )
{
    Server* server = new Server();
    if( !server->_connect( address ))
    {
        delete server;
        return NULL;
    }
    return server;
}


bool Server::_connect( const char* serverAddress )
{
    if( _state != STATE_STOPPED )
        return false;

    Session* session = new Session( _sessionID++, this );
    Network* network = session->newNetwork( eqNet::PROTO_TCPIP );

    Connection*           connection = Connection::create(PROTO_TCPIP);
    ConnectionDescription serverDesc;
    ConnectionDescription clientDesc;

    if( serverAddress )
        serverDesc.parameters.TCPIP.address = serverAddress;
    else
    {
        // If the server address is <code>NULL</code>, the environment
        // variable EQSERVER is used to determine the server address.
        const char* env = getenv( "EQSERVER" );
        if( env )
            serverDesc.parameters.TCPIP.address = env;
        else
        {
            // If the environment variable is not set, the local server on the
            // default port is contacted.
            char *address = (char *)alloca( 16 );
            sprintf( address, "localhost:%d", DEFAULT_PORT );
            serverDesc.parameters.TCPIP.address = address;
        }
    }

    char address[MAXHOSTNAMELEN+8];
    gethostname( address, MAXHOSTNAMELEN+1 );
    sprintf( address, "%s:%d", address, DEFAULT_PORT );
    clientDesc.parameters.TCPIP.address = address;
    INFO << "local node TCP/IP address is " << address << endl;

    network->addNode( this->getID(), serverDesc );
    network->addNode( session->getLocalNodeID(), clientDesc );

    if( !network->init() || !network->start() ) // use locally forked server
    {
        network->exit();
        session->deleteNetwork( network );
        network = session->newNetwork( PROTO_PIPE );

        serverDesc.parameters.PIPE.entryFunc = "Server::run";
        clientDesc.parameters.PIPE.entryFunc = NULL;

        network->addNode( this->getID(), serverDesc );
        network->addNode( session->getLocalNodeID(), clientDesc );

        if( !network->init() || !network->start() ) 
        {
            network->exit();
            session->deleteNetwork( network );
            delete session;
            return false;
        }
    }

    _sessions[session->getID()]=session;
    _state = STATE_STARTED;
    
    INFO << endl << this;
    return true;
}

//----------------------------------------------------------------------
// main loop
//----------------------------------------------------------------------
int Server::_run()
{
    if( _state != STATE_STARTED )
        return EXIT_FAILURE;

    while( true )
    {
//         short event;
//         Connection *connection = Connection::select( _connections, -1, event );

//         switch( event )
//         {
//             case 0:
//                 WARN << "Got timeout during connection selection?" << endl;
//                 break;

//             case POLLERR:
//                 WARN << "Got error during connection selection" << endl;
//                 break;

//             case POLLIN:
//             case POLLPRI: // data is ready for reading
//                 _handleRequest( connection );
//                 break;

//             case POLLHUP: // disconnect happened
//                 WARN << "Unhandled: Connection disconnect" << endl;
//                 break;

//             case POLLNVAL: // disconnected connection
//                 WARN << "Unhandled: Disconnected connection" << endl;
//                 break;
//         }
//         break;
    }

    return EXIT_SUCCESS;
}

void Server::_handleRequest( Connection *connection )
{
//     switch( connection->getState() )
//     {
//         case Connection::STATE_CONNECTED:
//             break;
            
//         case Connection::STATE_LISTENING:
//             Connection *newConn = connection->accept();
//             _connections.push_back( newConn );
//             break;
//     }
}
