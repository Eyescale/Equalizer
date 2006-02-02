
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
          _launchID(INVALID_ID)
{
    ASSERT( nCommands >= CMD_NODE_CUSTOM );
    registerCommand( CMD_NODE_STOP, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdStop ));
    registerCommand( CMD_NODE_MAP_SESSION, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdMapSession ));
    registerCommand( CMD_NODE_MAP_SESSION_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Node::_cmdMapSessionReply ));

    _receiverThread = new ReceiverThread( this );
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
        ASSERT( connection->getDescription().isValid( ));
        _connectionSet.addConnection( connection, this );
        _listener = connection;
    }

    _state = STATE_LISTENING;
    _receiverThread->start();

    if( !getLocalNode( ))
        setLocalNode( this );

    INFO << this << " listening." << endl;
    return true;
}

bool Node::stopListening()
{
    if( _state != STATE_LISTENING )
        return false;

    NodeStopPacket packet;
    send( packet );
    const bool joined = _receiverThread->join();
    ASSERT( joined );
    _cleanup();
    return true;
}

void Node::_cleanup()
{
    ASSERTINFO( _state == STATE_STOPPED, _state );
    ASSERT( _connection );

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
}

bool Node::_listenToSelf()
{
    // setup local connection to myself
    _connection = Connection::create(TYPE_UNI_PIPE);
    eqBase::RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;
    connDesc->type = TYPE_UNI_PIPE;

    if( !_connection->connect( connDesc ))
    {
        ERROR << "Could not create pipe() connection to receiver thread."
              << endl;
        _connection = NULL;
        return false;
    }

    // add to connection set
    ASSERT( _connection->getDescription().isValid( ));
    _connectionSet.addConnection( _connection, this );
    return true;
}

void Node::_addConnectedNode( RefPtr<Node> node, RefPtr<Connection> connection )
{
    ASSERT( node.isValid( ));
    ASSERT( _state == STATE_LISTENING );
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );
    ASSERT( node->_state == STATE_STOPPED || node->_state == STATE_LAUNCHED );
    ASSERT( connection->getDescription().isValid( ));

    node->_connection = connection;
    node->_state      = STATE_CONNECTED;
    
    _connectionSet.addConnection( connection, node );
    INFO << node.get() << " connected to " << this << endl;
}

bool Node::connect( RefPtr<Node> node, RefPtr<Connection> connection )
{
    if( !node.isValid() || _state != STATE_LISTENING ||
        connection->getState() != Connection::STATE_CONNECTED ||
        node->_state != STATE_STOPPED )
        return false;

    node->_connection = connection;
    node->_state      = STATE_CONNECTED;
    
    ASSERT( connection->getDescription().isValid( ));
    _connectionSet.addConnection( connection, node );
    INFO << node.get() << " connected to " << this << endl;
    return true;
}

RefPtr<Node> Node::_findConnectedNode( const char* connectionDescription )
{
    const size_t nConnections = _connectionSet.nConnections();
    for( size_t i=0; i<nConnections; i++ )
    {
        RefPtr<Connection>      connection = _connectionSet.getConnection(i);
        RefPtr<ConnectionDescription> desc = connection->getDescription();
        ASSERT( desc.isValid( ));

        if( desc->toString() == connectionDescription )
            return _connectionSet.getNode( connection );
    }
    return NULL;
}

RefPtr<Node> Node::getNodeWithConnection( const char* connectionDescription )
{
    RefPtr<Node> node = _findConnectedNode( connectionDescription );
    if( node.isValid( ))
        return node;

    node = createNode();
    RefPtr<ConnectionDescription> cd = new ConnectionDescription();
    if( !cd->fromString( connectionDescription ))
        return false;

    node->addConnectionDescription( cd );
    if( node->checkConnection( ))
        return node;

    return NULL;
}

bool Node::disconnect( Node* node )
{
    if( !node || _state != STATE_LISTENING || 
        !node->_state == STATE_CONNECTED || !node->_connection )
        return false;

    if( !_connectionSet.removeConnection( node->_connection ))
        return false;
    
    node->_state      = STATE_STOPPED;
    node->_connection = NULL;
    INFO << node << " disconnected from " << this << endl;
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

bool Node::send( Packet& packet, const std::string& string )
{
    if( !checkConnection() )
        return false;
    
    const uint32_t stringLen = string.size() + 1;

    if( stringLen <= 8 ) // fits in existing packet
    {
        memcpy( (char*)(&packet)+packet.size, string.c_str(), stringLen );
        packet.size += 8;
        const bool result = ( _connection->send( packet ) == packet.size );
        packet.size -= 8;
        return result;
    }

    uint64_t       size      = packet.size + stringLen;
    size += (4 - size%4);
    char*          buffer    = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size );
    memcpy( buffer+packet.size, string.c_str(), stringLen );

    ((Packet*)buffer)->size = size;
    return ( _connection->send( buffer, size ) == size );
}


