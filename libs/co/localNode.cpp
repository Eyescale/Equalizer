
/* Copyright (c)  2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                     2010, Cedric Stalder <cedric.stalder@gmail.com> 
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
 
#include "localNode.h"

#include "command.h"
#include "connectionDescription.h"
#include "dataIStream.h"
#include "exception.h"
#include "global.h"
#include "nodePackets.h"
#include "object.h"
#include "objectStore.h"
#include "pipeConnection.h"

#include <co/base/log.h>
#include <co/base/requestHandler.h>
#include <co/base/scopedMutex.h>

namespace co
{

typedef CommandFunc<LocalNode> CmdFunc;

LocalNode::LocalNode( )
        : _hasSendToken( true )
        , _lastTokenTime( 0 )
        , _objectStore( 0 )
{
    _receiverThread = new ReceiverThread( this );
    _commandThread  = new CommandThread( this );

    _objectStore = new ObjectStore( this );

    CommandQueue* queue = getCommandThreadQueue();
    _registerCommand( CMD_NODE_ACK_REQUEST, 
                      CmdFunc( this, &LocalNode::_cmdAckRequest ), 0 );
    registerCommand( CMD_NODE_STOP_RCV,
                     CmdFunc( this, &LocalNode::_cmdStopRcv ), 0 );
    registerCommand( CMD_NODE_STOP_CMD,
                     CmdFunc( this, &LocalNode::_cmdStopCmd ), queue );
    registerCommand( CMD_NODE_CONNECT,
                    CmdFunc( this, &LocalNode::_cmdConnect ), 0);
    registerCommand( CMD_NODE_CONNECT_REPLY,
                    CmdFunc( this, &LocalNode::_cmdConnectReply ), 0 );
    registerCommand( CMD_NODE_CONNECT_ACK, 
                    CmdFunc( this, &LocalNode::_cmdConnectAck ), 0 );
    registerCommand( CMD_NODE_ID,
                    CmdFunc( this, &LocalNode::_cmdID ), 0 );
    registerCommand( CMD_NODE_DISCONNECT,
                    CmdFunc( this, &LocalNode::_cmdDisconnect ), 0 );
    registerCommand( CMD_NODE_GET_NODE_DATA,
                    CmdFunc( this, &LocalNode::_cmdGetNodeData), queue );
    registerCommand( CMD_NODE_GET_NODE_DATA_REPLY,
                    CmdFunc( this, &LocalNode::_cmdGetNodeDataReply ), 0 );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN,
                    CmdFunc( this, &LocalNode::_cmdAcquireSendToken ), queue );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY,
                     CmdFunc( this, &LocalNode::_cmdAcquireSendTokenReply ), 0);
    registerCommand( CMD_NODE_RELEASE_SEND_TOKEN,
                     CmdFunc( this, &LocalNode::_cmdReleaseSendToken ), queue );
    registerCommand( CMD_NODE_ADD_LISTENER,
                     CmdFunc( this, &LocalNode::_cmdAddListener ), 0 );
    registerCommand( CMD_NODE_REMOVE_LISTENER,
                     CmdFunc( this, &LocalNode::_cmdRemoveListener ), 0 );
    registerCommand( CMD_NODE_PING,
                     CmdFunc( this, &LocalNode::_cmdPing ), queue );
    registerCommand( CMD_NODE_PING_REPLY,
                     CmdFunc( this, &LocalNode::_cmdDiscard ), 0 );
}

LocalNode::~LocalNode( )
{
    EQASSERT( _incoming.isEmpty( ));
    EQASSERT( _connectionNodes.empty( ));
    EQASSERT( _pendingCommands.empty( ));
    EQASSERT( _nodes->empty( ));
    EQASSERT( !hasPendingRequests( ));

    delete _objectStore;
    _objectStore = 0;
    EQASSERT( !_commandThread->isRunning( ));
    delete _commandThread;
    _commandThread = 0;

    EQASSERT( !_receiverThread->isRunning( ));
    delete _receiverThread;
    _receiverThread = 0;
}

bool LocalNode::initLocal( const int argc, char** argv )
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
    // - different behavior of GNU and BSD implementations
    // - incomplete man pages
    for( int i=1; i<argc; ++i )
    {
        if( std::string( "--eq-listen" ) == argv[i] )
        {
            if( (i+1)<argc && argv[i+1][0] != '-' )
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
                    EQWARN << "Ignoring listen option: " << argv[i] <<std::endl;
            }
            else
            {
                EQWARN << "No argument given to --eq-listen!" << std::endl;
            }
        }
        else if ( std::string( "--co-globals" ) == argv[i] )
        {
            if( (i+1)<argc && argv[i+1][0] != '-' )
            {
                const std::string data = argv[++i];
                if( !Global::fromString( data ))
                {
                    EQWARN << "Invalid global variables string: " << data
                           << ", using default global variables." << std::endl;
                }
            }
            else
            {
                EQWARN << "No argument given to --co-globals!" << std::endl;
            }
        }
    }
    
    if( !listen( ))
    {
        EQWARN << "Can't setup listener(s) on " << *static_cast< Node* >( this )
               << std::endl; 
        return false;
    }
    return true;
}

bool LocalNode::listen()
{
    EQVERB << "Listener data: " << serialize() << std::endl;
    if( !isClosed() || !_connectSelf( ))
        return false;

    ConnectionDescriptions descriptions = getConnectionDescriptions();
    for( ConnectionDescriptionsCIter i = descriptions.begin();
         i != descriptions.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        ConnectionPtr connection = Connection::create( description );

        if( !connection || !connection->listen( ))
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
    
    EQVERB << base::className( this ) << " start command and receiver thread "
           << std::endl;
    _receiverThread->start();

    EQINFO << *this << std::endl;
    return true;
}

bool LocalNode::listen( ConnectionPtr connection )
{
    if( !listen( ))
        return false;
    _addConnection( connection );
    return true;
}

bool LocalNode::close() 
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

ConnectionPtr LocalNode::addListener( ConnectionDescriptionPtr desc )
{
    EQASSERT( isListening( ));

    ConnectionPtr connection = Connection::create( desc );

    if( connection && connection->listen( ))
    {
        addListener( connection );
        return connection;
    }
    return 0;
}

void LocalNode::addListener( ConnectionPtr connection )
{
    EQASSERT( isListening( ));
    EQASSERT( connection->isListening( ));

    connection->ref( CO_REFERENCED_PARAM );
    NodeAddListenerPacket packet( connection );

    // Update everybody's description list of me
    // I will add the listener to myself in my handler
    Nodes nodes;
    getNodes( nodes );

    for( NodesIter i = nodes.begin(); i != nodes.end(); ++i )
        (*i)->send( packet, connection->getDescription()->toString( ));
}

void LocalNode::removeListeners( const Connections& connections )
{
    std::vector< uint32_t > requests;
    for( ConnectionsCIter i = connections.begin(); i != connections.end(); ++i )
    {
        co::ConnectionPtr connection = *i;
        requests.push_back( _removeListenerNB( connection ));
    }

    for( size_t i = 0; i < connections.size(); ++i )
    {
        co::ConnectionPtr connection = connections[i];
        waitRequest( requests[ i ] );
        connection->close();
        // connection and connections hold a reference
        EQASSERTINFO( connection->getRefCount()==2 ||
              connection->getDescription()->type >= co::CONNECTIONTYPE_MULTICAST,
                      connection->getRefCount() << ": " << *connection );
    }
}

uint32_t LocalNode::_removeListenerNB( ConnectionPtr connection )
{
    EQASSERT( isListening( ));
    EQASSERT( connection->isListening( ));

    connection->ref( CO_REFERENCED_PARAM );
    NodeRemoveListenerPacket packet( connection, registerRequest( ));
    Nodes nodes;
    getNodes( nodes );

    for( Nodes::iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        (*i)->send( packet, connection->getDescription()->toString( ));
    }
    return packet.requestID;
}

void LocalNode::_addConnection( ConnectionPtr connection )
{
    _incoming.addConnection( connection );
    connection->recvNB( new uint64_t, sizeof( uint64_t ));
}

void LocalNode::_removeConnection( ConnectionPtr connection )
{
    EQASSERT( connection.isValid( ));

    _incoming.removeConnection( connection );

    void* buffer( 0 );
    uint64_t bytes( 0 );
    connection->getRecvData( &buffer, &bytes );
    EQASSERTINFO( !connection->isConnected() || buffer, *connection );
    EQASSERT( !buffer || bytes == sizeof( uint64_t ));

    if( !connection->isClosed( ))
        connection->close(); // cancels pending IO's
    delete reinterpret_cast< uint64_t* >( buffer );
}

void LocalNode::_cleanup()
{
    EQVERB << "Clean up stopped node" << std::endl;
    EQASSERTINFO( _state == STATE_CLOSED, _state );

    _multicasts.clear();
    PipeConnectionPtr pipe = EQSAFECAST( PipeConnection*, _outgoing.get( ));
    _removeConnection( pipe->acceptSync( ));
    _connectionNodes.erase( pipe->acceptSync( ));
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

bool LocalNode::_connectSelf()
{
    // setup local connection to myself
    PipeConnectionPtr connection = new PipeConnection;
    if( !connection->connect( ))
    {
        EQERROR << "Could not create local connection to receiver thread."
                << std::endl;
        return false;
    }

    _outgoing = connection->acceptSync();

    // add to connection set
    EQASSERT( connection->getDescription().isValid( )); 
    EQASSERT( _connectionNodes.find( connection ) == _connectionNodes.end( ));

    _connectionNodes[ connection ] = this;
    _nodes.data[ _id ] = this;
    _addConnection( connection );

    EQVERB << "Added node " << _id << " using " << connection << std::endl;
    return true;
}

void LocalNode::_connectMulticast( NodePtr node )
{
    EQASSERT( _inReceiverThread( ));
    base::ScopedMutex<> mutex( _outMulticast );

    if( node->_outMulticast.data.isValid( ))
        // multicast already connected by previous _cmdID
        return;

    // Search if the connected node is in the same multicast group as we are
    ConnectionDescriptions descriptions = getConnectionDescriptions();
    for( ConnectionDescriptions::const_iterator i = descriptions.begin();
         i != descriptions.end(); ++i )
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

bool LocalNode::disconnect( NodePtr node )
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
    _objectStore->removeNode( node );
    return true;
}

void LocalNode::ackRequest( NodePtr node, const uint32_t requestID )
{
    if( requestID == EQ_UNDEFINED_UINT32 ) // no need to ack operation
        return;

    if( node == this ) // OPT
        serveRequest( requestID );
    else
    {
        NodeAckRequestPacket reply( requestID );
        node->send( reply );
    }
}

void LocalNode::ping( NodePtr remoteNode )
{
    EQASSERT( !_inReceiverThread( ) );
    NodePingPacket packet;
    remoteNode->send( packet );
}

bool LocalNode::pingIdleNodes()
{
    EQASSERT( !_inReceiverThread( ) );
    const int64_t timeout = co::Global::getKeepaliveTimeout();
    Nodes nodes;
    getNodes( nodes, false );

    bool pinged = false;
    for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
    {
        NodePtr node = *i;
        if( getTime64() - node->getLastReceiveTime() > timeout )
        {
            EQINFO << " Ping Node: " <<  node->getNodeID() << " last seen "
                   << node->getLastReceiveTime() << std::endl;
            NodePingPacket packet;
            node->send( packet );
            pinged = true;
        }
    }
    return pinged;
}

//----------------------------------------------------------------------
// Object functionality
//----------------------------------------------------------------------
void LocalNode::disableInstanceCache()
{
    _objectStore->disableInstanceCache();
}

void LocalNode::expireInstanceData( const int64_t age )
{
    _objectStore->expireInstanceData( age );
}

void LocalNode::enableSendOnRegister()
{
    _objectStore->enableSendOnRegister();
}

void LocalNode::disableSendOnRegister()
{
    _objectStore->disableSendOnRegister();
}

bool LocalNode::registerObject( Object* object )
{
    return _objectStore->registerObject( object );
}

void LocalNode::deregisterObject( Object* object )
{
    _objectStore->deregisterObject( object );
}
bool LocalNode::mapObject( Object* object, const base::UUID& id,
                           const uint128_t& version )
{
    const uint32_t requestID = _objectStore->mapObjectNB( object, id, version );
    return _objectStore->mapObjectSync( requestID );
}

uint32_t LocalNode::mapObjectNB( Object* object, const base::UUID& id, 
                            const uint128_t& version )
{
    return _objectStore->mapObjectNB( object, id, version );
}

uint32_t LocalNode::mapObjectNB( Object* object, const base::UUID& id, 
                                 const uint128_t& version, NodePtr master )
{
    return _objectStore->mapObjectNB( object, id, version, master );
}


bool LocalNode::mapObjectSync( const uint32_t requestID )
{
    return _objectStore->mapObjectSync( requestID );
}

void LocalNode::unmapObject( Object* object )
{
    _objectStore->unmapObject( object );
}

void LocalNode::swapObject( Object* oldObject, Object* newObject )
{
    _objectStore->swapObject( oldObject, newObject );
}

void LocalNode::releaseObject( Object* object )
{
    EQASSERT( object );
    if( !object || !object->isAttached( ))
        return;

    if( object->isMaster( ))
        _objectStore->deregisterObject( object );
    else
        _objectStore->unmapObject( object );
}

void LocalNode::objectPush( const uint128_t& groupID, const uint128_t& typeID,
                            const uint128_t& objectID, DataIStream& istream )
{
    if( istream.hasData( ))
        EQWARN << "Incomplete Object::push for group " << groupID << " type "
               << typeID << " object " << objectID << std::endl;
}

LocalNode::SendToken LocalNode::acquireSendToken( NodePtr node )
{
    EQASSERT( !inCommandThread( ));
    EQASSERT( !_inReceiverThread( ));

    NodeAcquireSendTokenPacket packet;
    packet.requestID = registerRequest();
    node->send( packet );
    
    bool ret = false;
    if( waitRequest( packet.requestID, ret, Global::getTimeout( )))
        return node;

    EQERROR << "Timeout while acquiring send token " << packet.requestID
            << std::endl;
    return 0;
}

void LocalNode::releaseSendToken( SendToken& node )
{
    EQASSERT( !_inReceiverThread( ));
    if( !node )
        return;

    NodeReleaseSendTokenPacket packet;
    node->send( packet );
    node = 0; // In case app stores token in member variable
}

//----------------------------------------------------------------------
// Connecting a node
//----------------------------------------------------------------------
NodePtr LocalNode::connect( const NodeID& nodeID )
{
    EQASSERT( nodeID != NodeID::ZERO );
    EQASSERT( _state == STATE_LISTENING );

    Nodes nodes;
    getNodes( nodes );

    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        NodePtr peer = *i;
        if( peer->getNodeID() == nodeID && peer->isConnected( )) // early out
            return peer;
    }

    EQINFO << "Connecting node " << nodeID << std::endl;
    for( Nodes::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        NodePtr peer = *i;
        NodePtr node = _connect( nodeID, peer );
        if( node.isValid( ))
            return node;
    }

    EQWARN << "Node " << nodeID << " connection failed" << std::endl;
    EQUNREACHABLE;
    return 0;
}

NodePtr LocalNode::_connect( const NodeID& nodeID, NodePtr peer )
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
        EQASSERT( node->isConnected( ));
        if( !node->isConnected( ))
            connect( node );
        return node->isConnected() ? node : 0;
    }
    EQASSERT( _id != nodeID );

    NodeGetNodeDataPacket packet;
    packet.requestID = registerRequest();
    packet.nodeID    = nodeID;
    peer->send( packet );

    void* result = 0;
    waitRequest( packet.requestID, result );

    if( !result )
    {
        EQINFO << "Node " << nodeID << " not found on " << peer->getNodeID() 
               << std::endl;
        return 0;
    }

    EQASSERT( dynamic_cast< Node* >( (Dispatcher*)result ));
    node = static_cast< Node* >( result );
    node->unref( CO_REFERENCED_PARAM ); // ref'd before serveRequest()

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
            if( !node->isConnected( ))
                connect( node );
        }
    }

    return node->isConnected() ? node : 0;
}

bool LocalNode::connect( NodePtr node )
{
    EQASSERTINFO( _state == STATE_LISTENING, _state );
    if( node->_state == STATE_CONNECTED || node->_state == STATE_LISTENING )
        return true;

    EQASSERT( node->_state == STATE_CLOSED );
    EQINFO << "Connecting " << node << std::endl;

    // try connecting using the given descriptions
    const ConnectionDescriptions& cds = node->getConnectionDescriptions();
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

bool LocalNode::_connect( NodePtr node, ConnectionPtr connection )
{
    EQASSERT( connection.isValid( ));
    EQASSERT( node->getNodeID() != getNodeID( ));

    if( !node.isValid() || _state != STATE_LISTENING ||
        !connection->isConnected() || node->_state != STATE_CLOSED )
    {
        return false;
    }

    _addConnection( connection );

    // send connect packet to peer
    NodeConnectPacket packet( this );
    packet.requestID = registerRequest( node.get( ));
    connection->send( packet, serialize( ));

    bool connected = false;
    if( !waitRequest( packet.requestID, connected, 10000 /*ms*/ ))
    {
        EQWARN << "Node connection handshake timeout - peer not a Collage node?"
               << std::endl;
        return false;
    }
    if( !connected )
        return false;

    EQASSERT( node->_id != NodeID::ZERO );
    EQASSERTINFO( node->_id != _id, _id );
    EQINFO << node << " connected to " << *(Node*)this << std::endl;
    return true;
}

