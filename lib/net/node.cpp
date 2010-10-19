
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "node.h"

#include "command.h"
#include "connectionDescription.h"
#include "connectionSet.h"
#include "global.h"
#include "nodePackets.h"
#include "pipeConnection.h"
#include "session.h"
#include "socketConnection.h"

#include <eq/base/base.h>
#include <eq/base/rng.h>

#include <errno.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace eq
{
namespace net
{
typedef CommandFunc<Node> NodeFunc;

Node::Node()
        : _id( true )
        , _state( STATE_CLOSED )
        , _hasSendToken( true )
{
    _receiverThread = new ReceiverThread( this );
    _commandThread  = new CommandThread( this );

    EQINFO << "New Node @" << (void*)this << " " << _id << std::endl;
}

Node::~Node()
{
    EQINFO << "Delete Node @" << (void*)this << " " << _id << std::endl;
    EQASSERT( _outgoing == 0 );
    EQASSERT( _incoming.isEmpty( ));
    EQASSERT( _connectionNodes.empty( ));
    EQASSERT( _pendingCommands.empty( ));
    EQASSERT( _nodes->empty( ));
    EQASSERT( !hasPendingRequests( ));

#ifndef NDEBUG
    if( !_sessions->empty( ))
    {
        EQINFO << _sessions->size() << " mapped sessions" << std::endl;
        
        for( SessionHash::const_iterator i = _sessions->begin();
             i != _sessions->end(); ++i )
        {
            const Session* session = i->second;
            EQINFO << "    Session " << session->getID() << std::endl;
        }
    }
#endif

    EQASSERT( _sessions->empty( ));

    EQASSERT( !_commandThread->isRunning( ));
    delete _commandThread;
    _commandThread = 0;

    EQASSERT( !_receiverThread->isRunning( ));
    delete _receiverThread;
    _receiverThread = 0;

    _connectionDescriptions.clear();
}

bool Node::operator == ( const Node* node ) const
{ 
    EQASSERTINFO( _id != node->_id || this == node,
                  "Two node instances with the same ID found "
                  << (void*)this << " and " << (void*)node );

    return ( this == node );
}

const ConnectionDescriptions& Node::getConnectionDescriptions() const 
{
    return _connectionDescriptions;
}

ConnectionPtr Node::getMulticast()
{
    EQASSERT( isConnected( ));

    if( !isConnected( ))
        return 0;
    
    ConnectionPtr connection = _outMulticast.data;
    if( connection.isValid() && !connection->isClosed( ))
        return connection;

    base::ScopedMutex<> mutex( _outMulticast );
    if( _multicasts.empty( ))
        return 0;

    MCData data = _multicasts.back();
    _multicasts.pop_back();
    NodePtr node = data.node;

    // prime multicast connections on peers
    EQINFO << "Announcing id " << node->getNodeID() << " to multicast group "
           << data.connection->getDescription() << std::endl;

    NodeIDPacket packet;
    packet.id = node->getNodeID();
    packet.nodeType = getType();

    data.connection->send( packet, node->serialize( ));
    _outMulticast.data = data.connection;
    return data.connection;
}

bool Node::initLocal( const int argc, char** argv )
{
#ifndef NDEBUG
    EQVERB << base::disableFlush << "args: ";
    for( int i=0; i<argc; i++ )
         EQVERB << argv[i] << ", ";
    EQVERB << std::endl << base::enableFlush;
#endif

    // We do not use getopt_long because it really does not work due to the
    // following aspects:
    // - reordering of arguments
    // - different behaviour of GNU and BSD implementations
    // - incomplete man pages
    for( int i=1; i<argc; ++i )
    {
        if( std::string( "--eq-listen" ) == argv[i] )
        {
            if( i<argc && argv[i+1][0] != '-' )
            {
                std::string data = argv[++i];
                ConnectionDescriptionPtr desc = new ConnectionDescription;
                desc->port = Global::getDefaultPort();

                if( desc->fromString( data ))
                {
                    addConnectionDescription( desc );
                    EQASSERTINFO( data.empty(), data );
                }
                else
                    EQWARN << "Ignoring listen option: " << argv[i]
                           << std::endl;
            }
        }
    }
    
    if( !listen( ))
    {
        EQWARN << "Can't setup listener(s) on " << *this << std::endl; 
        return false;
    }
    
    return true;
}

bool Node::listen()
{
    EQVERB << "Listener data: " << serialize() << std::endl;
    if( !isClosed() || !_connectSelf( ))
        return false;

    for( ConnectionDescriptions::const_iterator i =
             _connectionDescriptions.begin();
         i != _connectionDescriptions.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        ConnectionPtr            connection = Connection::create( description );

        if( !connection->listen( ))
        {
            EQWARN << "Can't create listener connection: " << description
                   << std::endl;
            return false;
        }

        _connectionNodes[ connection ] = this;
        _incoming.addConnection( connection );
        if( description->type >= CONNECTIONTYPE_MULTICAST )
        {
            MCData data;
            data.connection = connection;
            data.node = this;
            _multicasts.push_back( data );
        }

        connection->acceptNB();

        EQVERB << "Added node " << _id << " using " << connection << std::endl;
    }

    _state = STATE_LISTENING;

    CommandQueue* queue = &_commandThreadQueue;
    registerCommand( CMD_NODE_STOP, NodeFunc( this, &Node::_cmdStop ), queue );
    registerCommand( CMD_NODE_REGISTER_SESSION,
                     NodeFunc( this, &Node::_cmdRegisterSession ), 0 );
    registerCommand( CMD_NODE_MAP_SESSION,
                     NodeFunc( this, &Node::_cmdMapSession ), queue );
    registerCommand( CMD_NODE_MAP_SESSION_REPLY,
                     NodeFunc( this, &Node::_cmdMapSessionReply ), 0 );
    registerCommand( CMD_NODE_UNMAP_SESSION, 
                     NodeFunc( this, &Node::_cmdUnmapSession ), queue );
    registerCommand( CMD_NODE_UNMAP_SESSION_REPLY,
                     NodeFunc( this, &Node::_cmdUnmapSessionReply ), 0 );
    registerCommand( CMD_NODE_CONNECT, NodeFunc( this, &Node::_cmdConnect ), 0);
    registerCommand( CMD_NODE_CONNECT_REPLY,
                     NodeFunc( this, &Node::_cmdConnectReply ), 0 );
    registerCommand( CMD_NODE_CONNECT_ACK, 
                     NodeFunc( this, &Node::_cmdConnectAck ), 0 );
    registerCommand( CMD_NODE_ID, NodeFunc( this, &Node::_cmdID ), 0 );
    registerCommand( CMD_NODE_DISCONNECT,
                     NodeFunc( this, &Node::_cmdDisconnect ), 0 );
    registerCommand( CMD_NODE_GET_NODE_DATA,
                     NodeFunc( this, &Node::_cmdGetNodeData), queue );
    registerCommand( CMD_NODE_GET_NODE_DATA_REPLY,
                     NodeFunc( this, &Node::_cmdGetNodeDataReply ), 0 );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN,
                     NodeFunc( this, &Node::_cmdAcquireSendToken ), queue );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY,
                     NodeFunc( this, &Node::_cmdAcquireSendTokenReply ) , 0 );
    registerCommand( CMD_NODE_RELEASE_SEND_TOKEN,
                     NodeFunc( this, &Node::_cmdReleaseSendToken ), queue );

    EQVERB << base::className( this ) << " start command and receiver thread "
           << std::endl;
    _receiverThread->start();

    EQINFO << this << " listening." << std::endl;
    return true;
}

