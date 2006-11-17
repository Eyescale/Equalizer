
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
#include "launcher.h"
#include "packets.h"
#include "pipeConnection.h"
#include "session.h"
#include "uniPipeConnection.h"

#include <alloca.h>
#include <fcntl.h>
#include <sstream>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

extern char **environ;

#define MAX_PACKET_SIZE (4096)


PerThread<Node*> Node::_localNode;

//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node()
        : _autoLaunch(false),
          _autoLaunched(false),
          _id(true),
          _state(STATE_STOPPED),
          _launchID(EQ_ID_INVALID),
          _programName( Global::getProgramName( )),
          _workDir( Global::getWorkDir( )),
          _clientRunning(false),
          _nextConnectionRequestID(1)
{
    registerCommand( CMD_NODE_STOP, 
                     PacketFunc<Node>( this, &Node::_cmdStop ));
    registerCommand( CMD_NODE_MAP_SESSION, 
                     PacketFunc<Node>( this, &Node::_cmdMapSession ));
    registerCommand( CMD_NODE_MAP_SESSION_REPLY,
                     PacketFunc<Node>( this, 
                                          &Node::_cmdMapSessionReply ));
    registerCommand( CMD_NODE_UNMAP_SESSION, 
                     PacketFunc<Node>( this,
                                          &Node::_cmdUnmapSession ));
    registerCommand( CMD_NODE_UNMAP_SESSION_REPLY,
                     PacketFunc<Node>( this,
                                          &Node::_cmdUnmapSessionReply ));
    registerCommand( CMD_NODE_GET_CONNECTION_DESCRIPTION,
                     PacketFunc<Node>( this, 
                                          &Node::_cmdGetConnectionDescription));
    registerCommand( CMD_NODE_GET_CONNECTION_DESCRIPTION_REPLY,
         PacketFunc<Node>( this, &Node::_cmdGetConnectionDescriptionReply ));

    _receiverThread = new ReceiverThread( this );

    EQINFO << "New Node @" << (void*)this << endl;
}

Node::~Node()
{
    delete _receiverThread;
    EQINFO << "Delete Node @" << (void*)this << endl;
}

bool Node::listen( RefPtr<Connection> connection )
{
    if( _state != STATE_STOPPED )
        return false;

    if( connection.isValid() &&
        connection->getState() != Connection::STATE_LISTENING )
        return false;

    if( !_listenToSelf( ))
        return false;

    if( connection.isValid( ))
    {
        EQASSERT( connection->getDescription().isValid( ));
        EQASSERT( _connectionNodes.find( connection.get( ))
                  == _connectionNodes.end( ));

        _connectionNodes[ connection.get() ] = this;
        _connectionSet.addConnection( connection );
        _listener = connection;
        addConnectionDescription( connection->getDescription( ));
    }

    _state = STATE_LISTENING;
    _receiverThread->start();

    if( !getLocalNode( ))
        setLocalNode( this );

    EQINFO << this << " listening." << endl;
    return true;
}

bool Node::stopListening()
{
    if( _state != STATE_LISTENING )
        return false;

    NodeStopPacket packet;
    send( packet );
    const bool joined = _receiverThread->join();
    EQASSERT( joined );
    _cleanup();
    return true;
}

void Node::_cleanup()
{
    EQASSERTINFO( _state == STATE_STOPPED, _state );
    EQASSERT( _connection );

    _connectionSet.removeConnection( _connection );
    _connectionNodes.erase( _connection.get( ));
    _connection = NULL;
    _listener   = NULL;

    const size_t nConnections = _connectionSet.nConnections();
    for( size_t i = 0; i<nConnections; i++ )
    {
        RefPtr<Connection> connection = _connectionSet.getConnection(i);
        RefPtr<Node>       node       = _connectionNodes[ connection.get() ];

        node->_state      = STATE_STOPPED;
        node->_connection = NULL;
    }

    _connectionSet.clear();
    _connectionNodes.clear();
    _nodes.clear();
}

bool Node::_listenToSelf()
{
    // setup local connection to myself
    _connection = new UniPipeConnection;
    if( !_connection->connect())
    {
        EQERROR << "Could not create pipe() connection to receiver thread."
                << endl;
        _connection = NULL;
        return false;
    }

    // add to connection set
    EQASSERT( _connection->getDescription().isValid( )); 
    EQASSERT( _connectionNodes.find(_connection.get())==_connectionNodes.end());

    _connectionNodes[ _connection.get() ] = this;
    _connectionSet.addConnection( _connection );
    _nodes[ _id ] = this;
    return true;
}

