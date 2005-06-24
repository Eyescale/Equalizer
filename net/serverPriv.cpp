
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

using namespace eqNet::priv;
using namespace std;

Server::Server()
        : Node(NODE_ID_SERVER),
          _sessionID(1),
          _state(STATE_STOPPED)
{}

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

    Session* session = new Session( _sessionID++ );
    Network* network = session->addNetwork( eqNet::PROTO_PIPE );
    Node*    client  = session->addNode();

    ConnectionDescription serverDescription;
    ConnectionDescription clientDescription;

    session->addNode( this );
    network->addNode( this->getID(), serverDescription );
    network->addNode( client->getID(), clientDescription );
    
    PipeNetwork *pipeNetwork = static_cast<PipeNetwork*>(network);
    pipeNetwork->setStarted( client->getID(), connection );
    
    _sessions[session->getID()]=session;
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

    Session* session = new Session( _sessionID++ );
    Network* network = session->addNetwork( eqNet::PROTO_TCPIP );

    Connection*           connection = Connection::create(PROTO_TCPIP);
    ConnectionDescription connDesc;
    connDesc.parameters.TCPIP.address = "localhost:4242";

    session->addNode( this );
    network->addNode( this->getID(), connDesc );

    if( !network->init() || !network->start())
        return false;

    _sessions[session->getID()]=session;
    _state = STATE_STARTED;
    
    INFO << endl << this;
}

//----------------------------------------------------------------------
// main loop
//----------------------------------------------------------------------
int Server::_run()
{
    if( _state != STATE_STARTED )
        return EXIT_FAILURE;

//     while( true )
//     {
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
//     }

//     return EXIT_SUCCESS;
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