bool Node::close()
{
    if( _state != STATE_LISTENING )
        return false;

    NodeStopPacket packet;
    send( packet );

    EQCHECK( _receiverThread->join( ));
    _cleanup();

    EQINFO << _incoming.getSize() << " connections open after close"
           << std::endl;
#ifndef NDEBUG
    const Connections& connections = _incoming.getConnections();
    for( Connections::const_iterator i = connections.begin();
         i != connections.end(); ++i )
    {
        EQINFO << "    " << *i << std::endl;
    }
#endif

    EQASSERTINFO( !hasPendingRequests(),
                  *static_cast< base::RequestHandler* >( this ));
    return true;
}

void Node::_addConnection( ConnectionPtr connection )
{
    _incoming.addConnection( connection );
    connection->recvNB( new uint64_t, sizeof( uint64_t ));
}

void Node::_removeConnection( ConnectionPtr connection )
{
    EQASSERT( connection.isValid( ));

    _incoming.removeConnection( connection );

    void* buffer( 0 );
    if( !connection->isListening( ))
    {
        uint64_t bytes( 0 );
        connection->getRecvData( &buffer, &bytes );
        EQASSERT( buffer );
        EQASSERT( bytes == sizeof( uint64_t ));
    }

    if( !connection->isClosed( ))
        connection->close(); // cancels pending IO's
    delete reinterpret_cast< uint64_t* >( buffer );
}

void Node::_cleanup()
{
    EQVERB << "Clean up stopped node" << std::endl;
    EQASSERTINFO( _state == STATE_CLOSED, _state );

    _multicasts.clear();
    _removeConnection( _outgoing );
    _connectionNodes.erase( _outgoing );
    _outMulticast.data = 0;
    _outgoing = 0;

    const Connections& connections = _incoming.getConnections();
    while( !connections.empty( ))
    {
        ConnectionPtr connection = connections.back();
        NodePtr       node       = _connectionNodes[ connection ];

        if( node.isValid( ))
        {
            node->_state = STATE_CLOSED;
            node->_outMulticast.data = 0;
            node->_outgoing = 0;
            node->_multicasts.clear();
        }

        _connectionNodes.erase( connection );
        if( node.isValid( ))
            _nodes->erase( node->_id );
        _removeConnection( connection );
    }

    if( !_connectionNodes.empty( ))
        EQINFO << _connectionNodes.size() << " open connections during cleanup"
               << std::endl;
#ifndef NDEBUG
    for( ConnectionNodeHash::const_iterator i = _connectionNodes.begin();
         i != _connectionNodes.end(); ++i )
    {
        NodePtr node = i->second;
        EQINFO << "    " << i->first << " : " << node << std::endl;
        EQINFO << "    Node ref count " << node->getRefCount() - 1 
               << ' ' << node->_outgoing << ' ' << node->_state
               << ( node == this ? " self" : "" ) << std::endl;
    }
#endif

    _connectionNodes.clear();

    if( !_nodes->empty( ))
        EQINFO << _nodes->size() << " nodes connected during cleanup"
               << std::endl;

#ifndef NDEBUG
    for( NodeHash::const_iterator i = _nodes->begin(); i != _nodes->end(); ++i )
    {
        NodePtr node = i->second;
        EQINFO << "    " << node << " ref count " << node->getRefCount() - 1 
               << ' ' << node->_outgoing << ' ' << node->_state
               << ( node == this ? " self" : "" ) << std::endl;
    }
#endif

    _nodes->clear();
}

bool Node::_connectSelf()
{
    // setup local connection to myself
    _outgoing = new PipeConnection;
    if( !_outgoing->connect( ))
    {
        EQERROR << "Could not create local connection to receiver thread."
                << std::endl;
        _outgoing = 0;
        return false;
    }

    // add to connection set
    EQASSERT( _outgoing->getDescription().isValid( )); 
    EQASSERT( _connectionNodes.find( _outgoing ) == _connectionNodes.end( ));

    _connectionNodes[ _outgoing ] = this;
    _nodes.data[ _id ] = this;
    _addConnection( _outgoing );

    EQVERB << "Added node " << _id << " using " << _outgoing << std::endl;
    return true;
}

void Node::_connectMulticast( NodePtr node )
{
    EQASSERT( inReceiverThread( ));
    base::ScopedMutex<> mutex( _outMulticast );

    if( node->_outMulticast.data.isValid( ))
        // multicast already connected by previous _cmdID
        return;

    // Search if the connected node is in the same multicast group as we are
    for( ConnectionDescriptions::const_iterator i =
             _connectionDescriptions.begin();
         i != _connectionDescriptions.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        if( description->type < CONNECTIONTYPE_MULTICAST )
            continue;

        const ConnectionDescriptions& fromDescs =
            node->getConnectionDescriptions();
        for( ConnectionDescriptions::const_iterator j = fromDescs.begin();
             j != fromDescs.end(); ++j )
        {
            ConnectionDescriptionPtr fromDescription = *j;
            if( !description->isSameMulticastGroup( fromDescription ))
                continue;
            
            EQASSERT( !node->_outMulticast.data );
            EQASSERT( node->_multicasts.empty( ));

            if( _outMulticast->isValid() && 
                _outMulticast.data->getDescription() == description )
            {
                node->_outMulticast.data = _outMulticast.data;
                EQINFO << "Using " << description << " as multicast group for "
                       << node->getNodeID() << std::endl;
            }
            // find unused multicast connection to node
            else for( MCDatas::const_iterator k = _multicasts.begin();
                      k != _multicasts.end(); ++k )
            {
                const MCData& data = *k;
                ConnectionDescriptionPtr dataDesc = 
                    data.connection->getDescription();
                if( !description->isSameMulticastGroup( dataDesc ))
                    continue;

                node->_multicasts.push_back( data );
                EQINFO << "Adding " << dataDesc << " as multicast group for "
                       << node->getNodeID() << std::endl;
            }
        }
    }
}