uint32_t Node::startConnectNode( NodeID& nodeID, RefPtr<Node> server )
{
    NodeGetConnectionDescriptionPacket packet;
    packet.requestID  = _nextConnectionRequestID++;
    packet.appRequest = false;
    packet.nodeID     = nodeID;
    packet.index      = 0;

    _connectionRequests[ packet.requestID ] = nodeID;

    server->send( packet );
    return packet.requestID;
}   

Node::ConnectState Node::pollConnectNode( uint32_t requestID )
{
    IDHash<NodeID>::iterator iter = _connectionRequests.find( requestID );
    if( iter == _connectionRequests.end( ))
        return CONNECT_FAILURE;

    NodeID& nodeID = iter->second;
    if( _nodes.find( nodeID ) != _nodes.end( ))
    {
        _connectionRequests.erase( requestID );
        return CONNECT_SUCCESS;
    }

    return CONNECT_PENDING;
}

void Node::_addConnectedNode( RefPtr<Node> node, RefPtr<Connection> connection )
{
    EQASSERT( node.isValid( ));
    EQASSERT( _state == STATE_LISTENING );
    EQASSERT( connection->getState() == Connection::STATE_CONNECTED );
    EQASSERT( node->_state == STATE_STOPPED || node->_state == STATE_LAUNCHED );
    EQASSERT( connection->getDescription().isValid( ));
    EQASSERT( _connectionNodes.find( connection.get())==_connectionNodes.end());

    node->_connection = connection;
    node->_state      = STATE_CONNECTED;
    
    _connectionNodes[ connection.get() ] = node;
    _connectionSet.addConnection( connection );
    _nodes[ node->_id ] = node;
    EQINFO << node.get() << " connected to " << this << endl;
}

bool Node::connect( RefPtr<Node> node, RefPtr<Connection> connection )
{
    EQASSERT( connection.isValid( ));
    if( !node.isValid() || _state != STATE_LISTENING ||
        connection->getState() != Connection::STATE_CONNECTED ||
        node->_state != STATE_STOPPED )
        return false;

    NodeConnectPacket* packet = _performConnect( connection );

    if( !packet )
        return false;

    EQASSERT( !packet->wasLaunched );

    node->_id = packet->nodeID;
    
    RefPtr<ConnectionDescription> desc = new ConnectionDescription;
    if( desc->fromString( packet->connectionDescription ))
        node->addConnectionDescription( desc );

    free( packet );
    _addConnectedNode( node, connection );

    EQINFO << node.get() << " connected to " << this << endl;
    return true;
}

// two-way handshake to exchange node identifiers and other information
NodeConnectPacket* Node::_performConnect( RefPtr<Connection> connection)
{
    EQASSERT( _state == STATE_LISTENING );
    // send connect packet to peer
    eqNet::NodeConnectPacket packet;

    packet.nodeID = _id;
    if( _listener.isValid( ))
        connection->send( packet, _listener->getDescription()->toString( ));
    else
        connection->send( packet );

    return _readConnectReply( connection );
}

NodeConnectPacket* Node::_readConnectReply( RefPtr<Connection> connection )
{
    // receive and verify connect packet from peer
    uint64_t size;
    bool gotData = connection->recv( &size, sizeof( size ));
    if( !gotData || size == 0 )
    {
        EQERROR << "Failed to read data from connection" << endl;
        return NULL;
    }

    NodeConnectPacket* reply = (NodeConnectPacket*)malloc( size );
    reply->size  = size;
    size        -= sizeof( size );

    char* ptr = (char*)reply + sizeof(size);
    gotData   = connection->recv( ptr, size );
    EQASSERT( gotData );

    if( reply->datatype != DATATYPE_EQNET_NODE ||
        reply->command != CMD_NODE_CONNECT )
    {
        EQERROR << "Received invalid node connect packet: " << reply << endl;
        free( reply );
        return NULL;
    }

    return reply;
}

eqBase::RefPtr<Node> Node::getNode( const NodeID& id ) const
{ 
    NodeIDHash< eqBase::RefPtr<Node> >::const_iterator iter = _nodes.find( id );
    if( iter == _nodes.end( ))
        return NULL;
    return iter->second;
}

void Node::setLocalNode( RefPtr<Node> node )
{
    // manual ref/unref to keep correct reference count
    if( _localNode.get( ))
        _localNode->unref();
    node->ref();
    _localNode = node.get();
}