NodePtr LocalNode::getNode( const NodeID& id ) const
{
    base::ScopedMutex< base::SpinLock > mutex( _nodes );
    NodeHash::const_iterator i = _nodes->find( id );
    if( i == _nodes->end( ))
        return 0;
    EQASSERT( i->second->isConnected( ));
    return i->second;
}

void LocalNode::getNodes( Nodes& nodes, const bool addSelf ) const
{
    base::ScopedMutex< base::SpinLock > mutex( _nodes );
    for( NodeHash::const_iterator i = _nodes->begin();
         i != _nodes->end(); ++i )
    {
        EQASSERT( i->second->isConnected( ));
        if( addSelf || i->second != this )
            nodes.push_back( i->second );
    }
}

//----------------------------------------------------------------------
// receiver thread functions
//----------------------------------------------------------------------
void LocalNode::_runReceiverThread()
{
    EQ_TS_THREAD( _rcvThread );

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
    _objectStore->clear();
    _pendingCommands.clear();
    _commandCache.flush();

    EQINFO << "Leaving receiver thread of " << base::className( this )
           << std::endl;
}

void LocalNode::_handleConnect()
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

void LocalNode::_handleDisconnect()
{
    while( _handleData( )) ; // read remaining data off connection

    ConnectionPtr connection = _incoming.getConnection();
    ConnectionNodeHash::iterator i = _connectionNodes.find( connection );

    if( i != _connectionNodes.end( ))
    {
        NodePtr node = i->second;
        Command& command = _commandCache.alloc( node, this,
                                                sizeof( NodeRemoveNodePacket ));
        NodeRemoveNodePacket* packet =
             command.getModifiable< NodeRemoveNodePacket >();
        *packet = NodeRemoveNodePacket();
        packet->node = node.get();
        _dispatchCommand( command );

        if( node->_outgoing == connection )
        {
            _objectStore->removeInstanceData( node->_id );
            _connectionNodes.erase( i );
            node->_state    = STATE_CLOSED;
            node->_outgoing = 0;

            if( node->_outMulticast.data.isValid( ) )
                _removeConnection( node->_outMulticast.data );

            node->_outMulticast = 0;
            node->_multicasts.clear();

            EQINFO << node << " disconnected from " << *this << std::endl;
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

        notifyDisconnect( node );
    }

    _removeConnection( connection );
    EQINFO << "connection used " << connection->getRefCount() << std::endl;
}

bool LocalNode::_handleData()
{
    ConnectionPtr connection = _incoming.getConnection();
    EQASSERT( connection.isValid( ));

    NodePtr node;
    ConnectionNodeHash::const_iterator i = _connectionNodes.find( connection );
    if( i != _connectionNodes.end( ))
        node = i->second;
    EQASSERTINFO( !node || // unconnected node
                  *(node->_outgoing) == *connection || // correct UC connection
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

    if( node )
        node->_lastReceive = getTime64();

    Command& command = _commandCache.alloc( node, this, size );
    uint8_t* ptr = reinterpret_cast< uint8_t* >(
        command.getModifiable< Packet >()) + sizeof( uint64_t );

    connection->recvNB( ptr, size - sizeof( uint64_t ));
    const bool gotData = connection->recvSync( 0, 0 );

    EQASSERT( gotData );
    EQASSERT( command.isValid( ));
    EQASSERT( command.isFree( ));

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
                 ( command->type == PACKETTYPE_CO_NODE &&
                  ( command->command == CMD_NODE_CONNECT  || 
                    command->command == CMD_NODE_CONNECT_REPLY ||
                    command->command == CMD_NODE_ID )),
                  command << " connection " << connection );

    _dispatchCommand( command );
    return true;
}

Command& LocalNode::allocCommand( const uint64_t size )
{
    EQASSERT( _inReceiverThread( ));
    return _commandCache.alloc( this, this, size );
}

void LocalNode::_dispatchCommand( Command& command )
{
    EQASSERT( command.isValid( ));
    command.retain();

    if( dispatchCommand( command ))
    {
        command.release();
        _redispatchCommands();
    }
    else
    {
        _redispatchCommands();
        _pendingCommands.push_back( &command );
    }
}

bool LocalNode::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << " by " << _id << std::endl;
    EQASSERT( command.isValid( ));

    const uint32_t type = command->type;
    switch( type )
    {
        case PACKETTYPE_CO_NODE:
            EQCHECK( Dispatcher::dispatchCommand( command ));
            return true;

        case PACKETTYPE_CO_OBJECT:
        {
            return _objectStore->dispatchObjectCommand( command );
        }

        default:
            EQABORT( "Unknown packet type " << type << " for " << command );
            return true;
    }
}