NodePtr Node::getNode( const NodeID& id ) const
{ 
    base::ScopedMutex< base::SpinLock > mutex( _nodes );
    NodeHash::const_iterator i = _nodes->find( id );
    if( i == _nodes->end( ))
        return 0;
    return i->second;
}

bool Node::disconnect( NodePtr node )
{
    if( !node || _state != STATE_LISTENING )
        return false;

    if( node->_state != STATE_CONNECTED )
        return true;

    EQASSERT( !inCommandThread( ));

    NodeDisconnectPacket packet;
    packet.requestID = registerRequest( node.get( ));
    send( packet );

    waitRequest( packet.requestID );
    return true;
}

void Node::addConnectionDescription( ConnectionDescriptionPtr cd )
{
    if( cd->type >= CONNECTIONTYPE_MULTICAST && cd->port == 0 )
        cd->port = EQ_DEFAULT_PORT;

    _connectionDescriptions.push_back( cd ); 
}

bool Node::removeConnectionDescription( ConnectionDescriptionPtr cd )
{
    ConnectionDescriptions::iterator i = stde::find( _connectionDescriptions,
                                                     cd );
    if( i == _connectionDescriptions.end( ))
        return false;

    _connectionDescriptions.erase( i );
    return true;
}

//----------------------------------------------------------------------
// Node functionality
//----------------------------------------------------------------------
void Node::_addSession( Session* session, NodePtr server,
                        const SessionID& sessionID )
{
    EQ_TS_THREAD( _recvThread );

    session->_server    = server;
    session->_id        = sessionID;
    session->_isMaster  = ( server==this && isLocal( ));
    session->_setLocalNode( this );
    if( session->_isMaster )
        session->_idPool.freeIDs( 1, base::IDPool::MAX_CAPACITY );

    base::ScopedMutex< base::SpinLock > mutex( _sessions );
    EQASSERTINFO( _sessions->find( sessionID ) == _sessions->end(),
                  "Session " << sessionID << " @" << (void*)session
                  << " already mapped on " << this << " @"
                  <<  (void*)_sessions.data[ sessionID ] );
    _sessions.data[ sessionID ] = session;

    EQINFO << (session->_isMaster ? "master" : "client") << " session, id "
           << sessionID << ", served by " << *server << ", mapped on "
           << *this << std::endl;
}

void Node::_removeSession( Session* session )
{
    EQ_TS_THREAD( _recvThread );
    {
        base::ScopedMutex< base::SpinLock > mutex( _sessions );
        EQASSERT( _sessions->find( session->getID( )) != _sessions->end( ));

        _sessions->erase( session->getID( ));
        EQINFO << "Erased session " << session->getID() << ", " 
               << _sessions->size() << " left" << std::endl;
    }

    session->_setLocalNode( 0 );
    session->_server    = 0;
    if( session->_isMaster )
        session->_isMaster  = false;
    else
        // generate a new id for a slave session so it can become a master
        session->_id = SessionID( true );
}

void Node::registerSession( Session* session )
{
    EQASSERT( isLocal( ));
    EQASSERT( !inCommandThread( ));

    NodeRegisterSessionPacket packet;
    packet.requestID = registerRequest( session );
    send( packet );

    waitRequest( packet.requestID );
}

bool Node::mapSession( NodePtr server, Session* session, const SessionID& id )
{
    EQASSERT( isLocal( ));
    EQASSERT( id != SessionID::ZERO );
    EQASSERT( server != this );

    NodeMapSessionPacket packet;
    packet.requestID = registerRequest( session );
    packet.sessionID = id;
    server->send( packet );

    waitRequest( packet.requestID );
    return ( session->getID() != SessionID::ZERO );
}

bool Node::unmapSession( Session* session )
{
    EQASSERT( isLocal( ));

    NodeUnmapSessionPacket packet;
    packet.requestID = registerRequest( session );
    packet.sessionID = session->getID();
    session->getServer()->send( packet );

    bool ret = false;
    waitRequest( packet.requestID, ret );
    return ret;
}

Session* Node::getSession( const SessionID& id )
{
    base::ScopedMutex< base::SpinLock > mutex( _sessions );
    SessionHash::const_iterator i = _sessions->find( id );
    
    EQASSERT( i != _sessions->end( ));
    if( i == _sessions->end( ))
        return 0;

    return i->second;
}

std::string Node::serialize() const
{
    std::ostringstream data;
    data << _id << EQNET_SEPARATOR << _connectionDescriptions.size()
         << EQNET_SEPARATOR;

    for( ConnectionDescriptions::const_iterator i =
             _connectionDescriptions.begin();
         i != _connectionDescriptions.end(); ++i )
    {
        ConnectionDescriptionPtr desc = *i;
        desc->serialize( data );
    }
    
    return data.str();
}
 
bool Node::deserialize( std::string& data )
{
    EQASSERT( _state == STATE_CLOSED );

    EQVERB << "Node data: " << data << std::endl;
    if( !_connectionDescriptions.empty( ))
        EQWARN << "Node already holds data while deserializing it" << std::endl;

    // node id
    size_t nextPos = data.find( EQNET_SEPARATOR );
    if( nextPos == std::string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse node data" << std::endl;
        return false;
    }

    _id = data.substr( 0, nextPos );
    data = data.substr( nextPos + 1 );

    // num connection descriptions
    nextPos = data.find( EQNET_SEPARATOR );
    if( nextPos == std::string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse node data" << std::endl;
        return false;
    }

    const std::string sizeStr = data.substr( 0, nextPos );
    if( !isdigit( sizeStr[0] ))
    {
        EQERROR << "Could not parse node data" << std::endl;
        return false;
    }

    const size_t nDesc = atoi( sizeStr.c_str( ));
    data = data.substr( nextPos + 1 );

    // connection descriptions
    for( size_t i = 0; i<nDesc; ++i )
    {
        ConnectionDescriptionPtr desc = new ConnectionDescription;
        if( !desc->fromString( data ))
        {
            EQERROR << "Error during node connection data parsing" << std::endl;
            return false;
        }
        addConnectionDescription( desc );
    }

    return true;
}

