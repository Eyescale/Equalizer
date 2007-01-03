
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "command.h"
#include "connectionSet.h"
#include "global.h"
#include "pipeConnection.h"
#include "session.h"
#include "uniPipeConnection.h"

#include <eq/base/launcher.h>

#include <fcntl.h>
#include <sstream>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

PerThread<Node*> Node::_localNode;

// node get connection 'magics'
#define NEXT_INDEX_NONE       0xffffffffu
#define NEXT_INDEX_CONNECTING 0xfffffffeu

//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node()
        : Base( true ),
          _autoLaunch(false),
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
                     CommandFunc<Node>( this, &Node::_cmdStop ));
    registerCommand( CMD_NODE_MAP_SESSION, 
                     CommandFunc<Node>( this, &Node::_cmdMapSession ));
    registerCommand( CMD_NODE_MAP_SESSION_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdMapSessionReply ));
    registerCommand( CMD_NODE_UNMAP_SESSION, 
                     CommandFunc<Node>( this, &Node::_cmdUnmapSession ));
    registerCommand( CMD_NODE_UNMAP_SESSION_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdUnmapSessionReply ));
    registerCommand( CMD_NODE_LAUNCHED,
                     CommandFunc<Node>( this, &Node::_cmdLaunched ));
    registerCommand( CMD_NODE_CONNECT,
                     CommandFunc<Node>( this, &Node::_cmdConnect ));
    registerCommand( CMD_NODE_CONNECT_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdConnectReply ));
    registerCommand( CMD_NODE_GET_CONNECTION_DESCRIPTION,
                 CommandFunc<Node>( this, &Node::_cmdGetConnectionDescription));
    registerCommand( CMD_NODE_GET_CONNECTION_DESCRIPTION_REPLY,
           CommandFunc<Node>( this, &Node::_cmdGetConnectionDescriptionReply ));

    _receiverThread  = new ReceiverThread( this );
    _receivedCommand = new Command;
    EQINFO << "New Node @" << (void*)this << " " << _id << endl;
}