bool Node::disconnect()
{
    if( _state != STATE_CONNECTED )
        return false;

    if( _autoLaunched )
    {
        EQINFO << "Stopping autoaunched node" << endl;
        NodeStopPacket packet;
        send( packet );
    }

    return getLocalNode()->disconnect( this );
}

bool Node::disconnect( Node* node )
{
    if( !node || _state != STATE_LISTENING || 
        !node->_state == STATE_CONNECTED || !node->_connection )
        return false;

    if( !_connectionSet.removeConnection( node->_connection ))
        return false;

    EQINFO << node << " disconnecting from " << this << endl;
    EQASSERT( _connectionNodes.find( node->_connection.get( ))
              != _connectionNodes.end( ));

    _connectionNodes.erase( node->_connection.get( ));
    _nodes.erase( node->_id );

    node->_state      = STATE_STOPPED;
    node->_connection = NULL;
    return true;
}

//----------------------------------------------------------------------
// send functions
//----------------------------------------------------------------------
uint64_t Node::_getMessageSize( const MessageType type, const uint64_t count )
{
    switch( type )
    {
        default:
        case TYPE_BYTE:
            return count;
        case TYPE_SHORT:
            return 2 * count;
        case TYPE_INTEGER:
        case TYPE_FLOAT:
            return 4 * count;
    }
}

//----------------------------------------------------------------------
// Node functionality
//----------------------------------------------------------------------
void Node::addSession( Session* session, RefPtr<Node> server, 
                       const uint32_t sessionID, const string& name )
{
    CHECK_THREAD( _thread );

    session->_localNode = this;
    session->_server    = server;
    session->_id        = sessionID;
    session->_name      = name;
    session->_isMaster  = ( server==this && isLocal( ));

    _sessions[sessionID] = session;

    EQINFO << (session->_isMaster ? "master" : "client") << " session, id "
         << sessionID << ", name " << name << ", served by " << server.get() 
         << ", managed by " << this << endl;
}

void Node::removeSession( Session* session )
{
    CHECK_THREAD( _thread );
    _sessions.erase( session->getID( ));

    session->_localNode = NULL;
    session->_server    = NULL;
    session->_id        = EQ_ID_INVALID;
    session->_name      = "";
    session->_isMaster  = false;
}