NodePtr Node::createNode( const uint32_t type )
{
    EQASSERTINFO( type == NODETYPE_EQNET_NODE, type );
    return new Node();
}

void Node::acquireSendToken( NodePtr node )
{
    EQASSERT( !inCommandThread( ));
    EQASSERT( !inReceiverThread( ));

    NodeAcquireSendTokenPacket packet;
    packet.requestID = registerRequest();
    node->send( packet );
    waitRequest( packet.requestID );
}

void Node::releaseSendToken( NodePtr node )
{
    EQASSERT( !inReceiverThread( ));

    NodeReleaseSendTokenPacket packet;
    node->send( packet );
}

//----------------------------------------------------------------------
// Connecting a node
//----------------------------------------------------------------------
bool Node::connect( NodePtr node )
{
    EQASSERTINFO( _state == STATE_LISTENING, _state );
    if( node->_state == STATE_CONNECTED || node->_state == STATE_LISTENING )
        return true;

    EQASSERT( node->_state == STATE_CLOSED );

    // try connecting using the given descriptions
    const ConnectionDescriptions& cds = node->getConnectionDescriptions();
    EQASSERTINFO( !cds.empty(),
                  "Can't connect to a node without connection descriptions" );

    for( ConnectionDescriptions::const_iterator i = cds.begin();
        i != cds.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        if( description->type >= CONNECTIONTYPE_MULTICAST )
            continue; // Don't use multicast for primary connections

        ConnectionPtr connection = Connection::create( description );

        if( !connection || !connection->connect( ))
            continue;

        return _connect( node, connection );
    }
    return false;
}

bool Node::_connect( NodePtr node, ConnectionPtr connection )
{
    EQASSERT( connection.isValid( ));

    if( !node.isValid() || _state != STATE_LISTENING ||
        !connection->isConnected() || node->_state != STATE_CLOSED )
    {
        return false;
    }

    _addConnection( connection );

    // send connect packet to peer
    NodeConnectPacket packet;
    packet.requestID = registerRequest( node.get( ));
    packet.nodeID    = _id;
    packet.nodeType  = getType();
    connection->send( packet, serialize( ));

    bool connected = false;
    waitRequest( packet.requestID, connected );
    if( !connected )
        return false;

    EQASSERT( node->_id != NodeID::ZERO );
    EQASSERTINFO( node->_id != _id, _id );
    EQINFO << node << " connected to " << *this << std::endl;
    return true;
}

NodePtr Node::connect( const NodeID& nodeID )
{
    EQASSERT( nodeID != NodeID::ZERO );
    EQASSERT( _state == STATE_LISTENING );

    // Extract all node pointers, the _nodes hash will be modified later
    Nodes nodes;
    {
        base::ScopedMutex< base::SpinLock > mutex( _nodes );
        for( NodeHash::const_iterator i = _nodes->begin();
             i != _nodes->end(); ++i )
        {
            NodePtr node = i->second;

            if( node->getNodeID() == nodeID && node->isConnected( )) //early out
                return node;

            nodes.push_back( node );
        }
    }

    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        NodePtr node = _connect( nodeID, *i );
        if( node.isValid( ))
            return node;
    }
        
    EQWARN << "Node connection failed" << std::endl;
    return 0;
}

NodePtr Node::_connect( const NodeID& nodeID, NodePtr server )
{
    EQASSERT( nodeID != NodeID::ZERO );

    NodePtr node;

    // Make sure that only one connection request based on the node identifier
    // is pending at a given time. Otherwise a node with the same id might be
    // instantiated twice in _cmdGetNodeDataReply(). The alternative to this
    // mutex is to register connecting nodes with this local node, and handle
    // all cases correctly, which is far more complex. Node connections only
    // happen a lot during initialization, and are therefore not time-critical.
    base::ScopedMutex<> mutex( _connectMutex );
    {
        base::ScopedMutex< base::SpinLock > mutexNodes( _nodes ); 
        NodeHash::const_iterator i = _nodes->find( nodeID );
        if( i != _nodes->end( ))
            node = i->second;
    }

    if( node.isValid( ))
    {
        if( !node->isConnected( ))
            connect( node );
        return node->isConnected() ? node : 0;        
    }

    EQINFO << "Connecting node " << nodeID << std::endl;
    EQASSERT( _id != nodeID );

    NodeGetNodeDataPacket packet;
    packet.requestID = registerRequest();
    packet.nodeID    = nodeID;
    server->send( packet );

    void* result = 0;
    waitRequest( packet.requestID, result );

    if( !result )
    {
        EQWARN << "Node not found on server" << std::endl;
        return 0;
    }

    EQASSERT( dynamic_cast< Node* >( (Dispatcher*)result ));
    node = static_cast< Node* >( result );
    node.unref(); // ref'd before serveRequest()
    
    if( node->isConnected( ))
        return node;

    if( connect( node ))
        return node;

    {
        base::ScopedMutex< base::SpinLock > mutexNodes( _nodes );
        // connect failed - maybe simultaneous connect from peer?
        NodeHash::const_iterator i = _nodes->find( nodeID );
        if( i != _nodes->end( ))
        {
            node = i->second;
            if( node->isConnected( ))
                return node;
        }
    }

    connect( node );
    return node->isConnected() ? node : 0;
}

//----------------------------------------------------------------------
// receiver thread functions
//----------------------------------------------------------------------
void Node::_runReceiverThread()
{
    EQINFO << "Entered receiver thread of " << base::className( this )
           << std::endl;
    base::Thread::setDebugName( std::string( "Rcv " ) + base::className(this));

    int nErrors = 0;
    while( _state == STATE_LISTENING )
    {
        const ConnectionSet::Event result = _incoming.select();
        switch( result )
        {
            case ConnectionSet::EVENT_CONNECT:
                _handleConnect();
                break;

            case ConnectionSet::EVENT_DATA:      
                _handleData();
                break;

            case ConnectionSet::EVENT_DISCONNECT:
            case ConnectionSet::EVENT_INVALID_HANDLE:
                _handleDisconnect();
                break;

            case ConnectionSet::EVENT_TIMEOUT:
                EQINFO << "select timeout" << std::endl;
                break;

            case ConnectionSet::EVENT_ERROR:
                ++nErrors;
                EQWARN << "Connection error during select" << std::endl;
                if( nErrors > 100 )
                {
                    EQWARN << "Too many errors in a row, capping connection" 
                           << std::endl;
                    _handleDisconnect();
                }
                break;

            case ConnectionSet::EVENT_SELECT_ERROR:
                EQWARN << "Error during select" << std::endl;
                ++nErrors;
                if( nErrors > 10 )
                {
                    EQWARN << "Too many errors in a row" << std::endl;
                    EQUNIMPLEMENTED;
                }
                break;

            case ConnectionSet::EVENT_INTERRUPT:
                _redispatchCommands();
                break;

            default:
                EQUNIMPLEMENTED;
        }
        if( result != ConnectionSet::EVENT_ERROR && 
            result != ConnectionSet::EVENT_SELECT_ERROR )

            nErrors = 0;
    }

    if( !_pendingCommands.empty( ))
        EQWARN << _pendingCommands.size() 
               << " commands pending while leaving command thread" << std::endl;

    for( CommandList::const_iterator i = _pendingCommands.begin();
         i != _pendingCommands.end(); ++i )
    {
        Command* command = *i;
        command->release();
    }

    EQCHECK( _commandThread->join( ));
    _pendingCommands.clear();
    _commandCache.flush();

    EQINFO << "Leaving receiver thread of " << base::className( this )
           << std::endl;
}

