
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "serverPriv.h"

#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "networkPriv.h"
#include "nodeList.h"
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
          _connection(NULL)
{
    for( int i=0; i<CMD_SERVER_ALL; i++ )
        _cmdHandler[i] =  &eqNet::priv::Server::_handleUnknown;

    _cmdHandler[CMD_SESSION_CREATE] =&eqNet::priv::Server::_handleSessionCreate;
    _cmdHandler[CMD_SESSION_CREATED] = &eqNet::priv::Server::_handleSessionCreated;
    _cmdHandler[CMD_SESSION_NEW] = &eqNet::priv::Server::_handleSessionNew;
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
    server->_handleRequest( (Connection*)(connection) );
    return EXIT_SUCCESS;
}

bool Server::start( PipeConnection* connection )
{
    if( _state != STATE_STOPPED )
        return false;

    _state = STATE_STARTED;
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

    _connection = Connection::create(PROTO_TCPIP);
    ConnectionDescription connDesc;
    strncpy( connDesc.parameters.TCPIP.hostname, hostname, MAXHOSTNAMELEN );
    connDesc.parameters.TCPIP.port = port;

    if( !_connection->listen( connDesc ))
    {
        delete _connection;
        _connection = NULL;
        return false;
    }

    _state = STATE_STARTED;
    INFO << endl << this;
    return true;
}

//----------------------------------------------------------------------
// connect to a server
//----------------------------------------------------------------------
Session* Server::createSession( const char* address )
{
    Server*  server  = new Server();
    Session* session = server->_createSession( address );

    if( !session );
    {
        delete server;
        return NULL;
    }
    return session;
}

Session* Server::_createSession( const char* address )
{
    if( _state != STATE_STOPPED )
        return false;

    if( !_connect( address ))
        return false;

    _sendSessionCreate();

    while( _state != STATE_STARTED )
        _handleRequest( _connection );

    INFO << endl << this;
    return getSession(0); // XXX 
}

bool Server::_connect( const char* address )
{
    char *usedAddress;

    if( address )
        usedAddress = (char*)address;
    else
    {
        // If the server address is <code>NULL</code>, the environment
        // variable EQSERVER is used to determine the server address.
        usedAddress = getenv( "EQSERVER" );

        if( !usedAddress )
        {
            // If the environment variable is not set, the local server on the
            // default port is contacted.
            usedAddress = (char *)alloca( 16 );
            sprintf( usedAddress, "localhost:%d", DEFAULT_PORT );
        }
    }

    char   hostname[MAXHOSTNAMELEN];
    ushort port;
    Util::parseAddress( usedAddress, hostname, port );

    if( !_connectRemote( hostname, port ) && !_connectLocal( ))
        return false;

    return true;
}

bool Server::_connectRemote( const char* hostname, const ushort port )
{
    _connection = Connection::create( PROTO_TCPIP );

    ConnectionDescription connDesc;
    strncpy( connDesc.parameters.TCPIP.hostname, hostname, MAXHOSTNAMELEN+1);
    connDesc.parameters.TCPIP.port = port;

    if( !_connection->connect( connDesc ))
    {
        WARN << "Remote server connect failed" << endl;
        delete _connection;
        _connection = NULL;
        return false;
    }

    INFO << "Remote server connected" << endl;
    return true;
}

bool Server::_connectLocal()
{
    _connection = Connection::create( PROTO_PIPE );

    ConnectionDescription connDesc;
    connDesc.parameters.PIPE.entryFunc = "Server::run";

    if( !_connection->connect( connDesc ))
    {
        WARN << "Local server connect failed" << endl;
        delete _connection;
        _connection = NULL;
        return false;
    }

    INFO << "Local server connected" << endl;
    return true;
}

void Server::_sendSessionCreate()
{
    ASSERT( _connection );
    ASSERT( _connection->getState() == Connection::STATE_CONNECTED );

    SessionCreatePacket packet;
    gethostname( packet.requestorAddress, MAXHOSTNAMELEN+1);
    _connection->send( &packet, packet.size );
}

//----------------------------------------------------------------------
// main loop
//----------------------------------------------------------------------
int Server::_run()
{
    if( _state != STATE_STARTED )
        return EXIT_FAILURE;

    ASSERT( _connection->getState() == Connection::STATE_LISTENING );

    while( true )
    {
        Connection *connection = _connection->accept();
        if( connection == NULL )
        {
            WARN << "Error during accept()" << endl;
            continue;
        }
        _handleRequest( connection );
    }

    return EXIT_SUCCESS;
}

