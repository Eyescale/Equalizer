
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
#include "launcher.h"
#include "packets.h"
#include "pipeConnection.h"
#include "session.h"

#include <alloca.h>
#include <fcntl.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

extern char **environ;

#define MAX_PACKET_SIZE (4096)

//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node( const uint32_t nCommands )
        : Base( nCommands ),
          _autoLaunch(false),
          _state(STATE_STOPPED),
          _launchID(EQ_INVALID_ID)
{
    EQASSERT( nCommands >= CMD_NODE_CUSTOM );
    registerCommand( CMD_NODE_STOP, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdStop ));
    registerCommand( CMD_NODE_MAP_SESSION, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdMapSession ));
    registerCommand( CMD_NODE_MAP_SESSION_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdMapSessionReply ));
    registerCommand( CMD_NODE_UNMAP_SESSION, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdUnmapSession ));
    registerCommand( CMD_NODE_UNMAP_SESSION_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdUnmapSessionReply ));

    _receiverThread = new ReceiverThread( this );

#ifdef CHECK_THREADSAFETY
    _threadID = 0;
#endif
}

Node::~Node()
{
    delete _receiverThread;
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
        _connectionSet.addConnection( connection, this );
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
    _connection->close();
    _connection = NULL;
    _listener   = NULL;

    const size_t nConnections = _connectionSet.nConnections();
    for( size_t i = 0; i<nConnections; i++ )
    {
        RefPtr<Connection> connection = _connectionSet.getConnection(i);
        RefPtr<Node>       node       = _connectionSet.getNode( connection );

        node->_state      = STATE_STOPPED;
        node->_connection = NULL;
    }

    _connectionSet.clear();
    _nodes.clear();
}

bool Node::_listenToSelf()
{
    // setup local connection to myself
    _connection = Connection::create(TYPE_UNI_PIPE);
    eqBase::RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;
    connDesc->type = TYPE_UNI_PIPE;

    if( !_connection->connect( connDesc ))
    {
        EQERROR << "Could not create pipe() connection to receiver thread."
              << endl;
        _connection = NULL;
        return false;
    }

    // add to connection set
    EQASSERT( _connection->getDescription().isValid( ));
    _connectionSet.addConnection( _connection, this );
    _nodes[ _id ] = this;
    return true;
}

void Node::_addConnectedNode( RefPtr<Node> node, RefPtr<Connection> connection )
{
    EQASSERT( node.isValid( ));
    EQASSERT( _state == STATE_LISTENING );
    EQASSERT( connection->getState() == Connection::STATE_CONNECTED );
    EQASSERT( node->_state == STATE_STOPPED || node->_state == STATE_LAUNCHED );
    EQASSERT( connection->getDescription().isValid( ));

    node->_connection = connection;
    node->_state      = STATE_CONNECTED;
    
    _connectionSet.addConnection( connection, node );
    _nodes[ node->_id ] = node;
    EQINFO << node.get() << " connected to " << this << endl;
}

bool Node::connect( RefPtr<Node> node, RefPtr<Connection> connection )
{
    if( !node.isValid() || _state != STATE_LISTENING ||
        connection->getState() != Connection::STATE_CONNECTED ||
        node->_state != STATE_STOPPED )
        return false;

    NodeConnectPacket* packet = _performConnect( connection );

    if( !packet )
        return false;

    EQASSERT( !packet->wasLaunched );

    node->_id = packet->nodeID;

    free( packet );
    _addConnectedNode( node, connection );

    EQINFO << node.get() << " connected to " << this << endl;
    return true;
}

NodeConnectPacket* Node::_performConnect( eqBase::RefPtr<Connection> connection)
{
    // send connect packet to peer
    eqNet::NodeConnectPacket packet;

    packet.nodeID = _id;
    connection->send( packet );

    return _readConnectReply( connection );
}