void Node::_handleConnect()
{
    ConnectionPtr connection = _incoming.getConnection();
    ConnectionPtr newConn = connection->acceptSync();
    connection->acceptNB();

    if( !newConn )
    {
        EQINFO << "Received connect event, but accept() failed" << std::endl;
        return;
    }
    _addConnection( newConn );
}

void Node::_handleDisconnect()
{
    while( _handleData( )) ; // read remaining data off connection

    ConnectionPtr connection = _incoming.getConnection();
    ConnectionNodeHash::iterator i = _connectionNodes.find( connection );

    if( i != _connectionNodes.end( ))
    {
        NodePtr node = i->second;
        if( node->_outgoing == connection )
        {
            _connectionNodes.erase( i );
            node->_state    = STATE_CLOSED;
            node->_outgoing = 0;

            EQINFO << node << " disconnected from " << this << std::endl;
            base::ScopedMutex< base::SpinLock > mutex( _nodes );
            _nodes->erase( node->_id );
        }
        else
        {
            EQASSERT( connection->getDescription()->type >= 
                      CONNECTIONTYPE_MULTICAST );

            base::ScopedMutex<> mutex( _outMulticast );
            if( node->_outMulticast == connection )
                node->_outMulticast = 0;
            else
            {
                for( MCDatas::iterator j = node->_multicasts.begin();
                     j != node->_multicasts.end(); ++j )
                {
                    if( (*j).connection != connection )
                        continue;

                    node->_multicasts.erase( j );
                    break;
                }
            }
        }
    }

    _removeConnection( connection );
    EQINFO << "connection used " << connection->getRefCount() << std::endl;
}

bool Node::_handleData()
{
    ConnectionPtr connection = _incoming.getConnection();
    EQASSERT( connection.isValid( ));

    NodePtr node;
    ConnectionNodeHash::const_iterator i = _connectionNodes.find( connection );
    if( i != _connectionNodes.end( ))
        node = i->second;
    EQASSERTINFO( !node || // unconnected node
                  node->_outgoing == connection || // correct UC conn for node
                  connection->getDescription()->type>=CONNECTIONTYPE_MULTICAST,
                  base::className( node ));

    EQVERB << "Handle data from " << node << std::endl;

    void* sizePtr( 0 );
    uint64_t bytes( 0 );
    const bool gotSize = connection->recvSync( &sizePtr, &bytes, false );

    if( !gotSize ) // Some systems signal data on dead connections.
    {
        connection->recvNB( sizePtr, sizeof( uint64_t ));
        return false;
    }

    EQASSERT( sizePtr );
    const uint64_t size = *reinterpret_cast< uint64_t* >( sizePtr );
    if( bytes == 0 ) // fluke signal
    {
        EQWARN << "Erronous network event on " << connection->getDescription()
               << std::endl;
        _incoming.setDirty();
        return false;
    }

    EQASSERT( size );
    EQASSERTINFO( bytes == sizeof( uint64_t ), bytes );
    EQASSERT( size > sizeof( size ));

    Command& command = _commandCache.alloc( node, this, size );
    uint8_t* ptr = reinterpret_cast< uint8_t* >( command.getPacket()) +
                                                 sizeof( uint64_t );

    connection->recvNB( ptr, size - sizeof( uint64_t ));
    const bool gotData = connection->recvSync( 0, 0 );    

    EQASSERT( gotData );
    EQASSERT( command.isValid( ));

    // start next receive
    connection->recvNB( sizePtr, sizeof( uint64_t ));

    if( !gotData )
    {
        EQERROR << "Incomplete packet read: " << command << std::endl;
        return false;
    }

    // This is one of the initial packets during the connection handshake, at
    // this point the remote node is not yet available.
    EQASSERTINFO( node.isValid() ||
                 ( command->type == PACKETTYPE_EQNET_NODE &&
                  ( command->command == CMD_NODE_CONNECT  || 
                    command->command == CMD_NODE_CONNECT_REPLY ||
                    command->command == CMD_NODE_ID )),
                  command << " connection " << connection );

    _dispatchCommand( command );
    return true;
}

void Node::_dispatchCommand( Command& command )
{
    EQASSERT( command.isValid( ));

    const bool dispatched = dispatchCommand( command );
 
    _redispatchCommands();
  
    if( !dispatched )
    {
        command.retain();
        _pendingCommands.push_back( &command );
    }
}

bool Node::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << " by " << _id << std::endl;
    EQASSERT( command.isValid( ));

    const uint32_t type = command->type;
    switch( type )
    {
        case PACKETTYPE_EQNET_NODE:
            EQCHECK( Dispatcher::dispatchCommand( command ));
            return true;

        case PACKETTYPE_EQNET_SESSION:
        case PACKETTYPE_EQNET_OBJECT:
        {
            const SessionPacket* sessionPacket = 
                static_cast<SessionPacket*>( command.getPacket( ));
            const SessionID& id = sessionPacket->sessionID;
            SessionHash::const_iterator i = _sessions->find( id );
            EQASSERTINFO( i != _sessions->end(),
                          "Can't find session for " << command );

            Session* session = i->second;            
            return session->dispatchCommand( command );
        }

        default:
            EQABORT( "Unknown packet type " << type << " for " << command );
            return true;
    }
}

