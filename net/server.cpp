
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "networkPriv.h"
#include "pipeNetwork.h"
#include "sessionPriv.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

int Server::run( PipeConnection* connection )
{
    Server* server = new Server( connection );
    return server->_run();
}

Server::Server( PipeConnection* connection )
        : Node(NODE_ID_SERVER),
          _sessionID(1)
{
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
    
    INFO << endl << this;
}

//----------------------------------------------------------------------
// initialization
//----------------------------------------------------------------------

void Server::_init()
{
}

//----------------------------------------------------------------------
// main loop
//----------------------------------------------------------------------
int Server::_run()
{
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