bool Node::mapSession( RefPtr<Node> server, Session* session, 
                       const string& name )
{
    EQASSERT( isLocal( ));
    EQASSERT( !_receiverThread->isCurrent( ));

    NodeMapSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    server->send( packet, name );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

bool Node::mapSession( RefPtr<Node> server, Session* session, const uint32_t id)
{
    EQASSERT( isLocal( ));
    EQASSERT( id != EQ_ID_INVALID );

    NodeMapSessionPacket packet;
    packet.requestID = _requestHandler.registerRequest( session );
    packet.sessionID =  id;
    server->send( packet );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

bool Node::unmapSession( Session* session )
{
    EQASSERT( isLocal( ));

    NodeUnmapSessionPacket packet;
    packet.requestID = _requestHandler.registerRequest( session );
    packet.sessionID =  session->getID();
    session->getServer()->send( packet );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

uint32_t Node::_generateSessionID()
{
    CHECK_THREAD( _thread );
    uint32_t id = EQ_ID_INVALID;

    while( id == EQ_ID_INVALID || _sessions.find( id ) != _sessions.end( ))
    {
#ifdef __linux__
        int fd = ::open( "/dev/random", O_RDONLY );
        EQASSERT( fd != -1 );
        int read = ::read( fd, &id, sizeof( id ));
        EQASSERT( read == sizeof( id ));
        close( fd );
#else
        srandomdev();
        id = random();
#endif
    }

    return id;  
}

//----------------------------------------------------------------------
// receiver thread functions
//----------------------------------------------------------------------
void* Node::_runReceiver()
{
    EQINFO << "Entered receiver thread" << endl;

    if( !getLocalNode( ))
        setLocalNode( this );

    int nErrors = 0;
    while( _state == STATE_LISTENING )
    {
        const int result = _connectionSet.select( );
        switch( result )
        {
            case ConnectionSet::EVENT_CONNECT:
                _handleConnect();
                break;

            case ConnectionSet::EVENT_DATA:      
            {
                RefPtr<Connection> connection = _connectionSet.getConnection();
                RefPtr<Node>       node = _connectionNodes[ connection.get() ];
                EQASSERT( node->_connection == connection );
                _handleRequest( node.get() ); // do not pass down RefPtr atm
                break;
            }

            case ConnectionSet::EVENT_DISCONNECT:
            {
                _handleDisconnect();
                EQVERB << &_connectionSet << endl;
                break;
            } 

            case ConnectionSet::EVENT_TIMEOUT:   
                EQINFO << "select timeout" << endl;
                break;

            case ConnectionSet::EVENT_ERROR:      
                ++nErrors;
                EQWARN << "Error during select" << endl;
                if( nErrors > 100 )
                {
                    EQWARN << "Too many errors in a row, capping connection" 
                           << endl;
                    _handleDisconnect();
                }
                break;

            default:
                EQUNIMPLEMENTED;
        }
        if( result != ConnectionSet::EVENT_ERROR )
            nErrors = 0;
    }

    return EXIT_SUCCESS;
}

void Node::_handleConnect()
{
    RefPtr<Connection> connection = _connectionSet.getConnection();
    RefPtr<Connection> newConn    = connection->accept();
    handleConnect( newConn );
}

void Node::handleConnect( RefPtr<Connection> connection )
{
    NodeConnectPacket* packet = _performConnect( connection );
    if( !packet )
    {
        EQERROR << "Connection initialisation failed, rejecting connection"
                << endl;
        connection->close();
        return;
    }
    
    RefPtr<Node> node;

    if( packet->wasLaunched )
    {
        //ASSERT( dynamic_cast<Node*>( (Thread*)packet->launchID ));

        node = (Node*)packet->launchID;
        EQINFO << "Launched " << node.get() << " connecting" << endl;

        const uint32_t requestID = node->_launchID;
        EQASSERT( requestID != EQ_ID_INVALID );

        RefPtr<ConnectionDescription> desc = node->getConnectionDescription(0);
        bool readDesc = desc->fromString( packet->connectionDescription );
        EQASSERT( readDesc );

        node->_id           = packet->nodeID;
        node->_autoLaunched = true;

        _addConnectedNode( node, connection );
        _requestHandler.serveRequest( requestID, NULL );
    }
    else
    {
        node = createNode( REASON_INCOMING_CONNECT );
        node->_id = packet->nodeID;

        RefPtr<ConnectionDescription> desc = new ConnectionDescription;
        if( desc->fromString( packet->connectionDescription ))
            node->addConnectionDescription( desc );

        _addConnectedNode( node, connection );
    }

    free( packet );
}

void Node::_handleDisconnect()
{
    RefPtr<Connection> connection = _connectionSet.getConnection();
    RefPtr<Node>       node       = _connectionNodes[ connection.get() ];

    while( _handleRequest( node.get( ))); // read remaining data of connection

    handleDisconnect( node.get() ); // XXX
    connection->close();
}

void Node::handleDisconnect( Node* node )
{
    const bool disconnected = disconnect( node );
    EQASSERT( disconnected );
}

bool Node::_handleRequest( Node* node )
{
    EQVERB << "Handle request from " << node << endl;

    uint64_t size;
    const uint64_t read = node->_connection->recv( &size, sizeof( size ));
    if( read == 0 ) // Some systems signal data on dead connections.
        return false;

    EQASSERT( read == sizeof( size ));
    EQASSERT( size );

    // limit size due to use of alloca(). TODO: implement malloc-based recv?
    EQASSERT( size <= MAX_PACKET_SIZE );

    Packet* packet = (Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char*      ptr     = (char*)packet + sizeof(size);
    const bool gotData = node->_connection->recv( ptr, size );
    EQASSERT( gotData );

    const CommandResult result = dispatchPacket( node, packet );

    switch( result )
    {
        case COMMAND_ERROR:
            EQERROR << "Error handling command " << packet << endl;
            EQASSERT(0);
            break;
        
        case COMMAND_REDISPATCH:
        case COMMAND_HANDLED:
        case COMMAND_DISCARD:
            break;
            
        case COMMAND_PUSH:
            if( !pushCommand( node, packet ))
                EQASSERTINFO( 0, "Error handling command packet: " 
                              << "pushCommand failed for " << packet << endl );
            break;

        case COMMAND_PUSH_FRONT:
            if( !pushCommand( node, packet ))
                EQASSERTINFO( 0, "Error handling command packet: " 
                              << "pushCommandFront failed for " << packet 
                              << endl );
            break;

        default:
            EQUNIMPLEMENTED;
    }

    _redispatchPackets();

    if( result == COMMAND_REDISPATCH )
    {
        Request* request = _requestCache.alloc( node, packet );
        _pendingRequests.push_back( request );
    }

    return true;
}

void Node::_redispatchPackets()
{
    for( list<Request*>::iterator iter = _pendingRequests.begin(); 
         iter != _pendingRequests.end(); ++iter )
    {
        Request* request = (*iter);
        
        switch( dispatchPacket( request->node, request->packet ))
        {
            case COMMAND_HANDLED:
            case COMMAND_DISCARD:
            {
                list<Request*>::iterator handledIter = iter;
                ++iter;
                _pendingRequests.erase( handledIter );
                _requestCache.release( request );
            }
            break;

            case COMMAND_ERROR:
                EQERROR << "Error handling command " << request->packet << endl;
                EQASSERT(0);
                break;
                
            // Already a pushed packet?!
            case COMMAND_PUSH:
                EQUNIMPLEMENTED;
            case COMMAND_PUSH_FRONT:
                EQUNIMPLEMENTED;

            case COMMAND_REDISPATCH:
                break;
        }
    }
}

CommandResult Node::dispatchPacket( Node* node, const Packet* packet )
{
    EQVERB << "dispatch " << packet << " from " << (void*)node << " by " 
         << (void*)this << endl;
    const uint32_t datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQNET_NODE:
            return handleCommand( node, packet );

        case DATATYPE_EQNET_SESSION:
        case DATATYPE_EQNET_OBJECT:
        {
            const SessionPacket* sessionPacket = (SessionPacket*)packet;
            const uint32_t       id            = sessionPacket->sessionID;
            Session*             session       = _sessions[id];
            EQASSERTINFO( session, id );
            
            return session->dispatchPacket( node, sessionPacket );
        }

        default:
            if( datatype < DATATYPE_EQNET_CUSTOM )
            {
                EQERROR << "Unknown eqNet datatype " << datatype 
                      << ", dropping packet." << endl;
                return COMMAND_ERROR;
            }

            return handlePacket( node, packet );
    }
}

CommandResult Node::_cmdStop( Node* node, const Packet* pkg )
{
    EQINFO << "Cmd stop " << this << endl;
    EQASSERT( _state == STATE_LISTENING );

    if( _clientRunning ) // subclass may override _cmdStop to exit clientLoop
        EQWARN << "Stopping receiver thread while client loop is still running"
               << endl;

    // TODO: Process pending packets on all other connections?

    _state = STATE_STOPPED;
    _receiverThread->exit( EXIT_SUCCESS );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdMapSession( Node* node, const Packet* pkg )
{
    EQASSERT( getState() == STATE_LISTENING );

    NodeMapSessionPacket* packet  = (NodeMapSessionPacket*)pkg;
    EQINFO << "Cmd map session: " << packet << endl;
    CHECK_THREAD( _thread );
    
    Session*       session   = NULL;
    const uint32_t sessionID = packet->sessionID;
    string         sessionName;
    
    if( node == this ) // local mapping
    {
        if( sessionID == EQ_ID_INVALID ) // mapped by name
        {
            sessionName = packet->name;
            if( !_findSession( sessionName )) // not yet mapped 
            {
                session = (Session*)_requestHandler.getRequestData( packet->
                                                                    requestID );
                const uint32_t sessionID = _generateSessionID();
                addSession( session, this, sessionID, sessionName );
            }
        }
        // else mapped by identifier: not possible since we are the master
    }
    else // remote mapping
    {
        if( sessionID == EQ_ID_INVALID ) // mapped by name
        {
            sessionName = packet->name;
            session     = _findSession( sessionName );
        
            if( !session ) // session does not exist, wait until master maps it
                return COMMAND_REDISPATCH;
        }
        else // mapped by identifier, session has to exist already
        {
            session = _sessions[sessionID];
            if( session )
                sessionName = session->getName();
        }
    }

    NodeMapSessionReplyPacket reply( packet );
    reply.sessionID  = session ? session->getID() : EQ_ID_INVALID;
                
    node->send( reply, sessionName );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdMapSessionReply( Node* node, const Packet* pkg)
{
    NodeMapSessionReplyPacket* packet  = (NodeMapSessionReplyPacket*)pkg;
    EQINFO << "Cmd map session reply: " << packet << endl;
    CHECK_THREAD( _thread );

    const uint32_t requestID = packet->requestID;
    if( packet->sessionID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( requestID, (void*)false );
        return COMMAND_HANDLED;
    }        
    
    if( node == this ) // local mapping, was performed in _cmdMapSession
    {
        _requestHandler.serveRequest( requestID, (void*)true );
        return COMMAND_HANDLED;
    }

    Session* session = (Session*)_requestHandler.getRequestData( requestID );
    EQASSERT( session );

    addSession( session, node, packet->sessionID, packet->name );
    _requestHandler.serveRequest( requestID, (void*)true );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdUnmapSession( Node* node, const Packet* pkg )
{
    NodeUnmapSessionPacket* packet  = (NodeUnmapSessionPacket*)pkg;
    EQINFO << "Cmd unmap session: " << packet << endl;
    CHECK_THREAD( _thread );
    
    const uint32_t sessionID = packet->sessionID;
    Session*       session   = _sessions[sessionID];

    NodeUnmapSessionReplyPacket reply( packet );

    if( !session )
    {
        reply.result = false;
        node->send( reply );
        return COMMAND_HANDLED;
    }

    if( session->_server == this )
        removeSession( session );
        // TODO: unmap all session instances.

    reply.result = true;
    node->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdUnmapSessionReply( Node* node, const Packet* pkg)
{
    NodeUnmapSessionReplyPacket* packet  = (NodeUnmapSessionReplyPacket*)pkg;
    EQINFO << "Cmd unmap session reply: " << packet << endl;
    CHECK_THREAD( _thread );

    const uint32_t requestID = packet->requestID;

    if( !packet->result == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( requestID, (void*)false );
        return COMMAND_HANDLED;
    }        
    
    Session* session = (Session*)_requestHandler.getRequestData( requestID );
    EQASSERT( session );

    if( node != this ) // client instance unmapping
        removeSession( session );

    _requestHandler.serveRequest( requestID, (void*)true );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetConnectionDescription( Node* node, const Packet* pkg)
{
    NodeGetConnectionDescriptionPacket* packet =
        (NodeGetConnectionDescriptionPacket*)pkg;
    EQINFO << "cmd get connection description: " << packet << endl;

    RefPtr<Node> descNode = getNode( packet->nodeID );
    
    NodeGetConnectionDescriptionReplyPacket reply( packet );

    if( !descNode.isValid() ||
        descNode->nConnectionDescriptions() >= reply.nextIndex )
        
        reply.nextIndex = 0;
    else
        reply.nextIndex = packet->index + 1;

    RefPtr<ConnectionDescription> desc = descNode.isValid() ?
        descNode->getConnectionDescription( packet->index ) : NULL;

    if( desc.isValid( ))
        node->send( reply, desc->toString( ));
    else
        node->send( reply );

    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetConnectionDescriptionReply( Node* fromNode, 
                                                       const Packet* pkg )
{
    NodeGetConnectionDescriptionReplyPacket* packet =
        (NodeGetConnectionDescriptionReplyPacket*)pkg;
    EQINFO << "cmd get connection description reply: " << packet << endl;

    const uint32_t requestID = packet->requestID;
    NodeID&        nodeID    = packet->appRequest ? 
        *((NodeID*)_requestHandler.getRequestData( requestID )) :
        _connectionRequests[ requestID ];

    RefPtr<Node> node   = getNode( nodeID );
    if( node.isValid( )) // already connected
    {
        if( packet->appRequest )
            _requestHandler.serveRequest( requestID, NULL );
        return COMMAND_HANDLED;
    }

    RefPtr<ConnectionDescription> desc = new ConnectionDescription();
    if( desc->fromString( packet->connectionDescription ))
    {
        RefPtr<Connection> connection = Connection::create( desc->type );
        connection->setDescription( desc );
        if( connection->connect( ))
        {
            handleConnect( connection );
            
            node = getNode( nodeID );
            EQASSERT( node );
            EQASSERT( node->getNodeID() == nodeID );
            
            EQINFO << "Node " << nodeID << " connected" << endl;
            if( packet->appRequest )
                _requestHandler.serveRequest( requestID, NULL );
            return COMMAND_HANDLED;
        }
    }

    if( packet->nextIndex == 0 )
    {
        EQWARN << "Connection to node " << nodeID << " failed" << endl;
        if( packet->appRequest )
            _requestHandler.serveRequest( requestID, NULL );
        else
            _connectionRequests.erase( requestID );
        return COMMAND_HANDLED;
    }

    // Connection failed, try next connection description
    NodeGetConnectionDescriptionPacket reply;
    reply.requestID  = requestID;
    reply.nodeID     = nodeID;
    reply.appRequest = packet->appRequest;
    reply.index      = packet->nextIndex;
    fromNode->send( reply );
    
    return COMMAND_HANDLED;
}

//----------------------------------------------------------------------
// utility functions
//----------------------------------------------------------------------
Session* Node::_findSession( const std::string& name ) const
{
    CHECK_THREAD( _thread );
    for( IDHash<Session*>::const_iterator iter = _sessions.begin();
         iter != _sessions.end(); iter++ )
    {
        Session* session = (*iter).second;
        if( session->getName() == name )
            return session;
    }
    return NULL;
}

//----------------------------------------------------------------------
// Connecting and launching a node
//----------------------------------------------------------------------
bool Node::connect()
{
    if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
        return true;

    if( !initConnect( ))
    {
        EQERROR << "Connection initialisation failed." << endl;
        return false;
    }

    return syncConnect();
}

bool Node::initConnect()
{
    if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
        return true;

    EQASSERT( _state == STATE_STOPPED );

    RefPtr<Node> localNode = Node::getLocalNode();
    if( !localNode )
        return false;

    // try connection first
    const size_t nDescriptions = nConnectionDescriptions();
    for( size_t i=0; i<nDescriptions; i++ )
    {
        RefPtr<ConnectionDescription> description = getConnectionDescription(i);
        RefPtr<Connection> connection = Connection::create( description->type );
        connection->setDescription( description );
        if( !connection->connect( ))
            continue;

        if( !localNode->connect( this, connection ))
            return false;

        return true;
    }

    EQINFO << "Node could not be connected." << endl;
    if( !_autoLaunch )
        return false;
    
    EQINFO << "Attempting to launch node." << endl;
    for( size_t i=0; i<nDescriptions; i++ )
    {
        RefPtr<ConnectionDescription> description = getConnectionDescription(i);

        if( _launch( description ))
            return true;
    }

    return false;
}

bool Node::syncConnect()
{
    RefPtr<Node> localNode = Node::getLocalNode();
    EQASSERT( localNode )

    if( _launchID == EQ_ID_INVALID )
        return ( _state == STATE_CONNECTED );

    ConnectionDescription *description = (ConnectionDescription*)
        localNode->_requestHandler.getRequestData( _launchID );

    bool success;
    localNode->_requestHandler.waitRequest( _launchID, &success,
                                            description->launchTimeout );
    
    if( success )
    {
        EQASSERT( _state == STATE_CONNECTED );
    }
    else
    {
        _state = STATE_STOPPED;
        localNode->_requestHandler.unregisterRequest( _launchID );
    }

    _launchID = EQ_ID_INVALID;
    return success;
}

RefPtr<Node> Node::connect( NodeID& nodeID, RefPtr<Node> server)
{
    NodeIDHash< eqBase::RefPtr<Node> >::const_iterator iter = 
        _nodes.find( nodeID );
    if( iter != _nodes.end( ))
        return iter->second;

    NodeGetConnectionDescriptionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( &nodeID );
    packet.appRequest = true;
    packet.nodeID     = nodeID;
    packet.index      = 0;

    server->send( packet );

    _requestHandler.waitRequest( packet.requestID );
    return getNode( nodeID );
}

bool Node::_launch( RefPtr<ConnectionDescription> description )
{
    EQASSERT( _state == STATE_STOPPED );

    RefPtr<Node> localNode = Node::getLocalNode();
    EQASSERT( localNode );

    const uint32_t requestID     = 
        localNode->_requestHandler.registerRequest( description.get( ));
    string         launchCommand = _createLaunchCommand( description );

    if( !Launcher::run( launchCommand ))
    {
        EQWARN << "Could not launch node using '" << launchCommand << "'" 
               << endl;
        localNode->_requestHandler.unregisterRequest( requestID );
        return false;
    }
    
    _state    = STATE_LAUNCHED;
    _launchID = requestID;
    return true;
}

string Node::_createLaunchCommand( RefPtr<ConnectionDescription> description )
{
    RefPtr<Node> localNode = Node::getLocalNode();
    if( !localNode )
    {
        EQERROR << "No local node, can't launch " << this << endl;
        return "";
    }

    if( description->launchCommand.size() == 0 )
        return "";

    const string& launchCommand    = description->launchCommand;
    const size_t  launchCommandLen = launchCommand.size();
    bool          commandFound     = false;
    size_t        lastPos          = 0;
    string        result;

    for( size_t percentPos = launchCommand.find( '%' );
         percentPos != string::npos; 
         percentPos = launchCommand.find( '%', percentPos+1 ))
    {
        ostringstream replacement;
        switch( launchCommand[percentPos+1] )
        {
            case 'c':
            {
                replacement << _createRemoteCommand();
                commandFound = true;
                break;
            }
            case 'h':
                replacement << description->hostname;
                break;

            case 'n':
                replacement << getNodeID();
                break;

            default:
                EQWARN << "Unknown token " << launchCommand[percentPos+1] 
                       << endl;
        }

        result += launchCommand.substr( lastPos, percentPos-lastPos );
        if( !replacement.str().empty( ))
            result += replacement.str();

        lastPos  = percentPos+2;
    }

    result += launchCommand.substr( lastPos, launchCommandLen-lastPos );

    if( !commandFound )
        result += " " + _createRemoteCommand();

    EQINFO << "Launch command: " << result << endl;
    return result;
}

string Node::_createRemoteCommand()
{
    RefPtr<Node>       localNode = Node::getLocalNode();
    RefPtr<Connection> listener = localNode->_listener;
    if( !listener.isValid() || 
        listener->getState() != Connection::STATE_LISTENING )
    {
        EQERROR << "local node is not listening, can't launch " << this << endl;
        return "";
    }

    RefPtr<ConnectionDescription> listenerDesc = listener->getDescription();
    EQASSERT( listenerDesc.isValid( ));

    RefPtr<ConnectionDescription> nodeDesc = getConnectionDescription(0);
    EQASSERT( nodeDesc.isValid( ));

    ostringstream stringStream;

#ifdef Darwin
    char libPath[] = "DYLD_LIBRARY_PATH";
#else
    char libPath[] = "LD_LIBRARY_PATH";
#endif

    stringStream << "env "; // XXX
    char* env = getenv( libPath );
    if( env )
        stringStream << libPath << "=" << env << " ";

    env = getenv( "EQLOGLEVEL" );
    if( env )
        stringStream << "EQLOGLEVEL=" << env << " ";

    // for( int i=0; environ[i] != NULL; i++ )
    // {
    //     replacement += environ[i];
    //     replacement += " ";
    // }
    
    string program = _programName;
    if( program[0] != '/' )
        program = _workDir + '/' + program;

    stringStream << "'" << program << " --eq-listen=\"" 
                 << nodeDesc->toString() << "\" --eq-client \""
                 << (long long)this << ":" << _workDir << ":"
                 << listenerDesc->toString() << "\"'";

    return stringStream.str();
}

bool Node::runClient( const string& clientArgs )
{
    EQASSERT( _state == STATE_LISTENING );

    size_t colonPos = clientArgs.find( ':' );
    if( colonPos == string::npos )
    {
        EQERROR << "Could not parse request identifier" << endl;
        return false;
    }

    const string   request     = clientArgs.substr( 0, colonPos );
    const uint64_t requestID   = atoll( request.c_str( ));
    string         description = clientArgs.substr( colonPos + 1 );

    colonPos = description.find( ':' );
    if( colonPos == string::npos )
    {
        EQERROR << "Could not parse working directory" << endl;
        return false;
    }

    const string workDir = description.substr( 0, colonPos );
    Global::setWorkDir( workDir );

    if( chdir( workDir.c_str( )) == -1 )
        EQWARN << "Can't change working directory to " << workDir << ": "
               << strerror( errno ) << endl;
    
    EQINFO << "Launching node with launch ID=" << requestID << ", cwd="
           << workDir << endl;

    description = description.substr( colonPos + 1 );
    RefPtr<ConnectionDescription> connectionDesc = new ConnectionDescription;
    if( !connectionDesc->fromString( description ))
    {
        EQERROR << "Could not parse connection description" << endl;
        return false;
    }

    RefPtr<Connection> connection = Connection::create( connectionDesc->type );
    connection->setDescription( connectionDesc );
    if( !connection->connect( ))
    {
        EQERROR << "Can't contact node" << endl;
        return false;
    }

    RefPtr<Node> node = createNode( REASON_CLIENT_CONNECT );
    if( !node.isValid() )
    {
        EQERROR << "Can't create node" << endl;
        return false;
    }
    
    NodeConnectPacket packet;
    packet.nodeID      = _id;
    packet.wasLaunched = true;
    packet.launchID    = requestID;

    connection->send( packet, _listener->getDescription()->toString( ));

    NodeConnectPacket* reply = _readConnectReply( connection );

    if( !reply )
        return false;

    EQASSERT( !reply->wasLaunched );

    node->_id = reply->nodeID;

    free( reply );
    _clientRunning = true;
    _addConnectedNode( node, connection );

    clientLoop();
    _clientRunning = false;

    const bool joined = _receiverThread->join();
    EQASSERT( joined );
    _cleanup();
    return true;
}
