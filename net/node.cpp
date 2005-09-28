
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
#include "packets.h"
#include "pipeConnection.h"
#include "session.h"

#include <alloca.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node()
        : _state(STATE_STOPPED),
          _sessionID(0)
{
    for( int i=0; i<CMD_NODE_CUSTOM; i++ )
        _cmdHandler[i] =  &eqNet::Node::_cmdUnknown;

    _cmdHandler[CMD_NODE_STOP]              = &eqNet::Node::_cmdStop;
    _cmdHandler[CMD_NODE_MAP_SESSION]       = &eqNet::Node::_cmdMapSession;
    _cmdHandler[CMD_NODE_MAP_SESSION_REPLY] = &eqNet::Node::_cmdMapSessionReply;
    _cmdHandler[CMD_NODE_SESSION]           = &eqNet::Node::_cmdSession;
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
        _connectionSet.addConnection( connection, this );
    _state = STATE_LISTENING;

    // run the receiver thread
    start();
    INFO << this << " listening" << endl;
    return true;
}

bool Node::stopListening()
{
    if( _state != STATE_LISTENING )
        return false;

    NodeStopPacket packet;
    send( packet );
    join();

    ASSERT( _state == STATE_STOPPED );
    ASSERT( _connection );

    _connectionSet.removeConnection( _connection );
    _connection->close();
    _connection = NULL;

    const size_t nConnections = _connectionSet.nConnections();
    for( size_t i = 0; i<nConnections; i++ )
    {
        RefPtr<Connection> connection = _connectionSet.getConnection(i);
        Node*              node       = _connectionSet.getNode( connection );

        node->_state      = STATE_STOPPED;
        node->_connection = NULL;
    }

    _connectionSet.clear();    
    _state = STATE_STOPPED;
    return true;    
}

bool Node::_listenToSelf()
{
    // setup local connection to myself
    _connection = Connection::create(TYPE_UNI_PIPE);
    ConnectionDescription connDesc;
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

bool Node::connect( Node* node, RefPtr<Connection> connection )
{
    if( !node || _state != STATE_LISTENING ||
        connection->getState() != Connection::STATE_CONNECTED )
        return false;

    node->_connection = connection;
    node->_state      = STATE_CONNECTED;
    _connectionSet.addConnection( connection, node );

    INFO << node << " connected to " << this << endl;
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
    ASSERT( _state == STATE_CONNECTED ); // TODO: local send
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
    NodeMapSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    packet.nameLength =  name.size() + 1;
    server->send( packet );
    server->send( name.c_str(), packet.nameLength );

    const void* result = _requestHandler.waitRequest( packet.requestID );
    const uint sessionID = (int)((long long)result); // casts needed for MipsPro

    if( sessionID == INVALID_ID )
        return false;

    session->map( server, sessionID, name );
    return true;
}


//----------------------------------------------------------------------
// receiver thread functions
//----------------------------------------------------------------------
ssize_t Node::run()
{
    INFO << "Receiver started" << endl;

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
                Node*              node = _connectionSet.getNode( connection );
                ASSERT( node->_connection == connection );
                _handleRequest( node );
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
    Node*              newNode    = handleConnect( newConn );
    
    if( !newNode )
        newConn->close();
    else
        INFO << "New " << newNode << endl;
}

Node* Node::handleConnect( RefPtr<Connection> connection )
{
    Node* node = new Node();
    if( connect( node, connection ))
        return node;

    delete node;
    return NULL;
}

void Node::_handleDisconnect( ConnectionSet& connectionSet )
{
    RefPtr<Connection> connection = connectionSet.getConnection();
    Node*              node       = connectionSet.getNode( connection );

    handleDisconnect( node );
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
                handleCommand( node, (const NodePacket*)packet );
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
    Thread::exit( EXIT_SUCCESS );
}

void Node::_cmdMapSession( Node* node, const Packet* pkg )
{
    ASSERT( getState() == STATE_LISTENING );

    NodeMapSessionPacket* packet  = (NodeMapSessionPacket*)pkg;
    char*                 name    = (char*)alloca( packet->nameLength );
    const bool            gotData = node->recv( name, packet->nameLength );
    
    ASSERT( gotData );
    INFO << "Cmd map session: " << packet << ", " << name << endl;

    Session* session   = _findSession(name);
    uint     sessionID;
    if( !session )
    {
        sessionID = _sessionID++;
        session   = new Session();
        session->map( this, sessionID, name );
        _sessions[sessionID] = session;
    }
    else
        sessionID = session->getID();
    
    NodeSessionPacket sessionPacket;
    sessionPacket.requestID = packet->requestID;
    sessionPacket.sessionID = sessionID;

    node->send( sessionPacket );
    session->pack( node );

    NodeMapSessionReplyPacket replyPacket( packet );
    replyPacket.reply = sessionID;
    node->send(replyPacket);
}

void Node::_cmdMapSessionReply( Node* node, const Packet* pkg)
{
    NodeMapSessionReplyPacket* packet  = (NodeMapSessionReplyPacket*)pkg;
    INFO << "Cmd map session reply: " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)packet->reply );
}

void Node::_cmdSession( Node* node, const Packet* pkg )
{
    NodeSessionPacket* packet  = (NodeSessionPacket*)pkg;
    INFO << "cmd session: " << packet << endl;
    Session* session = static_cast<Session*>(
        _requestHandler.getRequestData( packet->requestID ));
    ASSERT( session );
    
    _sessions[packet->sessionID] = session;
}

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

// void Node::launch( const std::string& options )
// {
//     ASSERT( _state == STATE_STOPPED );
//     const std::string& launchCommand = _createLaunchCommand( node, options );
//     ASSERT( launchCommand );

//     Launcher::run( launchCommand );
//     _state = NODE_LAUNCHED;
// }