void Node::_redispatchCommands()
{
    bool changes = true;
    while( changes && !_pendingCommands.empty( ))
    {
        changes = false;

        for( CommandList::iterator i = _pendingCommands.begin();
             i != _pendingCommands.end(); ++i )
        {
            Command* command = *i;
            EQASSERT( command->isValid( ));

            if( dispatchCommand( *command ))
            {
                _pendingCommands.erase( i );
                command->release();
                changes = true;
                break;
            }
        }
    }

#ifndef NDEBUG
    if( !_pendingCommands.empty( ))
        EQVERB << _pendingCommands.size() << " undispatched commands" 
               << std::endl;
    EQASSERT( _pendingCommands.size() < 200 );
#endif
}

//----------------------------------------------------------------------
// command thread functions
//----------------------------------------------------------------------
void Node::_runCommandThread()
{
    EQINFO << "Entered command thread of " << base::className( this )
           << std::endl;
    base::Thread::setDebugName( std::string( "Cmd " ) + base::className(this));

    while( _state == STATE_LISTENING )
    {
        Command* command = _commandThreadQueue.pop();
        EQASSERT( command->isValid( ));

        if( !invokeCommand( *command ))
        {
            EQABORT( "Error handling " << *command );
        }
        command->release();
    }
 
    _commandThreadQueue.flush();
    EQINFO << "Leaving command thread of " << base::className( this )
           << std::endl;
}

bool Node::invokeCommand( Command& command )
{
    EQVERB << "dispatch " << command << " by " << _id << std::endl;
    EQASSERT( command.isValid( ));

    const uint32_t type = command->type;
    switch( type )
    {
        case PACKETTYPE_EQNET_NODE:
            return Dispatcher::invokeCommand( command );

        case PACKETTYPE_EQNET_SESSION:
        case PACKETTYPE_EQNET_OBJECT:
        {
            const SessionPacket* sessionPacket = 
                static_cast<SessionPacket*>( command.getPacket( ));
            const SessionID& id = sessionPacket->sessionID;
            Session* session = 0;            
            {
                base::ScopedMutex< base::SpinLock > mutex( _sessions );
                SessionHash::const_iterator i = _sessions->find( id );
                EQASSERTINFO( i != _sessions->end(),
                              "Can't find session for " << sessionPacket );

                session = i->second;
            } 
            if( !session )
                return false;

            return session->invokeCommand( command );
        }

        default:
            EQABORT( "Unknown packet type " << type << " for " << command );
            return false;
    }
}

bool Node::_cmdStop( Command& )
{
    EQINFO << "Cmd stop " << this << std::endl;
    EQASSERT( _state == STATE_LISTENING );

    _state = STATE_CLOSED;
    _incoming.interrupt();

    return true;
}

bool Node::_cmdRegisterSession( Command& command )
{
    EQ_TS_THREAD( _recvThread );
    EQASSERT( _state == STATE_LISTENING );
    EQASSERTINFO( command.getNode() == this, 
                  command.getNode() << " != " << this );

    const NodeRegisterSessionPacket* packet = 
        command.getPacket< NodeRegisterSessionPacket >();
    EQVERB << "Cmd register session: " << packet << std::endl;
    
    Session* session = static_cast< Session* >( 
        getRequestData( packet->requestID ));
    EQASSERT( session );

    _addSession( session, this, session->getID( ));
    serveRequest( packet->requestID );
    return true;
}


bool Node::_cmdMapSession( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    EQASSERT( _state == STATE_LISTENING );

    const NodeMapSessionPacket* packet = 
        command.getPacket<NodeMapSessionPacket>();
    EQVERB << "Cmd map session: " << packet << std::endl;
    
    NodePtr node = command.getNode();
    NodeMapSessionReplyPacket reply( packet );

    if( node == this )
    {
        EQASSERTINFO( node == this, 
                      "Can't map a session using myself as server " );
        reply.sessionID = SessionID::ZERO;
    }
    else
    {
        const SessionID& sessionID = packet->sessionID;
        base::ScopedMutex< base::SpinLock > mutex( _sessions );
        SessionHash::const_iterator i = _sessions->find( sessionID );
        
        if( i == _sessions->end( ))
        {
            EQWARN << "Can't find session " << sessionID << " to map"
                   << std::endl;
            reply.sessionID = SessionID::ZERO;
        }
    }

    node->send( reply );
    return true;
}

bool Node::_cmdMapSessionReply( Command& command)
{
    EQ_TS_THREAD( _recvThread );
    const NodeMapSessionReplyPacket* packet = 
        command.getPacket<NodeMapSessionReplyPacket>();
    EQVERB << "Cmd map session reply: " << packet << std::endl;

    const uint32_t requestID = packet->requestID;
    if( packet->sessionID != SessionID::ZERO )
    {
        NodePtr  node    = command.getNode(); 
        Session* session = static_cast< Session* >( 
            getRequestData( requestID ));
        EQASSERT( session );
        EQASSERT( node != this );

        _addSession( session, node, packet->sessionID );
    }

    serveRequest( requestID );
    return true;
}

bool Node::_cmdUnmapSession( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    const NodeUnmapSessionPacket* packet =
        command.getPacket<NodeUnmapSessionPacket>();
    EQVERB << "Cmd unmap session: " << packet << std::endl;
    
    const SessionID& sessionID = packet->sessionID;
    NodeUnmapSessionReplyPacket reply( packet );
    {
        base::ScopedMutex< base::SpinLock > mutex( _sessions );
        SessionHash::const_iterator i = _sessions->find( sessionID );
        reply.result = (i == _sessions->end() ? false : true );
    }

#if 0
    if( session && session->_server == this )
        ;// TODO: unmap all session slave instances.
#endif

    command.getNode()->send( reply );
    return true;
}

bool Node::_cmdUnmapSessionReply( Command& command)
{
    EQ_TS_THREAD( _recvThread );
    const NodeUnmapSessionReplyPacket* packet = 
        command.getPacket<NodeUnmapSessionReplyPacket>();
    EQVERB << "Cmd unmap session reply: " << packet << std::endl;

    const uint32_t requestID = packet->requestID;
    Session* session = static_cast< Session* >(
        getRequestData( requestID ));
    EQASSERT( session );

    if( session )
    {
        _removeSession( session ); // TODO use session existence as return value
        serveRequest( requestID, true );
    }
    else
        serveRequest( requestID, false );

    // packet->result is false if server-side session was already unmapped
    return true;
}