void LocalNode::_redispatchCommands()
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
bool LocalNode::CommandThread::init()
{
    setName( std::string( "Cmd " ) + base::className( _localNode ));
    return true;
}

bool LocalNode::CommandThread::notifyIdle()
{
    return _localNode->_objectStore->notifyCommandThreadIdle();
}

bool LocalNode::_cmdAckRequest( Command& command )
{
    const NodeAckRequestPacket* packet = command.get< NodeAckRequestPacket >();
    EQASSERT( packet->requestID != EQ_UNDEFINED_UINT32 );

    serveRequest( packet->requestID );
    return true;
}

bool LocalNode::_cmdStopRcv( Command& command )
{
    EQ_TS_THREAD( _rcvThread );
    EQASSERT( _state == STATE_LISTENING );
    EQINFO << "Cmd stop receiver " << this << std::endl;

    _state = STATE_CLOSING; // causes rcv thread exit

    command->command = CMD_NODE_STOP_CMD; // causes cmd thread exit
    _dispatchCommand( command );
    return true;
}

bool LocalNode::_cmdStopCmd( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    EQASSERT( _state == STATE_CLOSING );
    EQINFO << "Cmd stop command " << this << std::endl;

    _state = STATE_CLOSED;
    return true;
}

bool LocalNode::_cmdConnect( Command& command )
{
    EQASSERT( !command.getNode().isValid( ));
    EQASSERT( _inReceiverThread( ));

    const NodeConnectPacket* packet = command.get< NodeConnectPacket >();
    ConnectionPtr connection = _incoming.getConnection();
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
        base::ScopedFastWrite mutex( _nodes );
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

bool LocalNode::_cmdConnectReply( Command& command )
{
    EQASSERT( !command.getNode( ));
    EQASSERT( _inReceiverThread( ));

    const NodeConnectReplyPacket* packet =
        command.get< NodeConnectReplyPacket >();
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
        
        if( packet->requestID != EQ_UNDEFINED_UINT32 )
            serveRequest( packet->requestID, false );
        
        return true;
    }

    // create and add node
    if( !remoteNode )
    {
        if( packet->requestID != EQ_UNDEFINED_UINT32 )
        {
            void* ptr = getRequestData( packet->requestID );
            EQASSERT( dynamic_cast< Node* >( (Dispatcher*)ptr ));
            remoteNode = static_cast< Node* >( ptr );
        }
        else
            remoteNode = createNode( packet->nodeType );
    }

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
        base::ScopedFastWrite mutex( _nodes );
        _nodes.data[ remoteNode->_id ] = remoteNode;
    }
    EQVERB << "Added node " << nodeID << std::endl;

    if( packet->requestID != EQ_UNDEFINED_UINT32 )
        serveRequest( packet->requestID, true );

    NodeConnectAckPacket ack;
    remoteNode->send( ack );

    _connectMulticast( remoteNode );
    return true;
}