NodeConnectPacket* Node::_readConnectReply( eqBase::RefPtr<Connection> 
                                            connection )
{
    // receive and verify connect packet from peer
    uint64_t size;
    bool gotData = connection->recv( &size, sizeof( size ));
    EQASSERT( gotData ); EQASSERT( size );

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

bool Node::disconnect( Node* node )
{
    if( !node || _state != STATE_LISTENING || 
        !node->_state == STATE_CONNECTED || !node->_connection )
        return false;

    if( !_connectionSet.removeConnection( node->_connection ))
        return false;

    _nodes.erase( node->_id );

    node->_state      = STATE_STOPPED;
    node->_connection = NULL;
    EQINFO << node << " disconnected from " << this << endl;
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

bool Node::send( const MessageType type, const void *ptr, const uint64_t count )
{
    if( !checkConnection() )
        return false;
    
    NodeMessagePacket packet;
    packet.type = type;
    packet.nElements = count;

    if( !send( packet ))
        return false;

    const uint64_t size = _getMessageSize( type, count );
    return ( _connection->send( ptr, size ) == size );
}


//----------------------------------------------------------------------
// Node functionality
//----------------------------------------------------------------------
void Node::addSession( Session* session, RefPtr<Node> server, 
                       const uint32_t sessionID, const string& name )
{
    CHECK_THREAD;

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
    CHECK_THREAD;
    _sessions.erase( session->getID( ));

    session->_localNode = NULL;
    session->_server    = NULL;
    session->_id        = EQ_INVALID_ID;
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
    EQASSERT( id != EQ_INVALID_ID );

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
    session->send( packet );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

uint32_t Node::_generateSessionID()
{
    CHECK_THREAD;
    uint32_t id = EQ_INVALID_ID;

    while( id == EQ_INVALID_ID || _sessions.find( id ) != _sessions.end( ))
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
ssize_t Node::_runReceiver()
{
    EQINFO << "Receiver started" << endl;

    if( !getLocalNode( ))
        setLocalNode( this );

    int nErrors = 0;
    while( _state == STATE_LISTENING )
    {
        const int result = _connectionSet.select( );
        switch( result )
        {
            case ConnectionSet::EVENT_CONNECT:
                _handleConnect( _connectionSet );
                break;

            case ConnectionSet::EVENT_DATA:      
            {
                RefPtr<Connection> connection = _connectionSet.getConnection();
                RefPtr<Node>       node = _connectionSet.getNode( connection );
                EQASSERT( node->_connection == connection );
                _handleRequest( node.get() ); // do not pass down RefPtr atm
                break;
            }

            case ConnectionSet::EVENT_DISCONNECT:
            {
                _handleDisconnect( _connectionSet );
                EQVERB << &_connectionSet << endl;
                break;
            } 

            case ConnectionSet::EVENT_TIMEOUT:   
                EQINFO << "select timeout" << endl;
                break;

            case ConnectionSet::EVENT_ERROR:      
                ++nErrors;
                EQWARN << "Error during select" << endl;
                if( nErrors > 9 )
                {
                    EQWARN << "Too many errors in a row, capping connection" 
                           << endl;
                    _handleDisconnect( _connectionSet );
                }
                break;

            default:
                EQERROR << "UNIMPLEMENTED" << endl;
        }
        if( result != ConnectionSet::EVENT_ERROR )
            nErrors = 0;
    }

    return EXIT_SUCCESS;
}

void Node::_handleConnect( ConnectionSet& connectionSet )
{
    RefPtr<Connection> connection = connectionSet.getConnection();
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
        EQASSERT( requestID != EQ_INVALID_ID );

        node->_id = packet->nodeID;
        _addConnectedNode( node, connection );
        _requestHandler.serveRequest( requestID, NULL );
    }
    else
    {
        node = createNode();
        node->_id = packet->nodeID;

        _addConnectedNode( node, connection );
    }

    free( packet );
}

void Node::_handleDisconnect( ConnectionSet& connectionSet )
{
    RefPtr<Connection> connection = connectionSet.getConnection();
    RefPtr<Node>       node       = connectionSet.getNode( connection );

    handleDisconnect( node.get() ); // XXX
    connection->close();
}

void Node::handleDisconnect( Node* node )
{
    const bool disconnected = disconnect( node );
    EQASSERT( disconnected );
}

void Node::_handleRequest( Node* node )
{
    EQVERB << "Handle request from " << node << endl;

    uint64_t size;
    const uint64_t read = node->_connection->recv( &size, sizeof( size ));
    if( read == 0 ) // Some systems signal data on dead connections.
        return;

    EQASSERT( read == sizeof( size ));
    EQASSERT( size );

    // limit size due to use of alloca(). TODO: implement malloc-based recv?
    EQASSERT( size <= MAX_PACKET_SIZE );

    Packet* packet = (Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char*      ptr     = (char*)packet + sizeof(size);
    const bool gotData = node->recv( ptr, size );
    EQASSERT( gotData );

    const CommandResult result = dispatchPacket( node, packet );

    _redispatchPackets();

    switch( result )
    {
        case COMMAND_HANDLED:
            break;

        case COMMAND_ERROR:
            EQERROR << "Error handling command packet" << endl;
            abort();
            break;

        case COMMAND_RESCHEDULE:
        {
            Request* request = _requestCache.alloc( node, packet );
            _pendingRequests.push_back( request );
            break;
        }
    }
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
            {
                list<Request*>::iterator handledIter = iter;
                ++iter;
                _pendingRequests.erase( handledIter );
                _requestCache.release( request );
            }
            break;

            case COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();
                break;
                
            case COMMAND_RESCHEDULE:
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
        case DATATYPE_EQNET_MOBJECT:
        case DATATYPE_EQNET_USER:
        {
            const SessionPacket* sessionPacket = (SessionPacket*)packet;
            const uint32_t       id            = sessionPacket->sessionID;
            Session*             session       = _sessions[id];
            EQASSERTINFO( session, id );
            
            return session->dispatchPacket( node, sessionPacket );
        }

        default:
            if( datatype < DATATYPE_CUSTOM )
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
    CHECK_THREAD;
    
    Session*       session   = NULL;
    const uint32_t sessionID = packet->sessionID;
    string         sessionName;
    
    if( node == this ) // local mapping
    {
        if( sessionID == EQ_INVALID_ID ) // mapped by name
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
        if( sessionID == EQ_INVALID_ID ) // mapped by name
        {
            sessionName = packet->name;
            session     = _findSession( sessionName );
        
            if( !session ) // session does not exist, wait until master maps it
                return COMMAND_RESCHEDULE;
        }
        else // mapped by identifier, session has to exist already
        {
            session = _sessions[sessionID];
            if( session )
                sessionName = session->getName();
        }
    }

    NodeMapSessionReplyPacket reply( packet );
    reply.sessionID  = session ? session->getID() : EQ_INVALID_ID;
                
    node->send( reply, sessionName );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdMapSessionReply( Node* node, const Packet* pkg)
{
    NodeMapSessionReplyPacket* packet  = (NodeMapSessionReplyPacket*)pkg;
    EQINFO << "Cmd map session reply: " << packet << endl;
    CHECK_THREAD;

    const uint32_t requestID = packet->requestID;
    if( packet->sessionID == EQ_INVALID_ID )
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
    CHECK_THREAD;
    
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
    CHECK_THREAD;

    const uint32_t requestID = packet->requestID;

    if( !packet->result == EQ_INVALID_ID )
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

//----------------------------------------------------------------------
// utility functions
//----------------------------------------------------------------------
Session* Node::_findSession( const std::string& name ) const
{
    CHECK_THREAD;
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

    Node* localNode = Node::getLocalNode();
    if( !localNode )
        return false;

    // try connection first
    const size_t nDescriptions = nConnectionDescriptions();
    for( size_t i=0; i<nDescriptions; i++ )
    {
        RefPtr<ConnectionDescription> description = getConnectionDescription(i);
        RefPtr<Connection> connection = Connection::create( description->type );
        
        if( !connection->connect( description ))
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
    Node* localNode = Node::getLocalNode();
    EQASSERT( localNode )

    if( _launchID == EQ_INVALID_ID )
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

    _launchID = EQ_INVALID_ID;
    return success;
}

bool Node::_launch( RefPtr<ConnectionDescription> description )
{
    EQASSERT( _state == STATE_STOPPED );

    Node* localNode = Node::getLocalNode();
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
    if( description->launchCommand.size() == 0 )
        return description->launchCommand;

    const string& launchCommand    = description->launchCommand;
    const size_t  launchCommandLen = launchCommand.size();
    bool          commandFound     = false;
    size_t        lastPos          = 0;
    string        result;

    for( size_t percentPos = launchCommand.find( '%' );
         percentPos != string::npos; 
         percentPos = launchCommand.find( '%', percentPos+1 ))
    {
        string replacement;
        switch( launchCommand[percentPos+1] )
        {
            case 'c':
            {
                replacement = _createRemoteCommand();
                commandFound = true;
                break;
            }
            case 'h':
                replacement  = description->hostname;
                break;

            default:
                EQWARN << "Unknown token " << launchCommand[percentPos+1] << endl;
        }

        if( replacement.size() > 0 )
        {
            result  += launchCommand.substr( lastPos, percentPos-lastPos );
            result  += replacement;
        }
        else
            result += launchCommand.substr( lastPos, percentPos-lastPos );

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
    Node* localNode = Node::getLocalNode();
    if( !localNode )
    {
        EQERROR << "No local node, can't launch " << this << endl;
        return "";
    }

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

    stringStream << "'" << getProgramName() << " --eq-listen=\"" 
                 << nodeDesc->toString() << "\" --eq-client \""
                 << (long long)this << ":" << listenerDesc->toString() << "\"'";

    return stringStream.str();
}

bool Node::runClient( const string& clientArgs )
{
    EQASSERT( _state == STATE_LISTENING );

    const size_t colonPos = clientArgs.find( ':' );
    if( colonPos == string::npos )
    {
        EQERROR << "Could not parse request identifier" << endl;
        return false;
    }

    const string   request    = clientArgs.substr( 0, colonPos );
    const uint64_t requestID  = atoll( request.c_str( ));
    const string   serverDesc = clientArgs.substr( colonPos + 1 );

    RefPtr<ConnectionDescription> connectionDesc = new ConnectionDescription;
    if( !connectionDesc->fromString( serverDesc ))
    {
        EQERROR << "Could not parse connection description" << endl;
        return false;
    }

    RefPtr<Connection> connection = Connection::create( connectionDesc->type );
    if( !connection->connect( connectionDesc ))
    {
        EQERROR << "Can't contact node" << endl;
        return false;
    }

    RefPtr<Node> node = createNode();
    if( !node.isValid() )
    {
        EQERROR << "Can't create node" << endl;
        return false;
    }
    
    NodeConnectPacket packet;
    packet.wasLaunched = true;
    packet.launchID    = requestID;

    connection->send( packet );

    NodeConnectPacket* reply = _readConnectReply( connection );

    if( !reply )
        return false;

    EQASSERT( !reply->wasLaunched );

    node->_id = reply->nodeID;

    free( reply );
    _addConnectedNode( node, connection );

    clientLoop();

    const bool joined = _receiverThread->join();
    EQASSERT( joined );
    _cleanup();
    return true;
}
