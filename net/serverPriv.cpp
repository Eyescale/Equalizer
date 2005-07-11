
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "serverPriv.h"

#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "networkPriv.h"
#include "packet.h"
#include "pipeNetwork.h"
#include "sessionPriv.h"
#include "socketConnection.h"
#include "socketNetwork.h"
#include "util.h"

#include <eq/base/log.h>
#include <eq/base/thread.h>

#include <alloca.h>
#include <unistd.h>

using namespace eqNet::priv;
using namespace std;

Server::Server()
        : Node(NODE_ID_SERVER),
          _sessionID(1),
          _state(STATE_STOPPED),
          _listener(NULL)
{
    _cmdHandler[CMD_SESSION_CREATE] = &eqNet::priv::Server::_handleSessionCreate;
}

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

    Session* session = new Session( _sessionID++, this );
    Network* network = session->newNetwork( eqNet::PROTO_PIPE );
    Node*    client  = session->newNode();

    session->setLocalNode( getID( ));

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
int Server::run( const char* hostname, const ushort port )
{
    Server* server = new Server();
    if( !server->start( hostname, port ))
    {
        ERROR << "Server startup failed" << endl;
        return EXIT_FAILURE;
    }
    return server->_run();
}

bool Server::start( const char* hostname, const ushort port )
{
    if( _state != STATE_STOPPED )
        return false;

    _listener = Connection::create(PROTO_TCPIP);
    ConnectionDescription connDesc;
    strncpy( connDesc.parameters.TCPIP.hostname, hostname, MAXHOSTNAMELEN );
    connDesc.parameters.TCPIP.port = port;

    if( !_listener->listen( connDesc ))
    {
        delete _listener;
        _listener = NULL;
        return false;
    }

    _state = STATE_STARTED;
    INFO << endl << this;
    return true;
}

//----------------------------------------------------------------------
// connect to a server
//----------------------------------------------------------------------
Server* Server::connect( const char* hostname, const ushort port )
{
    Server* server = new Server();
    if( !server->_connect( hostname, port ))
    {
        delete server;
        return NULL;
    }
    return server;
}


bool Server::_connect( const char* hostname, const ushort port )
{
    if( _state != STATE_STOPPED )
        return false;

    Session* session = new Session( _sessionID++, this );

    if( !_connectRemote( session, hostname, port ) && !_connectLocal( session ))
    {
        delete session;
        return false;
    }

    _sessions[session->getID()]=session;
    _state = STATE_STARTED;
    
    INFO << endl << this;
    return true;
}

bool Server::_connectRemote( Session* session, const char* hostname, 
                             const ushort port )
{
    Network*              network = session->newNetwork( eqNet::PROTO_TCPIP);
    ConnectionDescription serverDesc;
    ConnectionDescription localDesc;

    const uint localNodeID = session->getLocalNodeID();

    gethostname( localDesc.parameters.TCPIP.hostname, MAXHOSTNAMELEN+1 );
    localDesc.parameters.TCPIP.port = DEFAULT_PORT; // XXX use listening port

    INFO << "Server node, id: " << getID() << ", TCP/IP address: " 
         << serverDesc.parameters.TCPIP.hostname << ":" 
         << serverDesc.parameters.TCPIP.port << endl;
    INFO << "Local node,  id: " << localNodeID << ", TCP/IP address: " 
         << localDesc.parameters.TCPIP.hostname << ":"
         << localDesc.parameters.TCPIP.port << endl;

    network->addNode( getID(), serverDesc );
    network->addNode( localNodeID, localDesc );
    network->setStarted( getID( ));

    if( !network->init() || !network->start() || !network->connect( getID( )))
    {
        WARN << "Remote server init failed" << endl;
        network->exit();
        session->deleteNetwork( network );
        return false;
    }

    INFO << "Remote server initialised" << endl;
    return true;
}

