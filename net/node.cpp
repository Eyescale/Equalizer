
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
#include "nodePackets.h"
#include "packet.h"
#include "pipeConnection.h"
#include "session.h"

using namespace eqNet;
using namespace std;

//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node()
        : _state(STATE_STOPPED),
          _connection(NULL),
          _sessionID(0)
{
    for( int i=0; i<CMD_NODE_CUSTOM; i++ )
        _cmdHandler[i] =  &eqNet::Node::_cmdUnknown;

    _cmdHandler[CMD_NODE_MAP_SESSION]       = &eqNet::Node::_cmdMapSession;
    _cmdHandler[CMD_NODE_MAP_SESSION_REPLY] = &eqNet::Node::_cmdMapSessionReply;
    _cmdHandler[CMD_NODE_SESSION]           = &eqNet::Node::_cmdSession;
}

bool Node::listen( Connection* connection )
{
    if( _state != STATE_STOPPED )
        return false;

    if( connection && connection->getState() != Connection::STATE_LISTENING )
        return false;

    _listenToSelf();
    if( connection )
        _connectionSet.addConnection( connection, this );
    _state = STATE_LISTENING;

    // run the receiver thread
    start();

    return true;
}

bool Node::stop()
{
    if( _state != STATE_LISTENING )
        return false;

    NodeStopPacket packet;
    send( packet );
    join();

    delete _connection;
    _connection = NULL;

    _state = STATE_STOPPED;
    return true;    
}

void Node::_listenToSelf()
{
    // setup local connection to myself
    _connection = Connection::create(TYPE_PIPE);
    ConnectionDescription connDesc;
    if( !_connection->connect( connDesc ))
    {
        _connection = NULL;
        return;
    }

    // setup connection set
    Connection* childConnection = 
        (static_cast<PipeConnection*>(_connection))->getChildEnd();

    _connectionSet.addConnection( childConnection, this );
}

bool Node::connect( Node* node, Connection* connection )
{
    if( !node || _state != STATE_LISTENING ||
        connection->getState() != Connection::STATE_CONNECTED )
        return false;

    node->_connection = connection;
    node->_state = STATE_CONNECTED;
    _connectionSet.addConnection( connection, node );
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

bool Node::mapSession( Node* server, Session* session, const char* name )
{
    NodeMapSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    packet.nameLength = strlen( name ) + 1;
    server->send( packet );
    server->send( name, packet.nameLength );

    const uint sessionID = (uint)_requestHandler.waitRequest( packet.requestID);
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
                Node* node = _connectionSet.getNode();
                INFO << node << endl;
                ASSERT( node->_connection == _connectionSet.getConnection() );
                _handleRequest( node );
                break;
            }

            case ConnectionSet::EVENT_DISCONNECT:
            {
                Node* node        = _connectionSet.getNode();
                node->_state      = STATE_STOPPED;
                node->_connection = NULL; // XXX mem leak: use ref ptr?!

                Connection* connection = _connectionSet.getConnection();
                connection->close();
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
    Connection* connection   = connectionSet.getConnection();
    Connection* newConn      = connection->accept();
    Node*       newNode      = handleNewNode( newConn );
    
    if( !newNode )
    {
        newConn->close();
        delete newConn;
    }
    else
        INFO << "New " << newNode << endl;
}

Node* Node::handleNewNode( Connection* connection )
{
    Node* node = new Node();
    if( connect( node, connection ))
        return node;

    delete node;
    return NULL;
}

void Node::_handleRequest( Node* node )
{
    INFO << "Handle request from " << node << endl;

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
    INFO << "handle " << packet << endl;
    switch( packet->datatype )
    {
        case DATATYPE_EQ_NODE:
            if( packet->command < CMD_NODE_CUSTOM )
                (this->*_cmdHandler[packet->command])(node, packet);
            else
                handleCommand( node, (const NodePacket*)packet );
            break;

        default:
            handlePacket( node, packet );
    }
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

    NodeMapSessionReplyPacket replyPacket;
    replyPacket.requestID = packet->requestID;
    replyPacket.reply     = sessionID;
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

Session* Node::_findSession( const char* name ) const
{
    for( IDHash<Session*>::const_iterator iter = _sessions.begin();
         iter != _sessions.end(); iter++ )
    {
        Session* session = (*iter).second;
        if( strcmp( session->getName(), name) == 0 )
            return session;
    }
    return NULL;
}

// void Node::launch( const char* options )
// {
//     ASSERT( _state == STATE_STOPPED );
//     const char* launchCommand = _createLaunchCommand( node, options );
//     ASSERT( launchCommand );

//     Launcher::run( launchCommand );
//     _state = NODE_LAUNCHED;
// }
