
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "connectionSet.h"
#include "global.h"
//#include "sessionPriv.h"

using namespace eqNet;
using namespace std;

Node::Node()
        : _state(STATE_STOPPED),
          _connection(NULL),
          _sessionID(0)
{
    for( int i=0; i<CMD_NODE_CUSTOM; i++ )
        _cmdHandler[i] =  &eqNet::Node::_cmdUnknown;

    _cmdHandler[CMD_SESSION_CREATE]  = &eqNet::Node::_cmdSessionCreate;
    _cmdHandler[CMD_SESSION_CREATED] = &eqNet::Node::_cmdSessionCreated;
    _cmdHandler[CMD_SESSION_NEW]     = &eqNet::Node::_cmdSessionNew;
}

bool Node::listen( Connection* connection )
{
    if( _state != STATE_STOPPED )
        return false;

    if( connection->getState() != Connection::STATE_CONNECTED && 
        connection->getState() != Connection::STATE_LISTENING )
        return false;

    _connectionSet.addConnection( connection, this );
    _state = STATE_LISTENING;
    start();
    return true;
}

Node* Node::connect( Connection* connection )
{
    if( _state != STATE_LISTENING || 
        connection->getState() != Connection::STATE_CONNECTED )
        return NULL;

    Node* node = new Node();

    node->_connection = connection;
    node->_state = STATE_CONNECTED;
    _connectionSet.addConnection( connection, node );
    return node;
}

ssize_t Node::run()
{
    INFO << "Receiver thread started" << endl;

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
    Connection* connection = connectionSet.getConnection();
    Connection* newConn    = connection->accept();
    Node*       newNode    = connect( newConn );

    ASSERT( newNode );
    INFO << "New " << newNode << endl;
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

void Node::_cmdSessionCreate( Node* node, const Packet* packet )
{
    //Session* session = new Session();
}

void Node::_cmdSessionCreated( Node* node, const Packet* packet)
{
}

void Node::_cmdSessionNew( Node* node, const Packet* packet )
{
}

// void Node::launch( const char* options )
// {
//     ASSERT( _state == STATE_STOPPED );
//     const char* launchCommand = _createLaunchCommand( node, options );
//     ASSERT( launchCommand );

//     Launcher::run( launchCommand );
//     _state = NODE_LAUNCHED;
// }

//----------------------------------------------------------------------
// Session API
//----------------------------------------------------------------------
Session* Node::createSession()
{
    SessionCreatePacket packet;
    packet.requestID = _requestHandler.registerRequest();

    send( packet );
    return (Session*)_requestHandler.waitRequest(packet.requestID);
}

//----------------------------------------------------------------------
// Messaging API
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