bool LocalNode::_cmdConnectAck( Command& command )
{
    NodePtr node = command.getNode();
    EQASSERT( node.isValid( ));
    EQASSERT( _inReceiverThread( ));
    EQVERB << "handle connect ack" << std::endl;
    
    _connectMulticast( node );
    return true;
}

bool LocalNode::_cmdID( Command& command )
{
    EQASSERT( _inReceiverThread( ));

    const NodeIDPacket* packet = command.get< NodeIDPacket >();
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
                base::ScopedFastWrite mutex( _nodes );
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
           << " to " << _id << std::endl;
    return true;
}

bool LocalNode::_cmdDisconnect( Command& command )
{
    EQASSERT( _inReceiverThread( ));

    const NodeDisconnectPacket* packet = command.get< NodeDisconnectPacket >();

    NodePtr node = static_cast<Node*>( getRequestData( packet->requestID ));
    EQASSERT( node.isValid( ));

    ConnectionPtr connection = node->_outgoing;
    if( connection.isValid( ))
    {
        node->_state    = STATE_CLOSED;
        node->_outgoing = 0;

        _removeConnection( connection );

        EQASSERT( _connectionNodes.find( connection )!=_connectionNodes.end( ));
        _objectStore->removeInstanceData( node->_id );
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

bool LocalNode::_cmdGetNodeData( Command& command)
{
    const NodeGetNodeDataPacket* packet = command.get<NodeGetNodeDataPacket>();
    EQVERB << "cmd get node data: " << packet << std::endl;

    const NodeID& nodeID = packet->nodeID;
    NodePtr node = getNode( nodeID );
    NodePtr toNode = command.getNode();
    NodeGetNodeDataReplyPacket reply( packet );

    std::string nodeData;
    if( node.isValid( ))
    {
        reply.nodeType = node->getType();
        nodeData = node->serialize();
        EQINFO << "Sent node data '" << nodeData << "' for " << nodeID << " to "
               << toNode << std::endl;
    }
    else
    {
        EQVERB << "Node " << nodeID << " unknown" << std::endl;
        reply.nodeType = NODETYPE_CO_INVALID;
    }

    toNode->send( reply, nodeData );
    return true;
}

bool LocalNode::_cmdGetNodeDataReply( Command& command )
{
    EQASSERT( _inReceiverThread( ));

    const NodeGetNodeDataReplyPacket* packet = 
        command.get< NodeGetNodeDataReplyPacket >();
    EQVERB << "cmd get node data reply: " << packet << std::endl;

    const uint32_t requestID = packet->requestID;
    const NodeID& nodeID = packet->nodeID;

    // No locking needed, only recv thread writes
    NodeHash::const_iterator i = _nodes->find( nodeID );
    if( i != _nodes->end( ))
    {
        // Requested node connected to us in the meantime
        NodePtr node = i->second;
        
        node->ref( CO_REFERENCED_PARAM );
        serveRequest( requestID, node.get( ));
        return true;
    }

    if( packet->nodeType == NODETYPE_CO_INVALID )
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

    node->ref( CO_REFERENCED_PARAM );
    serveRequest( requestID, node.get( ));
    return true;
}

bool LocalNode::_cmdAcquireSendToken( Command& command )
{
    EQASSERT( inCommandThread( ));
    if( !_hasSendToken ) // no token available
    {
        const uint32_t timeout = Global::getTimeout();
        if( timeout == EQ_TIMEOUT_INDEFINITE ||
            ( getTime64() - _lastTokenTime <= timeout ))
        {
            command.retain();
            _sendTokenQueue.push_back( &command );
            return true;
        }

        // timeout! - clear old requests
        _sendTokenQueue.clear();
        // 'generate' new token - release is robust
    }

    _hasSendToken = false;

    const NodeAcquireSendTokenPacket* packet = 
        command.get< NodeAcquireSendTokenPacket >();
    NodeAcquireSendTokenReplyPacket reply( packet );
    command.getNode()->send( reply );
    return true;
}

bool LocalNode::_cmdAcquireSendTokenReply( Command& command )
{
    const NodeAcquireSendTokenReplyPacket* packet = 
        command.get< NodeAcquireSendTokenReplyPacket >();
    serveRequest( packet->requestID );
    return true;
}

bool LocalNode::_cmdReleaseSendToken( Command& )
{
    EQASSERT( inCommandThread( ));
    _lastTokenTime = getTime64();

    if( _hasSendToken )
        return true; // double release due to timeout
    if( _sendTokenQueue.empty( ))
    {
        _hasSendToken = true;
        return true;
    }

    Command* request = _sendTokenQueue.front();
    _sendTokenQueue.pop_front();

    const NodeAcquireSendTokenPacket* packet = 
        request->get< NodeAcquireSendTokenPacket >();
    NodeAcquireSendTokenReplyPacket reply( packet );

    request->getNode()->send( reply );
    request->release();
    return true;
}

bool LocalNode::_cmdAddListener( Command& command )
{
    NodeAddListenerPacket* packet =
        command.getModifiable< NodeAddListenerPacket >();
    ConnectionDescriptionPtr description =
        new ConnectionDescription( packet->connectionData );
    command.getNode()->addConnectionDescription( description );

    if( command.getNode() != this )
        return true;

    EQASSERT( packet->connection );
    ConnectionPtr connection = packet->connection;
    packet->connection = 0;
    connection->unref( CO_REFERENCED_PARAM );

    _connectionNodes[ connection ] = this;
    _incoming.addConnection( connection );
    if( connection->getDescription()->type >= CONNECTIONTYPE_MULTICAST )
    {
        MCData data;
        data.connection = connection;
        data.node = this;

        base::ScopedMutex<> mutex( _outMulticast );
        _multicasts.push_back( data );
    }

    connection->acceptNB();
    return true;
}

bool LocalNode::_cmdRemoveListener( Command& command )
{
    NodeRemoveListenerPacket* packet = 
        command.getModifiable< NodeRemoveListenerPacket >();

    ConnectionDescriptionPtr description =
        new ConnectionDescription( packet->connectionData );
    EQCHECK( command.getNode()->removeConnectionDescription( description ));

    if( command.getNode() != this )
        return true;

    EQASSERT( packet->connection );
    ConnectionPtr connection = packet->connection;
    packet->connection = 0;
    connection->unref( CO_REFERENCED_PARAM );

    if( connection->getDescription()->type >= CONNECTIONTYPE_MULTICAST )
    {
        base::ScopedMutex<> mutex( _outMulticast );
        for( MCDatas::iterator i = _multicasts.begin();
             i != _multicasts.end(); ++i )
        {
            if( i->connection == connection )
            {
                _multicasts.erase( i );
                break;
            }
        }
    }

    _incoming.removeConnection( connection );
    EQASSERT( _connectionNodes.find( connection ) != _connectionNodes.end( ));
    _connectionNodes.erase( connection );
    serveRequest( packet->requestID );
    return true;
}

bool LocalNode::_cmdPing( Command& command )
{
    EQASSERT( inCommandThread( ));
    NodePingReplyPacket reply;
    command.getNode()->send( reply );
    return true;
}

}