Node::~Node()
{
    delete _receivedCommand;
    delete _receiverThread;
    EQINFO << "Delete Node @" << (void*)this << " " << _id << endl;
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
    _connection = 0;
    _listener   = 0;

    const size_t nConnections = _connectionSet.nConnections();
    for( size_t i = 0; i<nConnections; i++ )
    {
        RefPtr<Connection> connection = _connectionSet.getConnection(i);
        RefPtr<Node>       node       = _connectionNodes[ connection.get() ];

        node->_state      = STATE_STOPPED;
        node->_connection = 0;
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
        _connection = 0;
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

uint32_t Node::startNodeConnect( const NodeID& nodeID, RefPtr<Node> server )
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

Node::ConnectState Node::pollNodeConnect( uint32_t requestID )
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
    EQASSERT( node->_id != EQ_ID_INVALID );

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

    // send connect packet to peer
    const uint32_t requestID = _initiateConnection( node, connection );
    _requestHandler.waitRequest( requestID );

    EQASSERT( node->_id != EQ_ID_INVALID );
    EQINFO << node.get() << " connected to " << this << endl;
    return true;
}

uint32_t Node::_initiateConnection( RefPtr<Node> node, 
                                    RefPtr<Connection> connection )
{
    eqNet::NodeConnectPacket packet;
    packet.nodeID    = _id;

    node->ref();
    packet.requestID = _requestHandler.registerRequest( node.get( ));
    
    _connectionSet.addConnection( connection );
    if( _listener.isValid( ))
        connection->send( packet, _listener->getDescription()->toString( ));
    else
        connection->send( packet );

    return packet.requestID;
}

eqBase::RefPtr<Node> Node::getNode( const NodeID& id ) const
{ 
    NodeIDHash< eqBase::RefPtr<Node> >::const_iterator iter = _nodes.find( id );
    if( iter == _nodes.end( ))
        return 0;
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

bool Node::disconnect( RefPtr<Node> node )
{
    if( !node || _state != STATE_LISTENING || 
        !node->_state == STATE_CONNECTED || !node->_connection )
        return false;

    node->_state      = STATE_STOPPED;

    RefPtr<Connection> connection = node->_connection;
    node->_connection = 0;

    if( !_connectionSet.removeConnection( connection ))
        return false;

    EQINFO << node << " disconnecting from " << this << endl;
    EQASSERT( _connectionNodes.find( connection.get( )) !=
              _connectionNodes.end( ));

    _connectionNodes.erase( connection.get( ));
    _nodes.erase( node->_id );

    return true;
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

    session->_localNode = 0;
    session->_server    = 0;
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
                _handleData();
                break;

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

            case ConnectionSet::EVENT_INTERRUPT:
                _redispatchCommands();
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

    _connectionSet.addConnection( newConn );
    // Node will be created when receiving NodeConnectPacket from other side
}

void Node::_handleDisconnect()
{
    while( _handleData( )); // read remaining data off connection

    RefPtr<Connection> connection = _connectionSet.getConnection();
    RefPtr<Node>       node;
    if( _connectionNodes.find( connection.get( )) != _connectionNodes.end( ))
        node = _connectionNodes[ connection.get() ];

    EQASSERT( !node || node->_connection == connection );

    if( node.isValid( ))
    {
        node->_state      = STATE_STOPPED;
        node->_connection = 0;
        _connectionNodes.erase( connection.get( ));
        _nodes.erase( node->_id );
    }

    _connectionSet.removeConnection( connection );

    EQINFO << node << " disconnected from " << this << endl;
    connection->close();
}

bool Node::_handleData()
{
    RefPtr<Connection> connection = _connectionSet.getConnection();
    RefPtr<Node>       node;

    if( _connectionNodes.find( connection.get( )) != _connectionNodes.end( ))
        node = _connectionNodes[ connection.get() ];

    EQASSERT( !node || node->_connection == connection );
    EQVERB << "Handle data from " << node << endl;

    uint64_t       size;
    const uint64_t read = connection->recv( &size, sizeof( size ));
    if( read == 0 ) // Some systems signal data on dead connections.
        return false;

    EQASSERT( read == sizeof( size ));
    EQASSERT( size );

    _receivedCommand->allocate( node, size );
    size -= sizeof( size );

    char*      ptr     = (char*)_receivedCommand->getPacket() + sizeof(size);
    const bool gotData = connection->recv( ptr, size );

    EQASSERT( gotData );
    EQASSERT( _receivedCommand->isValid( ));

    // If no node is associated with the connection, the incoming packet has to
    // be a one of the connection initialization packets
    EQASSERT( node.isValid() ||
              ( (*_receivedCommand)->datatype == DATATYPE_EQNET_NODE &&
                ( (*_receivedCommand)->command == CMD_NODE_LAUNCHED || 
                  (*_receivedCommand)->command == CMD_NODE_CONNECT  || 
                  (*_receivedCommand)->command == CMD_NODE_CONNECT_REPLY )));

    const CommandResult result = dispatchCommand( *_receivedCommand );
    switch( result )
    {
        case COMMAND_ERROR:
            EQASSERTINFO( 0, "Error handling command " << *_receivedCommand );
            break;
        
        case COMMAND_REDISPATCH:
        case COMMAND_HANDLED:
        case COMMAND_DISCARD:
            break;
            
        case COMMAND_PUSH:
            if( !pushCommand( *_receivedCommand ))
                EQASSERTINFO( 0, "Error handling command packet: pushCommand "
                              << "failed for " << *_receivedCommand << endl );
            break;

        case COMMAND_PUSH_FRONT:
            if( !pushCommandFront( *_receivedCommand ))
                EQASSERTINFO( 0, "Error handling command packet: "
                              << "pushCommandFront failed for "
                              << *_receivedCommand << endl );
            break;

        default:
            EQUNIMPLEMENTED;
    }

    _redispatchCommands();
 
    if( result == COMMAND_REDISPATCH )
    {
        Command* command = _commandCache.alloc( *_receivedCommand );
        _pendingCommands.push_back( command );
    }
    
#if 0 // Note: tradeoff between memory footprint and performance
    // dealloc 'big' packets immediately
    if( _receivedCommand->isValid() && (*_receivedCommand)->exceedsMinSize( ))
        _receivedCommand->release();
#endif

    return true;
}

void Node::_redispatchCommands()
{
    bool changes = !_pendingCommands.empty();
    while( changes )
    {
        changes = false;
        for( list<Command*>::iterator iter = _pendingCommands.begin(); 
             iter != _pendingCommands.end(); ++iter )
        {
            Command* command = (*iter);
        
            switch( dispatchCommand( *command ))
            {
                case COMMAND_HANDLED:
                case COMMAND_DISCARD:
                {
                    list<Command*>::iterator handledIter = iter;
                    ++iter;
                    _pendingCommands.erase( handledIter );
                    _commandCache.release( command );
                    changes = true;
                }
                break;

                case COMMAND_ERROR:
                    EQERROR << "Error handling command " << command << endl;
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
}

CommandResult Node::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << " by " << (void*)this << endl;
    EQASSERT( command.isValid( ));

    const uint32_t datatype = command->datatype;
    switch( datatype )
    {
        case DATATYPE_EQNET_NODE:
            return invokeCommand( command );

        case DATATYPE_EQNET_SESSION:
        case DATATYPE_EQNET_OBJECT:
        {
            const SessionPacket* sessionPacket = 
                static_cast<SessionPacket*>( command.getPacket( ));
            const uint32_t       id            = sessionPacket->sessionID;
            Session*             session       = _sessions[id];
            EQASSERTINFO( session, id );
            
            return session->dispatchCommand( command );
        }

        default:
            if( datatype < DATATYPE_EQNET_CUSTOM )
            {
                EQERROR << "Unknown eqNet datatype " << datatype << endl;
                return COMMAND_ERROR;
            }
            
            return handleCommand( command );
    }
}

CommandResult Node::_cmdStop( Command& command )
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

CommandResult Node::_cmdMapSession( Command& command )
{
    EQASSERT( getState() == STATE_LISTENING );

    const NodeMapSessionPacket* packet = 
        command.getPacket<NodeMapSessionPacket>();
    EQINFO << "Cmd map session: " << packet << endl;
    CHECK_THREAD( _thread );
    
    Session*       session   = 0;
    const uint32_t sessionID = packet->sessionID;
    string         sessionName;
    RefPtr<Node>   node      = command.getNode();

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

CommandResult Node::_cmdMapSessionReply( Command& command)
{
    const NodeMapSessionReplyPacket* packet = 
        command.getPacket<NodeMapSessionReplyPacket>();
    EQINFO << "Cmd map session reply: " << packet << endl;
    CHECK_THREAD( _thread );

    const uint32_t requestID = packet->requestID;
    if( packet->sessionID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( requestID, (void*)false );
        return COMMAND_HANDLED;
    }        
    
    RefPtr<Node>   node      = command.getNode();
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

CommandResult Node::_cmdUnmapSession( Command& command )
{
    const NodeUnmapSessionPacket* packet =
        command.getPacket<NodeUnmapSessionPacket>();
    EQINFO << "Cmd unmap session: " << packet << endl;
    CHECK_THREAD( _thread );
    
    const uint32_t sessionID = packet->sessionID;
    Session*       session   = _sessions[sessionID];

    NodeUnmapSessionReplyPacket reply( packet );

    RefPtr<Node>   node      = command.getNode();
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

CommandResult Node::_cmdUnmapSessionReply( Command& command)
{
    const NodeUnmapSessionReplyPacket* packet = 
        command.getPacket<NodeUnmapSessionReplyPacket>();
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

    RefPtr<Node>   node      = command.getNode();
    if( node != this ) // client instance unmapping
        removeSession( session );

    _requestHandler.serveRequest( requestID, (void*)true );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdConnect( Command& command )
{
    EQASSERT( !command.getNode().isValid( ));
    EQASSERT( inReceiverThread( ));

    const NodeConnectPacket* packet = command.getPacket<NodeConnectPacket>();
    RefPtr<Connection>   connection = _connectionSet.getConnection();
    
    EQINFO << "handle connect " << packet << endl;

    // disconnect this connection if already in _connectionNodes
    if( _connectionNodes.find( connection.get( )) != _connectionNodes.end( ))
    {
        EQASSERT( _nodes.find( packet->nodeID ) != _nodes.end( ));
        RefPtr<Node> existingNode = _nodes[ packet->nodeID ];
        EQASSERT( existingNode->_connection != connection );

        const bool removed = _connectionSet.removeConnection( connection );
        EQASSERT( removed );
        EQASSERT( connection->getRefCount() == 1 );

        connection->close();
        return COMMAND_HANDLED;
    }

    // create and add connected node
    RefPtr<Node>            remoteNode = createNode( REASON_INCOMING_CONNECT );
    RefPtr<ConnectionDescription> desc = new ConnectionDescription;

    if( desc->fromString( packet->connectionDescription ))
        remoteNode->addConnectionDescription( desc );

    remoteNode->_id         = packet->nodeID;
    remoteNode->_connection = connection;
    remoteNode->_state      = STATE_CONNECTED;
    
    _connectionNodes[ connection.get() ] = remoteNode;
    _nodes[ packet->nodeID ]             = remoteNode;

    // send our information as reply
    NodeConnectReplyPacket reply( packet );
    reply.nodeID    = _id;
        
    if( _listener.isValid( ))
        connection->send( reply, _listener->getDescription()->toString( ));
    else
        connection->send( reply );

    return COMMAND_HANDLED;
}

CommandResult Node::_cmdConnectReply( Command& command )
{
    EQASSERT( !command.getNode().isValid( ));
    EQASSERT( inReceiverThread( ));

    const NodeConnectReplyPacket* packet = 
        command.getPacket<NodeConnectReplyPacket>();
    RefPtr<Connection> connection = _connectionSet.getConnection();

    EQINFO << "handle connect reply " << packet << endl;

    RefPtr<Node> remoteNode;
    if( packet->requestID == EQ_ID_INVALID )
        remoteNode = createNode( REASON_OUTGOING_CONNECT );
    else
    {
        remoteNode = static_cast<Node*>
            ( _requestHandler.getRequestData( packet->requestID ));

        EQASSERT( remoteNode.isValid( ));
        remoteNode->unref();
    }

    // disconnect this connection if already in _connectionNodes
    if( _connectionNodes.find( connection.get( )) != _connectionNodes.end( ))
    {
        EQASSERT( _nodes.find( packet->nodeID ) != _nodes.end( ));
        RefPtr<Node> existingNode = _nodes[ packet->nodeID ];
        EQASSERT( existingNode->_connection != connection );

        const bool removed = _connectionSet.removeConnection( connection );
        EQASSERT( removed );
        EQASSERT( connection->getRefCount() == 1 );
        EQASSERT( remoteNode->_id == EQ_ID_INVALID );

        connection->close();
        _requestHandler.serveRequest( packet->requestID, 0 );
        return COMMAND_HANDLED;
    }

    // add connected node
    RefPtr<ConnectionDescription> desc = new ConnectionDescription;

    if( desc->fromString( packet->connectionDescription ))
        remoteNode->addConnectionDescription( desc );

    remoteNode->_id         = packet->nodeID;
    remoteNode->_connection = connection;
    remoteNode->_state      = STATE_CONNECTED;
    
    _connectionNodes[ connection.get() ] = remoteNode;
    _nodes[ packet->nodeID ]             = remoteNode;

    _requestHandler.serveRequest( packet->requestID, 0 );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdLaunched( Command& command )
{
    EQASSERT( !command.getNode().isValid( ));

    const NodeLaunchedPacket* packet = command.getPacket<NodeLaunchedPacket>();
    EQASSERT( packet->requestID != EQ_ID_INVALID );
    EQINFO << "handle launched " << packet << endl;

    RefPtr<Node>            remoteNode = static_cast<Node*>
        ( _requestHandler.getRequestData( packet->launchID ));
    remoteNode->unref();

    RefPtr<Connection>      connection = _connectionSet.getConnection();
    RefPtr<ConnectionDescription> desc = 
        remoteNode->getConnectionDescription( 0 );
    bool readDesc = desc->fromString( packet->connectionDescription );
    EQASSERT( readDesc );

    // add connected node
    remoteNode->_id         = packet->nodeID;
    remoteNode->_connection = connection;
    remoteNode->_state      = STATE_CONNECTED;
    
    EQASSERT( _connectionNodes.find( connection.get( )) == 
              _connectionNodes.end( ));
    EQASSERT( _nodes.find( packet->nodeID ) == _nodes.end( ));

    _connectionNodes[ connection.get() ] = remoteNode;
    _nodes[ packet->nodeID ]             = remoteNode;

    // send our information as reply
    NodeConnectReplyPacket reply( packet );
    reply.nodeID    = _id;
        
    if( _listener.isValid( ))
        connection->send( reply, _listener->getDescription()->toString( ));
    else
        connection->send( reply );

    _requestHandler.serveRequest( packet->launchID, 0 );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetConnectionDescription( Command& command)
{
    const NodeGetConnectionDescriptionPacket* packet = 
        command.getPacket<NodeGetConnectionDescriptionPacket>();
    EQINFO << "cmd get connection description: " << packet << endl;

    RefPtr<Node> descNode = getNode( packet->nodeID );
    
    NodeGetConnectionDescriptionReplyPacket reply( packet );

    if( !descNode.isValid() ||
        descNode->nConnectionDescriptions() >= reply.nextIndex )
        
        reply.nextIndex = NEXT_INDEX_NONE;
    else
        reply.nextIndex = packet->index + 1;

    RefPtr<ConnectionDescription> desc = descNode.isValid() ?
        descNode->getConnectionDescription( packet->index ) : 0;

    RefPtr<Node> node = command.getNode();
    if( desc.isValid( ))
        node->send( reply, desc->toString( ));
    else
        node->send( reply );

    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetConnectionDescriptionReply( Command& command )
{
    NodeGetConnectionDescriptionReplyPacket* packet = 
        command.getPacket<NodeGetConnectionDescriptionReplyPacket>();
    EQINFO << "cmd get connection description reply: " << packet << endl;

    const uint32_t requestID = packet->requestID;
    const NodeID&  nodeID    = packet->nodeID;
    RefPtr<Node>   node      = getNode( nodeID );

    if( node.isValid( )) // already connected
    {
        if( packet->appRequest )
            _requestHandler.serveRequest( requestID, 0 );

        return COMMAND_HANDLED;
    }

    if( packet->nextIndex == NEXT_INDEX_CONNECTING )
        return COMMAND_REDISPATCH;

    RefPtr<ConnectionDescription> desc = new ConnectionDescription();
    if( desc->fromString( packet->connectionDescription ))
    {
        RefPtr<Connection> connection = Connection::create( desc->type );
        connection->setDescription( desc );
        if( connection->connect( ))
        {
            // send connect packet to peer
            eqNet::NodeConnectPacket connectPacket;
            connectPacket.nodeID    = _id;

            _connectionSet.addConnection( connection );
            if( _listener.isValid( ))
                connection->send( connectPacket, 
                                  _listener->getDescription()->toString( ));
            else
                connection->send( connectPacket );

            packet->nextIndex = NEXT_INDEX_CONNECTING;
            return COMMAND_REDISPATCH;
        }
    }

    if( packet->nextIndex == NEXT_INDEX_NONE )
    {
        EQWARN << "Connection to node " << nodeID << " failed" << endl;
        if( packet->appRequest )
            _requestHandler.serveRequest( requestID, 0 );
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
    command.getNode()->send( reply );
    
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
    return 0;
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

    bool success;
    const uint32_t time = static_cast<uint32_t>( _launchTimeout.getTimef( ));
    localNode->_requestHandler.waitRequest( _launchID, &success, time );
                      
    
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

RefPtr<Node> Node::connect( const NodeID& nodeID, RefPtr<Node> server)
{
    NodeIDHash< eqBase::RefPtr<Node> >::const_iterator iter = 
        _nodes.find( nodeID );
    if( iter != _nodes.end( ))
        return iter->second;

    NodeGetConnectionDescriptionPacket packet;
    packet.requestID  = _requestHandler.registerRequest();
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

    this->ref();
    _launchID = localNode->_requestHandler.registerRequest( this );
    _launchTimeout.setAlarm( description->launchTimeout );

    const string launchCommand = _createLaunchCommand( description );

    if( !eqBase::Launcher::run( launchCommand ))
    {
        EQWARN << "Could not launch node using '" << launchCommand << "'" 
               << endl;
        localNode->_requestHandler.unregisterRequest( _launchID );
        _launchID = EQ_ID_INVALID;
        return false;
    }
    
    _state    = STATE_LAUNCHED;
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

    //----- environment
#ifdef Darwin
    const char libPath[] = "DYLD_LIBRARY_PATH";
#else
    const char libPath[] = "LD_LIBRARY_PATH";
#endif

    stringStream << "env "; // XXX
    char* env = getenv( libPath );
    if( env )
        stringStream << libPath << "=" << env << " ";

    for( int i=0; environ[i] != 0; i++ )
        if( strlen( environ[i] ) > 2 && strncmp( environ[i], "EQ_", 3 ) == 0 )
            stringStream << environ[i] << " ";

    stringStream << "EQ_LOG_LEVEL=" << eqBase::Log::getLogLevelString() << " ";
    if( eqBase::Log::topics != 0 )
        stringStream << "EQ_LOG_TOPICS=" << eqBase::Log::topics << " ";

    //----- program + args
    string program = _programName;
    if( program[0] != '/' )
        program = _workDir + '/' + program;

    stringStream << "'" << program << " --eq-listen=\"" 
                 << nodeDesc->toString() << "\" --eq-client \""
                 << _launchID << ":" << _workDir << ":"
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
    const uint32_t launchID    = strtoul( request.c_str(), 0, 10 );
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
    
    EQINFO << "Launching node with launch ID=" << launchID << ", cwd="
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

    RefPtr<Node>       server = createNode( REASON_CLIENT_LAUNCH );
    NodeLaunchedPacket packet;
    packet.nodeID    = _id;
    packet.launchID  = launchID;

    server->ref();
    packet.requestID = _requestHandler.registerRequest( server.get( ));

    _connectionSet.addConnection( connection );
    connection->send( packet, _listener->getDescription()->toString( ));

    _requestHandler.waitRequest( packet.requestID );

    _clientRunning = true;
    clientLoop();
    _clientRunning = false;

    const bool joined = _receiverThread->join();
    EQASSERT( joined );
    _cleanup();
    return true;
}
