
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
#include "launcher.h"
#include "packets.h"
#include "pipeConnection.h"
#include "session.h"

#include <alloca.h>
#include <sstream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

extern char **environ;

//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node()
        : _state(STATE_STOPPED),
          _autoLaunch(false),
          _pendingRequestID(INVALID_ID)
{
    for( int i=0; i<CMD_NODE_CUSTOM; i++ )
        _cmdHandler[i] =  &eqNet::Node::_cmdUnknown;

    _cmdHandler[CMD_NODE_STOP]              = &eqNet::Node::_cmdStop;
    _cmdHandler[CMD_NODE_MAP_SESSION]       = &eqNet::Node::_cmdMapSession;
    _cmdHandler[CMD_NODE_MAP_SESSION_REPLY] = &eqNet::Node::_cmdMapSessionReply;

    _receiverThread    = new ReceiverThread( this );
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

    if( connection.isValid() )
    {
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
    _receiverThread->join();
    _cleanup();
    return true;
}

void Node::_cleanup()
{
    ASSERT( _state == STATE_STOPPED );
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
    if( !_connection->connect( connDesc ))
    {
        ERROR << "Could not create pipe() connection to receiver thread."
              << endl;
        _connection = NULL;
        return false;
    }

    // add to connection set
    _connectionSet.addConnection( _connection, this );
    return true;
}

void Node::_addConnectedNode( RefPtr<Node> node, RefPtr<Connection> connection )
{
    ASSERT( node.isValid( ));
    ASSERT( _state == STATE_LISTENING );
    ASSERT( connection->getState() == Connection::STATE_CONNECTED );
    ASSERT( node->_state == STATE_STOPPED || node->_state == STATE_LAUNCHED );

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
    
    NodeConnectPacket packet;
    node->send( packet );

    _connectionSet.addConnection( connection, node );
    INFO << node.get() << " connected to " << this << endl;
    return true;
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
// Node functionality
//----------------------------------------------------------------------
uint64 Node::_getMessageSize( const MessageType type, const uint64 count )
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

bool Node::send( const MessageType type, const void *ptr, const uint64 count )
{
    NodeMessagePacket packet;
    packet.type = type;
    packet.nElements = count;

    if( !send( packet ))
        return false;

    const uint64 size = _getMessageSize( type, count );
    if( !send( ptr, size ))
        return false;

    return true;
}

bool Node::mapSession( Node* server, Session* session, const std::string& name )
{
    if( server == this && isLocal( ))
    {            
        uint sessionID = INVALID_ID;
        while( sessionID == INVALID_ID ||
               _sessions.find( sessionID ) != _sessions.end( ))
        {
            srandomdev();
            sessionID = random();
        }
            
        session->map( server, sessionID, name, true );
        _sessions[sessionID] = session;
        return true;
    }

    NodeMapSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    packet.nameLength =  name.size() + 1;
    server->send( packet );
    server->send( name.c_str(), packet.nameLength );

    return (bool)_requestHandler.waitRequest( packet.requestID );
}

bool Node::mapSession( Node* server, Session* session, const uint id )
{
    ASSERT( id != INVALID_ID );

    NodeMapSessionPacket packet;
    packet.requestID = _requestHandler.registerRequest( session );
    packet.sessionID =  id;
    server->send( packet );

    return (bool)_requestHandler.waitRequest( packet.requestID );
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
                _handleRequest( node.get() ); // do not pass down RefPtr for now
                break;
            }

            case ConnectionSet::EVENT_DISCONNECT:
            {
                _handleDisconnect( _connectionSet );
                break;
            } 

            case ConnectionSet::EVENT_TIMEOUT:   
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
    bool gotData = connection->recv( &packet, sizeof( NodeConnectPacket ));
    ASSERT( gotData );
    
    RefPtr<Node> node;

    if( packet.wasLaunched )
    {
        //ASSERT( dynamic_cast<Node*>( (Thread*)packet.launchID ));

        node = (Node*)packet.launchID;
        INFO << "Launched " << node.get() << " connecting" << endl;
 
        const uint requestID = node->_pendingRequestID;
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

void Node::_handleRequest( Node* node )
{
    VERB << "Handle request from " << node << endl;

    uint64 size;
    bool gotData = node->recv( &size, sizeof( size ));
    ASSERT( gotData );
    ASSERT( size );

    Packet* packet = (Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char* ptr = (char*)packet + sizeof(size);
    gotData   = node->recv( ptr, size );
    ASSERT( gotData );

    _handlePacket( node, packet );
}

void Node::_handlePacket( Node* node, const Packet* packet )
{
    VERB << "handle " << packet << endl;
    const uint datatype = packet->datatype;

    switch( datatype )
    {
        case DATATYPE_EQNET_NODE:
            if( packet->command < CMD_NODE_CUSTOM )
                (this->*_cmdHandler[packet->command])(node, packet);
            else
                handlePacket( node, packet );
            break;

        case DATATYPE_EQNET_SESSION:
        case DATATYPE_EQNET_USER:
            ERROR << "unimplemented" << endl;
            abort();
            break;

        default:
            if( datatype < DATATYPE_CUSTOM )
                ERROR << "Unknown datatype " << datatype << ", dropping packet."
                      << endl;
            else
                handlePacket( node, packet );
            break;
    }
}

void Node::_cmdStop( Node* node, const Packet* pkg )
{
    INFO << "Cmd stop" << endl;
    ASSERT( _state == STATE_LISTENING );

//     _connectionSet.clear();
//     _connection = NULL;
    _state = STATE_STOPPED;
    _receiverThread->exit( EXIT_SUCCESS );
}

void Node::_cmdMapSession( Node* node, const Packet* pkg )
{
    ASSERT( getState() == STATE_LISTENING );

    NodeMapSessionPacket* packet  = (NodeMapSessionPacket*)pkg;
    INFO << "Cmd map session: " << packet << endl;
    
    Session* session;
    uint     sessionID   = packet->sessionID;
    char*    sessionName = NULL;

    if( sessionID == INVALID_ID ) // mapped by name
    {
        sessionName        = (char*)alloca( packet->nameLength );
        const bool gotData = node->recv( sessionName, packet->nameLength );
        ASSERT( gotData );

        session = _findSession( sessionName );

        if( session )
            sessionID = session->getID();
        else
        {
            session = new Session();
            
            while( sessionID == INVALID_ID ||
                   _sessions.find( sessionID ) != _sessions.end( ))
            {
                srandomdev();
                sessionID = random();
            }
            
            session->map( this, sessionID, sessionName, true );
            _sessions[sessionID] = session;
        }
    }
    else // mapped by identifier, session has to exist already
    {
        session = _sessions[sessionID];
        if( !session )
            sessionID = INVALID_ID;
        else
            sessionName = (char*)session->getName().c_str();
    }

    NodeMapSessionReplyPacket reply( packet );
    reply.sessionID  = sessionID;
    reply.nameLength = sessionName ? strlen(sessionName) + 1 : 0;

    node->send( reply );
    if( reply.nameLength )
        node->send( sessionName, reply.nameLength );
}

void Node::_cmdMapSessionReply( Node* node, const Packet* pkg)
{
    NodeMapSessionReplyPacket* packet  = (NodeMapSessionReplyPacket*)pkg;
    INFO << "Cmd map session reply: " << packet << endl;

    const uint requestID = packet->requestID;
    Session*   session   = (Session*)_requestHandler.getRequestData( requestID);
    ASSERT( session );

    if( packet->sessionID == INVALID_ID )
    {
        _requestHandler.serveRequest( requestID, (void*)false );
        return;
    }        
    
    ASSERT( packet->nameLength );
    char*      sessionName = (char*)alloca( packet->nameLength );
    const bool gotData     = node->recv( sessionName, packet->nameLength );
    ASSERT( gotData );
        
    session->map( node, packet->sessionID, sessionName, false );
    _sessions[packet->sessionID] = session;
    _requestHandler.serveRequest( requestID, (void*)true );
}

//----------------------------------------------------------------------
// utility functions
//----------------------------------------------------------------------
Session* Node::_findSession( const std::string& name ) const
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
    if( _state==STATE_CONNECTED || _state==STATE_LISTENING )
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
    if( _state==STATE_CONNECTED || _state==STATE_LISTENING )
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
        
        if( connection->connect( description ))
            return localNode->connect( this, connection );
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
        localNode->_requestHandler.unregisterRequest( _pendingRequestID );
        _pendingRequestID = INVALID_ID;
        return true;
    }

    ASSERT( _state == STATE_LAUNCHED );
    ASSERT( _pendingRequestID != INVALID_ID );

    ConnectionDescription *description = (ConnectionDescription*)
        localNode->_requestHandler.getRequestData( _pendingRequestID );

    bool success;
    localNode->_requestHandler.waitRequest( _pendingRequestID, &success,
                                            description->launchTimeout );
    
    if( success )
    {
        ASSERT( _state == STATE_CONNECTED );
    }
    else
    {
        _state = STATE_STOPPED;
        localNode->_requestHandler.unregisterRequest( _pendingRequestID );
    }

    _pendingRequestID = INVALID_ID;
    return success;
}

bool Node::_launch( RefPtr<ConnectionDescription> description )
{
    ASSERT( _state == STATE_STOPPED );

    Node* localNode = Node::getLocalNode();
    ASSERT( localNode );

    const uint requestID = 
        localNode->_requestHandler.registerRequest( description.get( ));
    string launchCommand = _createLaunchCommand( description );

    if( !Launcher::run( launchCommand ))
    {
        WARN << "Could not launch node using '" << launchCommand << "'" << endl;
        localNode->_requestHandler.unregisterRequest( requestID );
        return false;
    }
    
    _state            = STATE_LAUNCHED;
    _pendingRequestID = requestID;
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

    RefPtr<ConnectionDescription> listenerDesc =
        listener->getConnectionDescription();
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
    INFO << "runClient, args: " << clientArgs << endl;

    const size_t colonPos = clientArgs.find( ':' );
    if( colonPos == string::npos )
    {
        ERROR << "Could not parse request identifier" << endl;
        return false;
    }

    const string request    = clientArgs.substr( 0, colonPos );
    const uint64 requestID  = atoll( request.c_str( ));
    const string serverDesc = clientArgs.substr( colonPos + 1 );

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

    _receiverThread->join();
    node->_cleanup();
    return true;
}
