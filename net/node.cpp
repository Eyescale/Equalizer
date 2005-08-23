
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
#include "nodePackets.h"
#include "packet.h"
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

    _cmdHandler[CMD_NODE_CREATE_SESSION] = &eqNet::Node::_cmdCreateSession;
    _cmdHandler[CMD_NODE_CREATE_SESSION_REPLY] = &eqNet::Node::_cmdCreateSessionReply;
    _cmdHandler[CMD_NODE_NEW_SESSION]     = &eqNet::Node::_cmdNewSession;
}

void eqNet_Node_runServer( eqNet::Connection* connection )
{
    Node node;
    node._listen( connection, false );
}

bool Node::_listen( Connection* connection, const bool threaded )
{
    if( _state != STATE_STOPPED )
        return false;

    if( connection->getState() != Connection::STATE_CONNECTED && 
        connection->getState() != Connection::STATE_LISTENING )
        return false;

    _connectionSet.addConnection( connection, this );
    _state = STATE_LISTENING;

    if( threaded )
        start();
    else
        run();

    return true;
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

bool Node::mapSession( Session* session, const char* name )
{
    NodeMapSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    packet.nameLength = strlen( name ) + 1;
    send( packet );
    send( name, packet.nameLength );
    return (bool)_requestHandler.waitRequest( packet.requestID );
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
                _handleRequest( _connectionSet.getConnection( ),
                                _connectionSet.getNode( ));
                break;

            case ConnectionSet::EVENT_DISCONNECT:
            {
                Node* node        = _connectionSet.getNode();
                node->_state      = STATE_STOPPED;
                node->_connection = NULL; // XXX mem leak: use ref ptr?

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

void Node::_handleRequest( Connection* connection, Node* node )
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

void Node::_cmdCreateSession( Node* node, const Packet* pkg )
{
    ASSERT( getState() == STATE_LISTENING );

    NodeCreateSessionPacket* packet  = (NodeCreateSessionPacket*)pkg;
    INFO << "Cmd create session: " << packet << endl;

    const uint sessionID = _sessionID++;
    Session* session = new Session( sessionID, this );
    _sessions[sessionID] = session;

    _packSession( node, session );

    NodeCreateSessionReplyPacket replyPacket;
    replyPacket.requestID = packet->requestID;
    replyPacket.reply = sessionID;
    node->send(replyPacket);
}

void Node::_cmdCreateSessionReply( Node* node, const Packet* pkg)
{
    NodeCreateSessionReplyPacket* packet  = (NodeCreateSessionReplyPacket*)pkg;
    INFO << "Cmd create session reply: " << packet << endl;
    Session* session = _sessions[packet->reply];
    _requestHandler.serveRequest( packet->requestID, session );
}

void Node::_cmdNewSession( Node* node, const Packet* packet )
{
}

void Node::_packSession( Node* node, const Session* session )
{
    NodeNewSessionPacket packet;
    packet.sessionID = session->getID();

    node->send( packet );
    session->pack( node );
}

// void Node::launch( const char* options )
// {
//     ASSERT( _state == STATE_STOPPED );
//     const char* launchCommand = _createLaunchCommand( node, options );
//     ASSERT( launchCommand );

//     Launcher::run( launchCommand );
//     _state = NODE_LAUNCHED;
// }