bool Server::_connectLocal( Session* session )
{
    Network*              network = session->newNetwork( eqNet::PROTO_PIPE );
    ConnectionDescription serverDesc;
    ConnectionDescription localDesc;

    serverDesc.parameters.PIPE.entryFunc = "Server::run";
    localDesc.parameters.PIPE.entryFunc = NULL;

    network->addNode( this->getID(), serverDesc );
    network->addNode( session->getLocalNodeID(), localDesc );

    if( !network->init() || !network->start() || !network->connect( getID( )))
    {
        WARN << "Local server init failed" << endl;
        network->exit();
        session->deleteNetwork( network );
        return false;
    }

    INFO << "Local server initialised" << endl;
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
        Connection *connection = _listener->accept();
        if( connection == NULL )
        {
            WARN << "Error during accept()" << endl;
            continue;
        }
        _handleRequest( connection );
    }

    return EXIT_SUCCESS;
}

bool Server::_handleRequest( Connection *connection )
{
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );

    uint   size;
    size_t received = connection->recv( &size, sizeof( size ));
    ASSERT( received == sizeof( size ));
    ASSERT( size );

    Packet* packet = (Packet*)malloc( size );
    packet->size   = size;

    received      = connection->recv( &(packet->command), size );
    ASSERT( received == size );
    ASSERT( packet->command < CMD_ALL );

    bool success = (this->*_cmdHandler[packet->command])( connection, packet );
    free( packet );
    return success;
}

bool Server::_handleSessionCreate( Connection* connection, Packet* pkg )
{
    ReqSessionCreatePacket* packet = (ReqSessionCreatePacket*)pkg;

    Session* session = _createSession( packet->localAddress, connection );
    if( !session )
        return false;

    _sessions[session->getID()] = session;
    INFO << "session created" << endl;

    // TODO: session->initRemote()

    // TODO: create session thread

    return true;
}

Session* Server::_createSession( const char* remoteAddress, 
                                 Connection* connection )
{
    Session*       session      = new Session( _sessionID++, this );
    SocketNetwork* network      = static_cast<SocketNetwork*>
        (session->newNetwork( eqNet::PROTO_TCPIP ));
    Node*          remoteNode   = session->newNode();
    const uint     remoteNodeID = remoteNode->getID();

    session->setLocalNode( getID( ));

    // create and init one TCP/IP network
    ConnectionDescription serverDesc;
    ConnectionDescription remoteDesc;

    Util::parseAddress( remoteAddress, remoteDesc.parameters.TCPIP.hostname, 
                        remoteDesc.parameters.TCPIP.port );

    INFO << "Server node, id: " << getID() << ", TCP/IP address: " 
         << serverDesc.parameters.TCPIP.hostname << ":" 
         << serverDesc.parameters.TCPIP.port << endl;
    INFO << "Local node,  id: " << remoteNodeID << ", TCP/IP address: " 
         << remoteDesc.parameters.TCPIP.hostname << ":" 
         << remoteDesc.parameters.TCPIP.port << endl;

    network->addNode( getID(), serverDesc );
    network->addNode( remoteNodeID, remoteDesc );
    network->setStarted( remoteNodeID, connection );

    if( !network->init() || !network->start() )
    {
        WARN << "session create failed" << endl;
        network->exit();
        session->deleteNetwork( network );
        delete session;
        return NULL;
    }

    session->initNode( remoteNodeID );
    return session;
}

//----------------------------------------------------------------------
// session thread initialisation
//----------------------------------------------------------------------

class SessionThread : public eqBase::Thread
{
public:
    SessionThread( Server* parent, Session* session )
            : eqBase::Thread( Thread::PTHREAD ),
              _parent( parent ),
              _session( session )
        {}

    virtual ssize_t run() { return _parent->runSession( _session ); }

private:
    Server*  _parent;
    Session* _session;
};

bool Server::_startSessionThread( Session* session )
{
    SessionThread* thread = new SessionThread( this, session );
    return thread->start();
}
ssize_t Server::runSession( Session* session )
{
    return EXIT_SUCCESS;
}