void Server::_handleRequest( Connection *connection )
{
    INFO << "Handle request on " << connection << endl;
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );

    uint64 size;
    uint64 received = connection->recv( &size, sizeof( size ));
    ASSERT( received == sizeof( size ));
    ASSERT( size );

    Packet* packet = (Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char* ptr = (char*)packet + sizeof(size);
    received      = connection->recv( ptr, size );
    ASSERT( received == size );

    _handlePacket( connection, packet );
}

void Server::_handlePacket( Connection* connection, const Packet* packet )
{
    INFO << "handle " << packet << endl;
    switch( packet->datatype )
    {
        case DATATYPE_SERVER:
            ASSERT( packet->command < CMD_SERVER_ALL );
            (this->*_cmdHandler[packet->command])( connection, packet );
            break;

        case DATATYPE_SESSION:
        case DATATYPE_NETWORK:
        case DATATYPE_NODE:
        {
            const SessionPacket* sessionPacket = (SessionPacket*)(packet);
            Session* session = _sessions[sessionPacket->sessionID];
            ASSERT( session );

            session->handlePacket( connection, sessionPacket );
        } break;

        default:
            WARN << "Unhandled packet " << packet << endl;
    }
}

void Server::_handleSessionCreate( Connection* connection, const Packet* pkg )
{
    const SessionCreatePacket* packet = (SessionCreatePacket*)pkg;
    INFO << "Handle session create: " << packet << endl;

    Node*    remoteNode;
    Session* session    = _createSession( packet->requestorAddress, connection,
                                          &remoteNode );
    ASSERT( session );
    NodeList nodes;
    nodes.push_back( remoteNode );

    _sessions[session->getID()] = session;
    INFO << "session created" << endl;

    session->pack( nodes, true );

    SessionCreatedPacket reply;
    reply.serverID    = getID();
    reply.sessionID   = session->getID();
    reply.localNodeID = remoteNode->getID();
    nodes.send( this, reply );

    _startSessionThread( session );
}

Session* Server::_createSession( const char* remoteAddress, 
                                 Connection* connection, Node** remoteNode )
{
    Session*       session      = new Session( _sessionID++, this );
    SocketNetwork* network      = static_cast<SocketNetwork*>
        (session->newNetwork( eqNet::PROTO_TCPIP ));

    *remoteNode = session->newNode();
    session->setLocalNode( getID( ));

    // create and init one TCP/IP network
    ConnectionDescription serverDesc;
    ConnectionDescription remoteDesc;

    Util::parseAddress( remoteAddress, remoteDesc.parameters.TCPIP.hostname, 
                        remoteDesc.parameters.TCPIP.port );

    INFO << "Server node, id: " << getID() << ", TCP/IP address: " 
         << serverDesc.parameters.TCPIP.hostname << ":" 
         << serverDesc.parameters.TCPIP.port << endl;
    INFO << "Local node,  id: " << (*remoteNode)->getID()
         << ", TCP/IP address: " << remoteDesc.parameters.TCPIP.hostname << ":" 
         << remoteDesc.parameters.TCPIP.port << endl;

    network->addNode( this, serverDesc );
    network->addNode( *remoteNode, remoteDesc );
    network->setStarted( *remoteNode, connection );
    network->setStarted( this );

    if( !network->init() || !network->start() )
    {
        WARN << "session create failed" << endl;
        network->exit();
        session->deleteNetwork( network );
        delete session;
        return NULL;
    }
    return session;
}

void Server::_handleSessionCreated( Connection* connection, const Packet* pkg )
{
    const SessionCreatedPacket* packet = (SessionCreatedPacket*)pkg;
    INFO << "Handle session created: " << packet << endl;

    ASSERT( packet->serverID == getID());

    Session* session = _sessions[packet->sessionID];
    ASSERT( session );

    session->setLocalNode( packet->localNodeID );
    _state = STATE_STARTED;
}

void Server::_handleSessionNew( Connection* connection, const Packet* pkg )
{
    const SessionNewPacket* packet  = (SessionNewPacket*)pkg;
    INFO << "Handle session new: " << packet << endl;
    
    ASSERT( getID() == packet->serverID );

    Session* session = new Session( packet->sessionID, this );
    _sessions[packet->sessionID] = session;
}

//----------------------------------------------------------------------
// session thread (server-side)
//----------------------------------------------------------------------

class SessionThread : public eqBase::Thread
{
public:
    SessionThread( Server* parent, Session* session )
            : eqBase::Thread( eqBase::Thread::PTHREAD ),
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