//----------------------------------------------------------------------
// Node functionality
//----------------------------------------------------------------------
void Node::addSession( Session* session, RefPtr<Node> server, 
                       const uint32_t sessionID, const string& name )
{
    session->_localNode = this;
    session->_server    = server;
    session->_id        = sessionID;
    session->_name      = name;
    session->_isMaster  = ( server==this && isLocal( ));

    _sessions[sessionID] = session;

    INFO << (session->_isMaster ? "master" : "client") << " session, id "
         << sessionID << ", name " << name << ", served by " << server.get() 
         << ", managed by " << this << endl;
}

bool Node::mapSession( RefPtr<Node> server, Session* session, 
                       const string& name )
{
    ASSERT( isLocal( ));
    ASSERT( !_receiverThread->isCurrent( ));

    if( findSession( name )) // Already mapped [to another session instance]
        return false;

    if( server.get() == this )
    {
        const uint32_t sessionID = _generateSessionID();
        addSession( session, server, sessionID, name );
        return true;
    }

    NodeMapSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    server->send( packet, name );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

bool Node::mapSession( RefPtr<Node> server, Session* session, const uint32_t id)
{
    ASSERT( isLocal( ));
    ASSERT( id != INVALID_ID );

    if( _sessions.find( id ) != _sessions.end( )) 
        // Already mapped [to another session instance]
        return false;

    NodeMapSessionPacket packet;
    packet.requestID = _requestHandler.registerRequest( session );
    packet.sessionID =  id;
    server->send( packet );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

uint32_t Node::_generateSessionID()
{
    uint32_t id = INVALID_ID;

    while( id == INVALID_ID || _sessions.find( id ) != _sessions.end( ))
    {
#ifdef __linux__
        int fd = ::open( "/dev/random", O_RDONLY );
        ASSERT( fd != -1 );
        int read = ::read( fd, &id, sizeof( id ));
        ASSERT( read == sizeof( id ));
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
    INFO << "Receiver started" << endl;

    if( !getLocalNode( ))
        setLocalNode( this );

    while( _state == STATE_LISTENING )
    {
        switch( _connectionSet.select( ))
        {
            case ConnectionSet::EVENT_CONNECT:
                _handleConnect( _connectionSet );
                break;

            case ConnectionSet::EVENT_DATA:      
            {
                RefPtr<Connection> connection = _connectionSet.getConnection();
                RefPtr<Node>       node = _connectionSet.getNode( connection );
                ASSERT( node->_connection == connection );
                _handleRequest( node.get() ); // do not pass down RefPtr atm
                break;
            }

            case ConnectionSet::EVENT_DISCONNECT:
            {
                _handleDisconnect( _connectionSet );
                break;
            } 

            case ConnectionSet::EVENT_TIMEOUT:   
                INFO << "select timeout" << endl;
                break;

            case ConnectionSet::EVENT_ERROR:      
            default:
                ERROR << "UNIMPLEMENTED" << endl;
        }
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
    NodeConnectPacket packet;
    const bool gotData = 
        connection->recv( &packet, sizeof( NodeConnectPacket ));
    ASSERT( gotData );
    
    RefPtr<Node> node;

    if( packet.wasLaunched )
    {
        //ASSERT( dynamic_cast<Node*>( (Thread*)packet.launchID ));

        node = (Node*)packet.launchID;
        INFO << "Launched " << node.get() << " connecting" << endl;
 
        const uint32_t requestID = node->_launchID;
        ASSERT( requestID != INVALID_ID );

        _requestHandler.serveRequest( requestID, NULL );
    }
    else
    {
        node = createNode();
    }

    _addConnectedNode( node, connection );
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
    ASSERT( disconnected );
}

Session* Node::createSession() 
{
    return new Session();
}

void Node::_handleRequest( Node* node )
{
    VERB << "Handle request from " << node << endl;

    uint64_t size;
    bool gotData = node->recv( &size, sizeof( size ));
    ASSERT( gotData );
    ASSERT( size );

    // limit size due to use of alloca(). TODO: implement malloc-based recv?
    ASSERT( size <= MAX_PACKET_SIZE );

    Packet* packet = (Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char* ptr = (char*)packet + sizeof(size);
    gotData   = node->recv( ptr, size );
    ASSERT( gotData );

    const CommandResult result = dispatchPacket( node, packet );

    _redispatchPackets();

    switch( result )
    {
        case COMMAND_HANDLED:
            break;

        case COMMAND_ERROR:
            ERROR << "Error handling command packet" << endl;
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
                iter++;
                _pendingRequests.erase( handledIter );
                _requestCache.release( request );
            }
            break;

            case COMMAND_ERROR:
                ERROR << "Error handling command packet" << endl;
                abort();
                break;
                
            case COMMAND_RESCHEDULE:
                break;
        }
    }
}

CommandResult Node::dispatchPacket( Node* node, const Packet* packet )
{
    VERB << "dispatch " << packet << " from " << (void*)node << " by " 
         << (void*)this << endl;
    const uint32_t datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQNET_NODE:
            return handleCommand( node, packet );

        case DATATYPE_EQNET_SESSION:
        case DATATYPE_EQNET_OBJECT:
        case DATATYPE_EQNET_USER:
        {
            const SessionPacket* sessionPacket = (SessionPacket*)packet;
            const uint32_t       id            = sessionPacket->sessionID;
            Session*             session       = _sessions[id];
            ASSERTINFO( session, id );
            
            return session->dispatchPacket( node, sessionPacket );
        }

        default:
            if( datatype < DATATYPE_CUSTOM )
            {
                ERROR << "Unknown eqNet datatype " << datatype 
                      << ", dropping packet." << endl;
                return COMMAND_ERROR;
            }

            return handlePacket( node, packet );
    }
}

CommandResult Node::_cmdStop( Node* node, const Packet* pkg )
{
    INFO << "Cmd stop " << this << endl;
    ASSERT( _state == STATE_LISTENING );

    // TODO: Process pending packets on all other connections?

    _state = STATE_STOPPED;
    _receiverThread->exit( EXIT_SUCCESS );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdMapSession( Node* node, const Packet* pkg )
{
    ASSERT( getState() == STATE_LISTENING );

    NodeMapSessionPacket* packet  = (NodeMapSessionPacket*)pkg;
    INFO << "Cmd map session: " << packet << endl;
    
    Session* session;
    uint32_t sessionID   = packet->sessionID;
    string   sessionName;

    if( sessionID == INVALID_ID ) // mapped by name
    {
        sessionName = packet->name;
        session     = findSession( sessionName );
        
        if( !session ) // session does not exist, create new one
        {
            session   = createSession();
            sessionID = _generateSessionID();
            addSession( session, this, sessionID, sessionName );
        }
        else
            sessionID = session->getID();
    }
    else // mapped by identifier, session has to exist already
    {
        session = _sessions[sessionID];
        if( !session )
            sessionID = INVALID_ID;
        else
            sessionName = session->getName();
    }

    NodeMapSessionReplyPacket reply( packet );
    reply.sessionID  = sessionID;

    node->send( reply, sessionName );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdMapSessionReply( Node* node, const Packet* pkg)
{
    NodeMapSessionReplyPacket* packet  = (NodeMapSessionReplyPacket*)pkg;
    INFO << "Cmd map session reply: " << packet << endl;

    const uint32_t requestID = packet->requestID;
    Session*    session = (Session*)_requestHandler.getRequestData( requestID );
    ASSERT( session );

    if( packet->sessionID == INVALID_ID )
    {
        _requestHandler.serveRequest( requestID, (void*)false );
        return COMMAND_HANDLED;
    }        
    
    addSession( session, node, packet->sessionID, packet->name );
    _requestHandler.serveRequest( requestID, (void*)true );
    return COMMAND_HANDLED;
}

//----------------------------------------------------------------------
// utility functions
//----------------------------------------------------------------------
Session* Node::findSession( const std::string& name ) const
{
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
        ERROR << "Connection initialisation failed." << endl;
        return false;
    }

    return syncConnect();
}

bool Node::initConnect()
{
    if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
        return true;

    ASSERT( _state == STATE_STOPPED );

    Node* localNode = Node::getLocalNode();
    if( !localNode )
        return false;

    // try connection first
    const size_t nDescriptions = nConnectionDescriptions();
    for( size_t i=0; i<nDescriptions; i++ )
    {
        RefPtr<ConnectionDescription> description = getConnectionDescription(i);
        RefPtr<Connection> connection = Connection::create( description->type );
        
        if( connection->connect( description ) &&
            localNode->connect( this, connection ))
        {
            NodeConnectPacket packet;
            send( packet );
            return true;
        }
        return false;
    }

    INFO << "Node could not be connected." << endl;
    if( !_autoLaunch )
        return false;
    
    INFO << "Attempting to launch node." << endl;
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
    ASSERT( localNode )

    if( _state == STATE_CONNECTED )
    {
        localNode->_requestHandler.unregisterRequest( _launchID );
        _launchID = INVALID_ID;
        return true;
    }

    ASSERT( _state == STATE_LAUNCHED );
    ASSERT( _launchID != INVALID_ID );

    ConnectionDescription *description = (ConnectionDescription*)
        localNode->_requestHandler.getRequestData( _launchID );

    bool success;
    localNode->_requestHandler.waitRequest( _launchID, &success,
                                            description->launchTimeout );
    
    if( success )
    {
        ASSERT( _state == STATE_CONNECTED );
    }
    else
    {
        _state = STATE_STOPPED;
        localNode->_requestHandler.unregisterRequest( _launchID );
    }

    _launchID = INVALID_ID;
    return success;
}

bool Node::_launch( RefPtr<ConnectionDescription> description )
{
    ASSERT( _state == STATE_STOPPED );

    Node* localNode = Node::getLocalNode();
    ASSERT( localNode );

    const uint32_t requestID     = 
        localNode->_requestHandler.registerRequest( description.get( ));
    string         launchCommand = _createLaunchCommand( description );

    if( !Launcher::run( launchCommand ))
    {
        WARN << "Could not launch node using '" << launchCommand << "'" << endl;
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
                WARN << "Unknown token " << launchCommand[percentPos+1] << endl;
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

    INFO << "Launch command: " << result << endl;
    return result;
}

string Node::_createRemoteCommand()
{
    Node* localNode = Node::getLocalNode();
    if( !localNode )
    {
        ERROR << "No local node, can't launch " << this << endl;
        return "";
    }

    RefPtr<Connection> listener = localNode->getListenerConnection();
    if( !listener.isValid() || 
        listener->getState() != Connection::STATE_LISTENING )
    {
        ERROR << "local node is not listening, can't launch " << this << endl;
        return "";
    }

    RefPtr<ConnectionDescription> listenerDesc = listener->getDescription();
    ASSERT( listenerDesc.isValid( ));

    ostringstream stringStream;

    stringStream << "env "; // XXX
    char* env = getenv( "DYLD_LIBRARY_PATH" );
    if( env )
        stringStream << "DYLD_LIBRARY_PATH=" << env << " ";

    env = getenv( "EQLOGLEVEL" );
    if( env )
        stringStream << "EQLOGLEVEL=" << env << " ";
    // for( int i=0; environ[i] != NULL; i++ )
    // {
    //     replacement += environ[i];
    //     replacement += " ";
    // }

    stringStream << "'" << getProgramName() << " --eq-listen --eq-client "
                 << (long long)this << ":" << listenerDesc->toString() << "'";

    return stringStream.str();
}

bool Node::runClient( const string& clientArgs )
{
    ASSERT( _state == STATE_LISTENING );

    const size_t colonPos = clientArgs.find( ':' );
    if( colonPos == string::npos )
    {
        ERROR << "Could not parse request identifier" << endl;
        return false;
    }

    const string   request    = clientArgs.substr( 0, colonPos );
    const uint64_t requestID  = atoll( request.c_str( ));
    const string   serverDesc = clientArgs.substr( colonPos + 1 );

    RefPtr<ConnectionDescription> connectionDesc = new ConnectionDescription;
    if( !connectionDesc->fromString( serverDesc ))
    {
        ERROR << "Could not parse connection description" << endl;
        return false;
    }

    RefPtr<Connection> connection = Connection::create( connectionDesc->type );
    if( !connection->connect( connectionDesc ))
    {
        ERROR << "Can't contact node" << endl;
        return false;
    }

    RefPtr<Node> node = createNode();
    if( !node.isValid() )
    {
        ERROR << "Can't create node" << endl;
        return false;
    }

    _addConnectedNode( node, connection );

    NodeConnectPacket packet;
    packet.wasLaunched = true;
    packet.launchID    = requestID;
    node->send( packet );

    clientLoop();

    const bool joined = _receiverThread->join();
    ASSERT( joined );
    _cleanup();
    return true;
}