bool Node::_cmdConnect( Command& command )
{
    EQASSERT( !command.getNode().isValid( ));
    EQASSERT( inReceiverThread( ));

    const NodeConnectPacket* packet = command.getPacket<NodeConnectPacket>();
    ConnectionPtr        connection = _incoming.getConnection();

    const NodeID& nodeID = packet->nodeID;

    EQVERB << "handle connect " << packet << std::endl;
    EQASSERT( nodeID != _id );
    EQASSERT( _connectionNodes.find( connection ) == _connectionNodes.end( ));

    NodePtr remoteNode;

    // No locking needed, only recv thread modifies
    NodeHash::const_iterator i = _nodes->find( nodeID );
    if( i != _nodes->end( ))
    {
        remoteNode = i->second;
        if( remoteNode->isConnected( ))
        {
            // Node exists, probably simultaneous connect from peer
            EQINFO << "Already got node " << nodeID << ", refusing connect"
                   << std::endl;

            // refuse connection
            NodeConnectReplyPacket reply( packet );
            connection->send( reply, serialize( ));

            // NOTE: There is no close() here. The reply packet above has to be
            // received by the peer first, before closing the connection.
            _removeConnection( connection );
            return true;
        }
    }

    // create and add connected node
    if( !remoteNode )
        remoteNode = createNode( packet->nodeType );
    else
        remoteNode->_connectionDescriptions.clear(); // use data from peer

    std::string data = packet->nodeData;
    if( !remoteNode->deserialize( data ))
        EQWARN << "Error during node initialization" << std::endl;
    EQASSERTINFO( data.empty(), data );
    EQASSERTINFO( remoteNode->_id == nodeID,
                  remoteNode->_id << "!=" << nodeID );

    remoteNode->_outgoing = connection;
    remoteNode->_state    = STATE_CONNECTED;
    
    _connectionNodes[ connection ] = remoteNode;
    {
        base::ScopedMutex< base::SpinLock > mutex( _nodes );
        _nodes.data[ remoteNode->_id ] = remoteNode;
    }
    EQVERB << "Added node " << nodeID << std::endl;

    // send our information as reply
    NodeConnectReplyPacket reply( packet );
    reply.nodeID    = _id;
    reply.nodeType  = getType();

    connection->send( reply, serialize( ));    
    return true;
}

bool Node::_cmdConnectReply( Command& command )
{
    EQASSERT( !command.getNode( ));
    EQASSERT( inReceiverThread( ));

    const NodeConnectReplyPacket* packet = 
        command.getPacket<NodeConnectReplyPacket>();
    ConnectionPtr connection = _incoming.getConnection();

    const NodeID& nodeID = packet->nodeID;

    EQVERB << "handle connect reply " << packet << std::endl;
    EQASSERT( _connectionNodes.find( connection ) == _connectionNodes.end( ));

    NodePtr remoteNode;

    // No locking needed, only recv thread modifies
    NodeHash::const_iterator i = _nodes->find( nodeID );
    if( i != _nodes->end( ))
        remoteNode = i->second;

    if( nodeID == NodeID::ZERO || // connection refused
        // Node exists, probably simultaneous connect
        ( remoteNode.isValid() && remoteNode->isConnected( )))
    {
        EQINFO << "ignoring connect reply, node already connected" << std::endl;
        _removeConnection( connection );
        
        if( packet->requestID != EQ_ID_INVALID )
            serveRequest( packet->requestID, false );
        
        return true;
    }

    // create and add node
    if( !remoteNode )
    {
        if( packet->requestID != EQ_ID_INVALID )
        {
            void* ptr = getRequestData( packet->requestID );
            EQASSERT( dynamic_cast< Node* >( (Dispatcher*)ptr ));
            remoteNode = static_cast< Node* >( ptr );
        }
        else
            remoteNode = createNode( packet->nodeType );
    }

    remoteNode->_connectionDescriptions.clear(); // use data from peer

    EQASSERT( remoteNode->getType() == packet->nodeType );
    EQASSERT( remoteNode->_state == STATE_CLOSED );

    std::string data = packet->nodeData;
    if( !remoteNode->deserialize( data ))
        EQWARN << "Error during node initialization" << std::endl;
    EQASSERT( data.empty( ));
    EQASSERT( remoteNode->_id == nodeID );

    remoteNode->_outgoing = connection;
    remoteNode->_state    = STATE_CONNECTED;
    
    _connectionNodes[ connection ] = remoteNode;
    {
        base::ScopedMutex< base::SpinLock > mutex( _nodes );
        _nodes.data[ remoteNode->_id ] = remoteNode;
    }
    EQVERB << "Added node " << nodeID << std::endl;

    if( packet->requestID != EQ_ID_INVALID )
        serveRequest( packet->requestID, true );

    NodeConnectAckPacket ack;
    remoteNode->send( ack );

    _connectMulticast( remoteNode );
    return true;
}

bool Node::_cmdConnectAck( Command& command )
{
    NodePtr node = command.getNode();
    EQASSERT( node.isValid( ));
    EQASSERT( inReceiverThread( ));
    EQVERB << "handle connect ack" << std::endl;
    
    _connectMulticast( node );
    return true;
}

bool Node::_cmdID( Command& command )
{
    EQASSERT( inReceiverThread( ));

    const NodeIDPacket* packet = command.getPacket< NodeIDPacket >();
    NodeID nodeID = packet->id;

    if( command.getNode().isValid( ))
    {
        EQASSERT( nodeID == command.getNode()->getNodeID( ));
        EQASSERT( command.getNode()->_outMulticast->isValid( ));
        return true;
    }

    EQINFO << "handle ID " << packet << " node " << nodeID << std::endl;

    ConnectionPtr connection = _incoming.getConnection();
    EQASSERT( connection->getDescription()->type >= CONNECTIONTYPE_MULTICAST );
    EQASSERT( _connectionNodes.find( connection ) == _connectionNodes.end( ));

    NodePtr node;
    if( nodeID == _id ) // 'self' multicast connection
        node = this;
    else
    {
        // No locking needed, only recv thread writes
        NodeHash::const_iterator i = _nodes->find( nodeID );

        if( i == _nodes->end( ))
        {
            // unknown node: create and add unconnected node
            node = createNode( packet->nodeType );
            std::string data = packet->data;

            if( !node->deserialize( data ))
                EQWARN << "Error during node initialization" << std::endl;
            EQASSERTINFO( data.empty(), data );

            {
                base::ScopedMutex< base::SpinLock > mutex( _nodes );
                _nodes.data[ nodeID ] = node;
            }
            EQVERB << "Added node " << nodeID << " with multicast "
                   << connection << std::endl;
        }
        else
            node = i->second;
    }
    EQASSERT( node.isValid( ));
    EQASSERTINFO( node->_id == nodeID, node->_id << "!=" << nodeID );

    base::ScopedMutex<> mutex( _outMulticast );
    MCDatas::iterator i = node->_multicasts.begin();
    for( ; i != node->_multicasts.end(); ++i )
    {
        if( (*i).connection == connection )
            break;
    }

    if( node->_outMulticast->isValid( ))
    {
        if( node->_outMulticast.data == connection ) // connection already used
        {
            // nop
            EQASSERT( i == node->_multicasts.end( ));
        }
        else // another connection is used as multicast connection, save this
        {
            if( i == node->_multicasts.end( ))
            {
                EQASSERT( _state == STATE_LISTENING );
                MCData data;
                data.connection = connection;
                data.node = this;
                node->_multicasts.push_back( data );
            }
            // else nop, already know connection
        }
    }
    else
    {
        node->_outMulticast.data = connection;
        if( i != node->_multicasts.end( ))
            node->_multicasts.erase( i );
    }

    _connectionNodes[ connection ] = node;
    EQINFO << "Added multicast connection " << connection << " from " << nodeID
           << " to " << _id<< std::endl;
    return true;
}

bool Node::_cmdDisconnect( Command& command )
{
    EQASSERT( inReceiverThread( ));

    const NodeDisconnectPacket* packet =
        command.getPacket<NodeDisconnectPacket>();

    NodePtr node = static_cast<Node*>( 
        getRequestData( packet->requestID ));
    EQASSERT( node.isValid( ));

    ConnectionPtr connection = node->_outgoing;
    if( connection.isValid( ))
    {
        node->_state    = STATE_CLOSED;
        node->_outgoing = 0;

        _removeConnection( connection );
        EQASSERT( _connectionNodes.find( connection )!=_connectionNodes.end( ));

        _connectionNodes.erase( connection );
        {
            base::ScopedMutex< base::SpinLock > mutex( _nodes );
            _nodes->erase( node->_id );
        }

        EQINFO << node << " disconnected from " << this << " connection used " 
               << connection->getRefCount() << std::endl;
    }

    EQASSERT( node->_state == STATE_CLOSED );
    serveRequest( packet->requestID );
    return true;
}

bool Node::_cmdGetNodeData( Command& command)
{
    const NodeGetNodeDataPacket* packet = 
        command.getPacket<NodeGetNodeDataPacket>();
    EQVERB << "cmd get node data: " << packet << std::endl;

    const NodeID& nodeID = packet->nodeID;
    NodePtr node = getNode( nodeID );
    NodeGetNodeDataReplyPacket reply( packet );

    std::string nodeData;
    if( node.isValid( ))
    {
        reply.nodeType = node->getType();
        nodeData = node->serialize();
    }
    else
    {
        EQVERB << "Node " << nodeID << " unknown" << std::endl;
        reply.nodeType = NODETYPE_EQNET_INVALID;
    }

    NodePtr toNode = command.getNode();
    toNode->send( reply, nodeData );
    EQINFO << "Sent node data " << nodeData << " to " << toNode << std::endl;
    return true;
}

bool Node::_cmdGetNodeDataReply( Command& command )
{
    EQASSERT( inReceiverThread( ));

    NodeGetNodeDataReplyPacket* packet = 
        command.getPacket<NodeGetNodeDataReplyPacket>();
    EQVERB << "cmd get node data reply: " << packet << std::endl;

    const uint32_t requestID = packet->requestID;
    const NodeID& nodeID = packet->nodeID;

    // No locking needed, only recv thread writes
    NodeHash::const_iterator i = _nodes->find( nodeID );
    if( i != _nodes->end( ))
    {
        // Requested node connected to us in the meantime
        NodePtr node = i->second;
        
        node.ref();
        serveRequest( requestID, node.get( ));
        return true;
    }

    if( packet->nodeType == NODETYPE_EQNET_INVALID )
    {
        serveRequest( requestID, (void*)0 );
        return true;
    }

    // new node: create and add unconnected node        
    NodePtr node = createNode( packet->nodeType );
    EQASSERT( node.isValid( ));

    std::string data = packet->nodeData;
    if( !node->deserialize( data ))
        EQWARN << "Failed to initialize node data" << std::endl;
    EQASSERT( data.empty( ));

    {
        base::ScopedMutex< base::SpinLock > mutex( _nodes );
        _nodes.data[ nodeID ] = node;
    }
    EQVERB << "Added node " << nodeID << " without connection" << std::endl;

    node.ref();
    serveRequest( requestID, node.get( ));
    return true;
}

bool Node::_cmdAcquireSendToken( Command& command )
{
    EQASSERT( inCommandThread( ));
    if( !_hasSendToken ) // no token available
    {
        command.retain();
        _sendTokenQueue.push_back( &command );
        return true;
    }

    _hasSendToken = false;

    NodeAcquireSendTokenPacket* packet = 
        command.getPacket<NodeAcquireSendTokenPacket>();
    NodeAcquireSendTokenReplyPacket reply( packet );
    command.getNode()->send( reply );
    return true;
}

bool Node::_cmdAcquireSendTokenReply( Command& command )
{
    NodeAcquireSendTokenReplyPacket* packet = 
        command.getPacket<NodeAcquireSendTokenReplyPacket>();

    serveRequest( packet->requestID );
    return true;
}

bool Node::_cmdReleaseSendToken( Command& )
{
    EQASSERT( inCommandThread( ));
    EQASSERT( !_hasSendToken );

    if( _sendTokenQueue.empty( ))
    {
        _hasSendToken = true;
        return true;
    }

    Command* request = _sendTokenQueue.front();
    _sendTokenQueue.pop_front();

    NodeAcquireSendTokenPacket* packet = 
        request->getPacket<NodeAcquireSendTokenPacket>();
    NodeAcquireSendTokenReplyPacket reply( packet );

    request->getNode()->send( reply );
    request->release();
    return true;
}

std::ostream& operator << ( std::ostream& os, const Node& node )
{
    os << "node " << node.getNodeID() << " " << node._state;
    const ConnectionDescriptions& descs = node.getConnectionDescriptions();
    for( ConnectionDescriptions::const_iterator i = descs.begin();
         i != descs.end(); ++i )
    {
        os << ", " << (*i)->toString();
    }
    return os;
}

std::ostream& operator << ( std::ostream& os, const Node::State state)
{
    os << ( state == Node::STATE_CLOSED ? "closed" :
            state == Node::STATE_CONNECTED ? "connected" :
            state == Node::STATE_LISTENING ? "listening" : "ERROR" );
    return os;
}

}
}
