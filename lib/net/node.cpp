
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "connectionSet.h"
#include "global.h"
#include "pipeConnection.h"
#include "session.h"
#include "socketConnection.h"

#include <eq/base/base.h>
#include <eq/base/launcher.h>
#include <eq/base/rng.h>

#include <errno.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32_API
#  include <direct.h>  // for chdir
#  define chdir _chdir
#endif

namespace eq
{
namespace net
{
typedef CommandFunc<Node> NodeFunc;

Node::Node()
        : _requestHandler( true )
        , _id( true )
        , _state( STATE_STOPPED )
        , _autoLaunch( false )
        , _launchID( EQ_ID_INVALID )
        , _launchTimeout( 60000 ) // ms
        , _launchCommandQuote( '\'' )
        , _programName( Global::getProgramName( ))
        , _workDir( Global::getWorkDir( ))
        , _hasSendToken( true )
{
    _receiverThread = new ReceiverThread( this );
    _commandThread  = new CommandThread( this );

    CommandQueue* queue = &_commandThreadQueue;
    registerCommand( CMD_NODE_STOP, NodeFunc( this, &Node::_cmdStop ), queue );
    registerCommand( CMD_NODE_REGISTER_SESSION,
                     NodeFunc( this, &Node::_cmdRegisterSession ), queue );
    registerCommand( CMD_NODE_REGISTER_SESSION_REPLY,
                     NodeFunc( this, &Node::_cmdRegisterSessionReply ), queue );
    registerCommand( CMD_NODE_MAP_SESSION,
                     NodeFunc( this, &Node::_cmdMapSession ), queue );
    registerCommand( CMD_NODE_MAP_SESSION_REPLY,
                     NodeFunc( this, &Node::_cmdMapSessionReply ), queue );
    registerCommand( CMD_NODE_UNMAP_SESSION, 
                     NodeFunc( this, &Node::_cmdUnmapSession ), queue );
    registerCommand( CMD_NODE_UNMAP_SESSION_REPLY,
                     NodeFunc( this, &Node::_cmdUnmapSessionReply ), queue );
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
                     NodeFunc( this, &Node::_cmdGetNodeDataReply ), queue );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN,
                     NodeFunc( this, &Node::_cmdAcquireSendToken ), 0 );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY,
                     NodeFunc( this, &Node::_cmdAcquireSendTokenReply ) , 0 );
    registerCommand( CMD_NODE_RELEASE_SEND_TOKEN,
                     NodeFunc( this, &Node::_cmdReleaseSendToken ), 0 );

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
    EQASSERT( _requestHandler.isEmpty( ));

#ifndef NDEBUG
    if( !_sessions.empty( ))
    {
        EQINFO << _sessions.size() << " mapped sessions" << std::endl;
        
        for( SessionHash::const_iterator i = _sessions.begin();
             i != _sessions.end(); ++i )
        {
            const Session* session = i->second;
            EQINFO << "    Session " << session->getID() << std::endl;
        }
    }
#endif

    EQASSERT( _sessions.empty( ));

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

void Node::setProgramName( const std::string& name )
{
    _programName = name;
}

void Node::setWorkDir( const std::string& name )
{
    _workDir = name;
}

const ConnectionDescriptionVector& Node::getConnectionDescriptions() const 
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

    base::ScopedMutex mutex( _outMulticast );
    if( _multicasts.empty( ))
        return 0;

    MCData data = _multicasts.back();
    _multicasts.pop_back();

    // prime multicast connections on peers
    EQINFO << "Announcing id " << data.serverID << " to multicast group "
           << data.connection->getDescription() << std::endl;

    NodeIDPacket packet;
    packet.id = data.serverID;
    packet.id.convertToNetwork();
    packet.type = getType();

    data.connection->send( packet, serialize( ));
    _outMulticast.data = data.connection;
    return data.connection;
}

void Node::setLaunchCommand( const std::string& launchCommand )
{
    _launchCommand = launchCommand;
}

const std::string& Node::getLaunchCommand() const
{
    return _launchCommand;
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
    bool   isClient   = false;
    bool   isResident = false;
    std::string clientOpts;

    for( int i=1; i<argc; ++i )
    {
        if( std::string( "--eq-listen" ) == argv[i] )
        {
            if( i<argc && argv[i+1][0] != '-' )
            {
                std::string                        data = argv[++i];
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
        else if( std::string( "--eq-client" ) == argv[i] )
        {
            isClient = true;
            if( i < argc-1 && argv[i+1][0] != '-' ) // server-started client
            {
                clientOpts = argv[++i];

                if( !deserialize( clientOpts ))
                    EQWARN << "Failed to parse client listen port parameters"
                           << std::endl;
                EQASSERT( !clientOpts.empty( ));
            }
            else // resident render client
                isResident = true;
        }
    }
    
    if( _connectionDescriptions.empty( )) // add default listener
    {
        ConnectionDescriptionPtr connDesc = new ConnectionDescription;
        connDesc->type = CONNECTIONTYPE_TCPIP;
        connDesc->port = Global::getDefaultPort();
        addConnectionDescription( connDesc );
    }

    EQVERB << "Listener data: " << serialize() << std::endl;

    if( !listen( ))
    {
        EQWARN << "Can't setup listener(s)" << std::endl; 
        return false;
    }
    
    if( isClient )
    {
        EQVERB << "Client node started from command line with option " 
               << clientOpts << std::endl;

        bool ret = (isResident ? clientLoop() : runClient( clientOpts ));

        EQINFO << "Exit node process " << getRefCount() << std::endl;
        ret &= exitClient();
        ::exit( ret ? EXIT_SUCCESS : EXIT_FAILURE );
    }

    return true;
}

bool Node::listen()
{
    if( _state != STATE_STOPPED )
        return false;

    if( !_connectSelf( ))
        return false;

    for( ConnectionDescriptionVector::const_iterator i =
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
            data.serverID = _id;
            _multicasts.push_back( data );
        }

        connection->acceptNB();

        EQVERB << "Added node " << _id << " using " << connection << std::endl;
    }

    _state = STATE_LISTENING;

    EQVERB << typeid(*this).name() << " starting command and receiver thread "
           << std::endl;
    _receiverThread->start();

    EQINFO << this << " listening." << std::endl;
    return true;
}

bool Node::stopListening()
{
    if( _state != STATE_LISTENING )
        return false;

    NodeStopPacket packet;
    send( packet );

    EQCHECK( _receiverThread->join( ));
    _cleanup();

    EQINFO << _incoming.getSize() << " connections open after stopListening"
           << std::endl;
#ifndef NDEBUG
    const ConnectionVector& connections = _incoming.getConnections();
    for( ConnectionVector::const_iterator i = connections.begin();
         i != connections.end(); ++i )
    {
        EQINFO << "    " << *i << std::endl;
    }
#endif

    EQASSERT( _requestHandler.isEmpty( ));
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
    EQASSERTINFO( _state == STATE_STOPPED, _state );

    _multicasts.clear();
    _removeConnection( _outgoing );
    _connectionNodes.erase( _outgoing );
    _outMulticast.data = 0;
    _outgoing = 0;

    const ConnectionVector& connections = _incoming.getConnections();
    while( !connections.empty( ))
    {
        ConnectionPtr connection = connections.back();
        NodePtr       node       = _connectionNodes[ connection ];

        if( node.isValid( ))
        {
            node->_state = STATE_STOPPED;
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
    base::ScopedMutex mutex( _outMulticast );

    // Search if the connected node is in the same multicast group as we are
    for( ConnectionDescriptionVector::const_iterator i =
             _connectionDescriptions.begin();
         i != _connectionDescriptions.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        if( description->type < CONNECTIONTYPE_MULTICAST )
            continue;

        const ConnectionDescriptionVector& fromDescs = 
            node->getConnectionDescriptions();
        for( ConnectionDescriptionVector::const_iterator j = fromDescs.begin();
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
    base::ScopedMutex mutex( _nodes );
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
    packet.requestID = _requestHandler.registerRequest( node.get( ));
    send( packet );

    _requestHandler.waitRequest( packet.requestID );
    return true;
}

void Node::addConnectionDescription( ConnectionDescriptionPtr cd )
{
    _connectionDescriptions.push_back( cd ); 
}

bool Node::removeConnectionDescription( ConnectionDescriptionPtr cd )
{
    ConnectionDescriptionVector::iterator i = 
        std::find( _connectionDescriptions.begin(),
                   _connectionDescriptions.end(), cd );
    if( i == _connectionDescriptions.end( ))
        return false;

    _connectionDescriptions.erase( i );
    return true;
}

//----------------------------------------------------------------------
// Node functionality
//----------------------------------------------------------------------
void Node::_addSession( Session* session, NodePtr server,
                       const uint32_t sessionID )
{
    CHECK_THREAD( _thread );

    session->_server    = server;
    session->_id        = sessionID;
    session->_isMaster  = ( server==this && isLocal( ));
    session->_setLocalNode( this );
    if( session->_isMaster )
        session->_idPool.freeIDs( 1, base::IDPool::MAX_CAPACITY );

    EQASSERTINFO( _sessions.find( sessionID ) == _sessions.end(),
                  "Session " << sessionID << " @" << (void*)session
                  << " already mapped on " << this << " @"
                  <<  (void*)_sessions[ sessionID ] );
    _sessions[ sessionID ] = session;

    EQINFO << (session->_isMaster ? "master" : "client") << " session, id "
           << sessionID << ", served by " << server.get() << ", mapped on "
           << this << std::endl;
}

void Node::_removeSession( Session* session )
{
    CHECK_THREAD( _thread );
    EQASSERT( _sessions.find( session->getID( )) != _sessions.end( ));

    _sessions.erase( session->getID( ));
    EQINFO << "Erased session " << session->getID() << ", " << _sessions.size()
           << " left" << std::endl;

    session->_setLocalNode( 0 );
    session->_server    = 0;
    session->_id        = EQ_ID_INVALID;
    session->_isMaster  = false;
}

bool Node::registerSession( Session* session )
{
    EQASSERT( isLocal( ));
    EQASSERT( !inCommandThread( ));

    NodeRegisterSessionPacket packet;
    packet.requestID  = _requestHandler.registerRequest( session );
    send( packet );

    uint32_t sessionID = EQ_ID_INVALID;
    _requestHandler.waitRequest( packet.requestID, sessionID );
    return (sessionID == session->getID( ));
}

bool Node::mapSession( NodePtr server, Session* session, const uint32_t id )
{
    EQASSERT( isLocal( ));
    EQASSERT( id != EQ_ID_INVALID );
    EQASSERT( server != this );

    NodeMapSessionPacket packet;
    packet.requestID = _requestHandler.registerRequest( session );
    packet.sessionID = id;
    server->send( packet );

    uint32_t sessionID = EQ_ID_INVALID;
    _requestHandler.waitRequest( packet.requestID, sessionID );
    return (sessionID == session->getID( ));
}

bool Node::unmapSession( Session* session )
{
    EQASSERT( isLocal( ));

    NodeUnmapSessionPacket packet;
    packet.requestID = _requestHandler.registerRequest( session );
    packet.sessionID = session->getID();
    session->getServer()->send( packet );

    bool ret = false;
    _requestHandler.waitRequest( packet.requestID, ret );
    return ret;
}

Session* Node::getSession( const uint32_t id )
{
    SessionHash::const_iterator i = _sessions.find( id );
    
    EQASSERT( i != _sessions.end( ));
    if( i == _sessions.end( ))
        return 0;

    return i->second;
}

uint32_t Node::_generateSessionID()
{
    CHECK_THREAD( _thread );
    base::RNG rng;
    uint32_t  id  = rng.get<uint32_t>();

    while( id == EQ_ID_INVALID || _sessions.find( id ) != _sessions.end( ))
        id = rng.get<uint32_t>();

    return id;  
}

#define SEPARATOR '#'

std::string Node::serialize() const
{
    std::ostringstream data;
    data << _id << SEPARATOR << _launchCommand << SEPARATOR
         << static_cast<int>( _launchCommandQuote ) << SEPARATOR
         << _launchTimeout << SEPARATOR << _connectionDescriptions.size()
         << SEPARATOR;

    for( ConnectionDescriptionVector::const_iterator i = 
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
    EQASSERT( getState() == STATE_STOPPED || getState() == STATE_LAUNCHED );

    EQVERB << "Node data: " << data << std::endl;
    if( !_connectionDescriptions.empty( ))
        EQWARN << "Node already holds data while deserializing it" << std::endl;

    // node id
    size_t nextPos = data.find( SEPARATOR );
    if( nextPos == std::string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse node data" << std::endl;
        return false;
    }

    _id = data.substr( 0, nextPos );
    data = data.substr( nextPos + 1 );

    // launch command
    nextPos = data.find( SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse node data launch command" << std::endl;
        return false;
    }

    if( nextPos == 0 ) // empty launch command
        _launchCommand.clear();
    else
        _launchCommand = data.substr( 0, nextPos );
    data = data.substr( nextPos + 1 );

    // launch command quote
    nextPos = data.find( SEPARATOR );
    if( nextPos == std::string::npos )
        return false;
    const std::string quoteStr = data.substr( 0, nextPos );
    _launchCommandQuote = static_cast< char >( atoi( quoteStr.c_str( )));
    data               = data.substr( nextPos + 1 );

    // launch timeout
    nextPos = data.find( SEPARATOR );
    if( nextPos == std::string::npos )
        return false;
    
    const std::string launchTimeoutStr = data.substr( 0, nextPos );
    data                          = data.substr( nextPos + 1 );
    _launchTimeout = atoi( launchTimeoutStr.c_str( ));

    // num connection descriptions
    nextPos = data.find( SEPARATOR );
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
    EQASSERTINFO( type == TYPE_EQNET_NODE, type );
    return new Node();
}


void Node::acquireSendToken( NodePtr node )
{
    NodeAcquireSendTokenPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    node->send( packet );
    _requestHandler.waitRequest( packet.requestID );
}

void Node::releaseSendToken( NodePtr node )
{
    NodeReleaseSendTokenPacket packet;
    node->send( packet );
}

//----------------------------------------------------------------------
// Connecting and launching a node
//----------------------------------------------------------------------
bool Node::connect( NodePtr node )
{
    if( node->getState() == STATE_CONNECTED ||
        node->getState() == STATE_LISTENING )

        return true;

    if( !initConnect( node ))
    {
        EQERROR << "Connection initialization to " << node << " failed." 
                << std::endl;
        return false;
    }

    return syncConnect( node, node->_launchTimeout );
}

bool Node::initConnect( NodePtr node )
{
    EQASSERTINFO( _state == STATE_LISTENING, _state );
    if( node->getState() == STATE_CONNECTED ||
        node->getState() == STATE_LISTENING )
    {
        return true;
    }

    EQASSERT( node->getState() == STATE_STOPPED );

    // try connecting using the given descriptions
    const ConnectionDescriptionVector& cds = node->getConnectionDescriptions();
    EQASSERTINFO( !cds.empty(),
                  "Can't connect to a node without connection descriptions" );

    for( ConnectionDescriptionVector::const_iterator i = cds.begin();
        i != cds.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        if( description->type >= CONNECTIONTYPE_MULTICAST )
            continue; // Don't use multicast for primary connections

        ConnectionPtr connection = Connection::create( description );

        if( !connection->connect( ))
            continue;

        return _connect( node, connection );
    }

    EQINFO << "Node could not be connected." << std::endl;
    if( !node->_autoLaunch )
        return false;
    
    EQINFO << "Attempting to launch node." << std::endl;
    for( ConnectionDescriptionVector::const_iterator i = cds.begin();
        i != cds.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;

        if( _launch( node, description ))
            return true;
    }

    return false;
}

bool Node::_connect( NodePtr node, ConnectionPtr connection )
{
    EQASSERT( connection.isValid( ));

    if( !node.isValid() || _state != STATE_LISTENING ||
        !connection->isConnected() || node->_state != STATE_STOPPED )
    {
        return false;
    }

    _addConnection( connection );

    // send connect packet to peer
    NodeConnectPacket packet;
    packet.requestID = _requestHandler.registerRequest( node.get( ));
    packet.nodeID    = _id;
    packet.nodeID.convertToNetwork();
    packet.type      = getType();
    packet.launchID  = node->_launchID;
    node->_launchID  = EQ_ID_INVALID;
    connection->send( packet, serialize( ));

    bool connected = false;
    _requestHandler.waitRequest( packet.requestID, connected );
    if( !connected )
        return false;

    EQASSERT( node->_id != NodeID::ZERO );
    EQASSERTINFO( node->_id != _id, _id );
    EQINFO << node << " connected to " << this << std::endl;
    return true;
}

bool Node::syncConnect( NodePtr node, const uint32_t timeout )
{
    if( node->_launchID == EQ_ID_INVALID )
        return ( node->getState() == STATE_CONNECTED );

    void* ret;
    if( _requestHandler.waitRequest( node->_launchID, ret, timeout ))
    {
        EQASSERT( node->getState() == STATE_CONNECTED );
        node->_launchID = EQ_ID_INVALID;
        return true;
    }

    node->_state = STATE_STOPPED;
    _requestHandler.unregisterRequest( node->_launchID );
    node->_launchID = EQ_ID_INVALID;
    return false;
}

NodePtr Node::connect( const NodeID& nodeID )
{
    EQASSERT( nodeID != NodeID::ZERO );
    EQASSERT( _state == STATE_LISTENING );

    // Extract all node pointers, the _nodes hash will be modified later
    NodeVector nodes;
    {
        base::ScopedMutex mutex( _nodes );
        for( NodeHash::const_iterator i = _nodes->begin();
             i != _nodes->end(); ++i )
        {
            NodePtr node = i->second;

            if( node->getNodeID() == nodeID && node->isConnected( )) //early out
                return node;

            nodes.push_back( node );
        }
    }

    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
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
    base::ScopedMutex mutex( _connectMutex );
    {
        base::ScopedMutex mutexNodes( _nodes ); 
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
    packet.requestID = _requestHandler.registerRequest();
    packet.nodeID    = nodeID;
    packet.nodeID.convertToNetwork();

    server->send( packet );

    void* result = 0;
    _requestHandler.waitRequest( packet.requestID, result );

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
        base::ScopedMutex mutexNodes( _nodes );
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

bool Node::_launch( NodePtr node,
                    ConnectionDescriptionPtr description )
{
    EQASSERT( node->_autoLaunch );
    EQASSERT( node->getState() == STATE_STOPPED );

    node->_launchID = _requestHandler.registerRequest( node.get() );

    const std::string launchCommand = _createLaunchCommand( node, description );

    if( !base::Launcher::run( launchCommand ))
    {
        EQWARN << "Could not launch node using '" << launchCommand << "'" 
               << std::endl;
        _requestHandler.unregisterRequest( node->_launchID );
        node->_launchID = EQ_ID_INVALID;
        return false;
    }
    
    node->_state = STATE_LAUNCHED;
    return true;
}

std::string Node::_createLaunchCommand( NodePtr node,
                                   ConnectionDescriptionPtr description )
{
    const std::string& launchCommand = node->getLaunchCommand();
    const size_t  launchCommandLen   = launchCommand.size();
    const char    quote              = node->getLaunchCommandQuote();

    bool          commandFound = false;
    size_t        lastPos      = 0;
    std::string   result;

    for( size_t percentPos = launchCommand.find( '%' );
         percentPos != std::string::npos; 
         percentPos = launchCommand.find( '%', percentPos+1 ))
    {
        std::ostringstream replacement;
        switch( launchCommand[percentPos+1] )
        {
            case 'c':
            {
                replacement << _createRemoteCommand( node, quote );
                commandFound = true;
                break;
            }
            case 'h':
            {
                const std::string& hostname = description->getHostname();
                if( hostname.empty( ))
                    replacement << "localhost";
                else
                    replacement << hostname;
                break;
            }
            case 'n':
                replacement << node->getNodeID();
                break;

            default:
                EQWARN << "Unknown token " << launchCommand[percentPos+1] 
                       << std::endl;
        }

        result += launchCommand.substr( lastPos, percentPos-lastPos );
        if( !replacement.str().empty( ))
            result += replacement.str();

        lastPos  = percentPos+2;
    }

    result += launchCommand.substr( lastPos, launchCommandLen-lastPos );

    if( !commandFound )
        result += " " + _createRemoteCommand( node, quote );

    EQVERB << "Launch command: " << result << std::endl;
    return result;
}

std::string Node::_createRemoteCommand( NodePtr node, const char quote )
{
    if( getState() != STATE_LISTENING )
    {
        EQERROR << "Node is not listening, can't launch " << this << std::endl;
        return "";
    }

    std::ostringstream stringStream;

    //----- environment
#ifndef WIN32
#  ifdef Darwin
    const char libPath[] = "DYLD_LIBRARY_PATH";
#  else
    const char libPath[] = "LD_LIBRARY_PATH";
#  endif

    stringStream << "env "; // XXX
    char* env = getenv( libPath );
    if( env )
        stringStream << libPath << "=" << env << " ";

    for( int i=0; environ[i] != 0; i++ )
        if( strlen( environ[i] ) > 2 && strncmp( environ[i], "EQ_", 3 ) == 0 )
            stringStream << environ[i] << " ";

    stringStream << "EQ_LOG_LEVEL=" << base::Log::getLogLevelString() << " ";
    if( eq::base::Log::topics != 0 )
        stringStream << "EQ_LOG_TOPICS=" << base::Log::topics << " ";
#endif // WIN32

    //----- program + args
    std::string program = node->_programName;
#ifdef WIN32
    EQASSERT( program.length() > 2 );
    if( !( program[1] == ':' && (program[2] == '/' || program[2] == '\\' )) &&
        // !( drive letter and full path present )
        !( program[0] == '/' || program[0] == '\\' ))
        // !full path without drive letter

        program = node->_workDir + '/' + program; // add _workDir to rel. path
#else
    if( program[0] != '/' )
        program = node->_workDir + '/' + program;
#endif

    const std::string ownData    = serialize();
    const std::string remoteData = node->serialize();

    stringStream
        << quote << program << quote << " -- --eq-client " << quote
        << remoteData << node->_launchID << SEPARATOR << node->_workDir 
        << SEPARATOR << node->_id << SEPARATOR << getType() << SEPARATOR
        << ownData << quote;

    return stringStream.str();
}

bool Node::runClient( const std::string& clientArgs )
{
    EQASSERT( _state == STATE_LISTENING );

    size_t nextPos = clientArgs.find( SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse request identifier: " << clientArgs 
                << std::endl;
        return false;
    }

    const std::string   request     = clientArgs.substr( 0, nextPos );
    std::string         description = clientArgs.substr( nextPos + 1 );
    const uint32_t launchID    = strtoul( request.c_str(), 0, 10 );

    nextPos = description.find( SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse working directory: " << description 
                << " is left from " << clientArgs << std::endl;
        return false;
    }

    const std::string workDir = description.substr( 0, nextPos );
    description          = description.substr( nextPos + 1 );

    Global::setWorkDir( workDir );
    if( !workDir.empty() && chdir( workDir.c_str( )) == -1 )
        EQWARN << "Can't change working directory to " << workDir << ": "
               << strerror( errno ) << std::endl;
    
    EQVERB << "Launching node with launch ID=" << launchID << ", cwd="
           << workDir << std::endl;

    nextPos = description.find( SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse node identifier: " << description
                << " is left from " << clientArgs << std::endl;
        return false;
    }
    _id         = description.substr( 0, nextPos );
    description = description.substr( nextPos + 1 );

    nextPos = description.find( SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse server node type: " << description
                << " is left from " << clientArgs << std::endl;
        return false;
    }
    const std::string nodeType = description.substr( 0, nextPos );
    description = description.substr( nextPos + 1 );
    const uint32_t type = atoi( nodeType.c_str( ));

    NodePtr node = createNode( type );
    if( !node )
    {
        EQERROR << "Can't create server node" << std::endl;
        return false;
    }
    
    node->setAutoLaunch( false );
    node->_launchID = launchID;

    if( !node->deserialize( description ))
        EQWARN << "Can't parse node data" << std::endl;

    if( !connect( node ))
    {
        EQERROR << "Can't connect node" << std::endl;
        return false;
    }

    return clientLoop();
}

//----------------------------------------------------------------------
// receiver thread functions
//----------------------------------------------------------------------
void* Node::_runReceiverThread()
{
    EQINFO << "Entered receiver thread of " << typeid( *this ).name()
           << std::endl;

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

    EQINFO << "Leaving receiver thread of " << typeid( *this ).name() 
           << std::endl;
    return EXIT_SUCCESS;
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
    NodePtr node;
    ConnectionNodeHash::iterator i = _connectionNodes.find( connection );
    if( i != _connectionNodes.end( ))
        node = i->second;

    _connectionNodes.erase( i );
    if( node.isValid( ))
    {
        node->_state    = STATE_STOPPED;
        node->_outgoing = 0;

        base::ScopedMutex mutex( _nodes );
        _nodes->erase( node->_id );
    }

    _removeConnection( connection );

    EQINFO << node << " disconnected from " << this << " connection used " 
           << connection->getRefCount() << std::endl;
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
                  typeid( *node.get( )).name( ));

    EQVERB << "Handle data from " << node << std::endl;

    void* sizePtr( 0 );
    uint64_t bytes( 0 );
    const bool gotSize = connection->recvSync( &sizePtr, &bytes );

    if( !gotSize ) // Some systems signal data on dead connections.
    {
        connection->recvNB( sizePtr, sizeof( uint64_t ));
        return false;
    }

    EQASSERT( sizePtr );
    const uint64_t size = *reinterpret_cast< uint64_t* >( sizePtr );
    EQASSERT( size );
    EQASSERT( bytes == sizeof( uint64_t ));
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
                 ( command->datatype == DATATYPE_EQNET_NODE &&
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

    const uint32_t datatype = command->datatype;
    switch( datatype )
    {
        case DATATYPE_EQNET_NODE:
            return Dispatcher::dispatchCommand( command );

        case DATATYPE_EQNET_SESSION:
        case DATATYPE_EQNET_OBJECT:
        {
            const SessionPacket* sessionPacket = 
                static_cast<SessionPacket*>( command.getPacket( ));
            const uint32_t       id            = sessionPacket->sessionID;

            EQASSERTINFO( _sessions.find( id ) != _sessions.end(), 
                          "Can't find session for " << sessionPacket );
            Session*             session       = _sessions[ id ];
            EQASSERT( session );
            
            return session->dispatchCommand( command );
        }

        default:
            EQABORT( "Unknown datatype " << datatype << " for " << command );
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
void* Node::_runCommandThread()
{
    EQINFO << "Entered command thread of " << typeid( *this ).name() 
           << std::endl;

    while( _state == STATE_LISTENING )
    {
        Command* command = _commandThreadQueue.pop();
        EQASSERT( command->isValid( ));

        const CommandResult result  = invokeCommand( *command );
        switch( result )
        {
            case COMMAND_ERROR:
                EQABORT( "Error handling " << *command );
                break;

            case COMMAND_HANDLED:
            case COMMAND_DISCARD:
                break;

            default:
                EQUNIMPLEMENTED;
        }

        command->release();
    }
 
    _commandThreadQueue.flush();
    EQINFO << "Leaving command thread of " << typeid( *this ).name()
           << std::endl;
    return EXIT_SUCCESS;
}

CommandResult Node::invokeCommand( Command& command )
{
    EQVERB << "dispatch " << command << " by " << _id << std::endl;
    EQASSERT( command.isValid( ));

    const uint32_t datatype = command->datatype;
    switch( datatype )
    {
        case DATATYPE_EQNET_NODE:
            return Dispatcher::invokeCommand( command );

        case DATATYPE_EQNET_SESSION:
        case DATATYPE_EQNET_OBJECT:
        {
            const SessionPacket* sessionPacket = 
                static_cast<SessionPacket*>( command.getPacket( ));
            const uint32_t id = sessionPacket->sessionID;

            EQASSERTINFO( _sessions.find( id ) != _sessions.end( ),
                          "Can't find session for " << sessionPacket );

            Session* session = _sessions[ id ];
            if( !session )
                return COMMAND_ERROR;

            return session->invokeCommand( command );
        }

        default:
            EQABORT( "Unknown datatype " << datatype << " for " << command );
            return COMMAND_ERROR;
    }
}

CommandResult Node::_cmdStop( Command& command )
{
    EQINFO << "Cmd stop " << this << std::endl;
    EQASSERT( _state == STATE_LISTENING );

    _state = STATE_STOPPED;
    _incoming.interrupt();

    return COMMAND_HANDLED;
}

CommandResult Node::_cmdRegisterSession( Command& command )
{
    EQASSERT( getState() == STATE_LISTENING );

    const NodeRegisterSessionPacket* packet = 
        command.getPacket<NodeRegisterSessionPacket>();
    EQVERB << "Cmd register session: " << packet << std::endl;
    CHECK_THREAD( _thread );
    
    Session* session = static_cast< Session* >( 
        _requestHandler.getRequestData( packet->requestID ));

    EQASSERTINFO( command.getNode() == this, 
                  command.getNode() << " != " << this );
    EQASSERT( session );

    const uint32_t sessionID = _generateSessionID();
    _addSession( session, this, sessionID );

    NodeRegisterSessionReplyPacket reply( packet );
    reply.sessionID = session->getID();
    send( reply );
        
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdRegisterSessionReply( Command& command)
{
    const NodeRegisterSessionReplyPacket* packet = 
        command.getPacket<NodeRegisterSessionReplyPacket>();
    EQVERB << "Cmd register session reply: " << packet << std::endl;
    CHECK_THREAD( _thread );

    EQASSERT( command.getNode() == this );

    _requestHandler.serveRequest( packet->requestID, packet->sessionID );
    return COMMAND_HANDLED;
}


CommandResult Node::_cmdMapSession( Command& command )
{
    EQASSERT( getState() == STATE_LISTENING );

    const NodeMapSessionPacket* packet = 
        command.getPacket<NodeMapSessionPacket>();
    EQVERB << "Cmd map session: " << packet << std::endl;
    CHECK_THREAD( _thread );
    
    NodePtr node = command.getNode();
    NodeMapSessionReplyPacket reply( packet );

    if( node == this )
    {
        EQASSERTINFO( node == this, 
                      "Can't map a session using myself as server " );
        reply.sessionID = EQ_ID_INVALID;
    }
    else
    {
        const uint32_t sessionID = packet->sessionID;
        SessionHash::const_iterator i = _sessions.find( sessionID );
        
        if( i == _sessions.end( ))
            reply.sessionID = EQ_ID_INVALID;
    }

    node->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdMapSessionReply( Command& command)
{
    const NodeMapSessionReplyPacket* packet = 
        command.getPacket<NodeMapSessionReplyPacket>();
    EQVERB << "Cmd map session reply: " << packet << std::endl;
    CHECK_THREAD( _thread );

    const uint32_t requestID = packet->requestID;
    if( packet->sessionID != EQ_ID_INVALID )
    {
        NodePtr  node    = command.getNode(); 
        Session* session = static_cast< Session* >( 
            _requestHandler.getRequestData( requestID ));
        EQASSERT( session );
        EQASSERT( node != this );

        _addSession( session, node, packet->sessionID );
    }

    _requestHandler.serveRequest( requestID, packet->sessionID );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdUnmapSession( Command& command )
{
    const NodeUnmapSessionPacket* packet =
        command.getPacket<NodeUnmapSessionPacket>();
    EQVERB << "Cmd unmap session: " << packet << std::endl;
    CHECK_THREAD( _thread );
    
    const uint32_t sessionID = packet->sessionID;
    SessionHash::const_iterator i = _sessions.find( sessionID );
    Session* session = (i == _sessions.end() ? 0 : i->second );

    NodeUnmapSessionReplyPacket reply( packet );
    reply.result = (session != 0);

#if 0
    if( session && session->_server == this )
        ;// TODO: unmap all session slave instances.
#endif

    command.getNode()->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdUnmapSessionReply( Command& command)
{
    const NodeUnmapSessionReplyPacket* packet = 
        command.getPacket<NodeUnmapSessionReplyPacket>();
    EQVERB << "Cmd unmap session reply: " << packet << std::endl;
    CHECK_THREAD( _thread );

    const uint32_t requestID = packet->requestID;
    Session* session = static_cast< Session* >(
        _requestHandler.getRequestData( requestID ));
    EQASSERT( session );

    if( session )
    {
        _removeSession( session ); // TODO use session existence as return value
        _requestHandler.serveRequest( requestID, true );
    }
    else
        _requestHandler.serveRequest( requestID, false );

    // packet->result is false if server-side session was already unmapped
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdConnect( Command& command )
{
    EQASSERT( !command.getNode().isValid( ));
    EQASSERT( inReceiverThread( ));

    const NodeConnectPacket* packet = command.getPacket<NodeConnectPacket>();
    ConnectionPtr        connection = _incoming.getConnection();

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

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
            EQASSERT( packet->launchID == EQ_ID_INVALID );
            EQINFO << "Already got node " << nodeID << ", refusing connect"
                   << std::endl;

            // refuse connection
            NodeConnectReplyPacket reply( packet );
            connection->send( reply, serialize( ));

            // NOTE: There is no close() here. The reply packet above has to be
            // received by the peer first, before closing the connection.
            _removeConnection( connection );
            return COMMAND_HANDLED;
        }
    }

    if( !remoteNode )
    {
        // create and add connected node
        if( packet->launchID != EQ_ID_INVALID )
        {
            void* ptr = _requestHandler.getRequestData( packet->launchID );
            EQASSERT( dynamic_cast< Node* >( (Dispatcher*)ptr ));
            remoteNode = static_cast< Node* >( ptr );
        }
        else
            remoteNode = createNode( packet->type );
    }
    else
    {
        EQASSERT( packet->launchID == EQ_ID_INVALID );
    }
    remoteNode->_connectionDescriptions.clear(); //use data from peer

    std::string data = packet->nodeData;
    if( !remoteNode->deserialize( data ))
        EQWARN << "Error during node initialization" << std::endl;
    EQASSERTINFO( data.empty(), data);
    EQASSERTINFO( remoteNode->_id == nodeID,
                  remoteNode->_id << "!=" << nodeID );

    remoteNode->_outgoing = connection;
    remoteNode->_state    = STATE_CONNECTED;
    
    _connectionNodes[ connection ] = remoteNode;
    {
        base::ScopedMutex mutex( _nodes );
        _nodes.data[ remoteNode->_id ] = remoteNode;
    }
    EQVERB << "Added node " << nodeID << " using " << connection << std::endl;

    // send our information as reply
    NodeConnectReplyPacket reply( packet );
    reply.nodeID    = _id;
    reply.nodeID.convertToNetwork();
    reply.type      = getType();

    connection->send( reply, serialize( ));

    if( packet->launchID != EQ_ID_INVALID )
        _requestHandler.serveRequest( packet->launchID );
    
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdConnectReply( Command& command )
{
    EQASSERT( !command.getNode( ));
    EQASSERT( inReceiverThread( ));

    const NodeConnectReplyPacket* packet = 
        command.getPacket<NodeConnectReplyPacket>();
    ConnectionPtr connection = _incoming.getConnection();

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

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
            _requestHandler.serveRequest( packet->requestID, false );
        
        return COMMAND_HANDLED;
    }

    // create and add node
    if( !remoteNode )
    {
        if( packet->requestID != EQ_ID_INVALID )
        {
            void* ptr = _requestHandler.getRequestData( packet->requestID );
            EQASSERT( dynamic_cast< Node* >( (Dispatcher*)ptr ));
            remoteNode = static_cast< Node* >( ptr );
        }
        else
            remoteNode = createNode( packet->type );
    }
    else
    {
        EQASSERT( packet->requestID == EQ_ID_INVALID );
    }
    remoteNode->_connectionDescriptions.clear(); //use data from peer

    EQASSERT( remoteNode->getType() == packet->type );
    EQASSERT( remoteNode->getState() == STATE_STOPPED );

    std::string data = packet->nodeData;
    if( !remoteNode->deserialize( data ))
        EQWARN << "Error during node initialization" << std::endl;
    EQASSERT( data.empty( ));
    EQASSERT( remoteNode->_id == nodeID );

    remoteNode->_outgoing = connection;
    remoteNode->_state    = STATE_CONNECTED;
    
    _connectionNodes[ connection ] = remoteNode;
    {
        base::ScopedMutex mutex( _nodes );
        _nodes.data[ remoteNode->_id ] = remoteNode;
    }
    EQVERB << "Added node " << nodeID << " using " << connection << std::endl;

    if( packet->requestID != EQ_ID_INVALID )
        _requestHandler.serveRequest( packet->requestID, true );

    NodeConnectAckPacket ack;
    remoteNode->send( ack );

    _connectMulticast( remoteNode );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdConnectAck( Command& command )
{
    NodePtr node = command.getNode();
    EQASSERT( node.isValid( ));
    EQASSERT( inReceiverThread( ));
    EQVERB << "handle connect ack" << std::endl;
    
    _connectMulticast( node );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdID( Command& command )
{
    EQASSERT( inReceiverThread( ));

    const NodeIDPacket* packet = command.getPacket< NodeIDPacket >();
    NodeID nodeID = packet->id;
    nodeID.convertToHost();

    if( command.getNode().isValid( ))
    {
        EQASSERT( nodeID == command.getNode()->getNodeID( ));
        EQASSERT( command.getNode()->_outMulticast->isValid( ));
        return COMMAND_HANDLED;
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
            node = createNode( packet->type );
            std::string data = packet->data;

            if( !node->deserialize( data ))
                EQWARN << "Error during node initialization" << std::endl;
            EQASSERTINFO( data.empty(), data );

            {
                base::ScopedMutex nodesMutex( _nodes );
                _nodes.data[ nodeID ] = node;
            }
            EQVERB << "Added node " << nodeID << " with multicast "
                   << connection << std::endl;
        }
        else
        {
            node = i->second;
            EQASSERT( node->isConnected( ));
        }
    }
    EQASSERT( node.isValid( ));
    EQASSERTINFO( node->_id == nodeID, node->_id << "!=" << nodeID );

    base::ScopedMutex mutex( _outMulticast );
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
                data.serverID = _id;
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
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdDisconnect( Command& command )
{
    EQASSERT( inReceiverThread( ));

    const NodeDisconnectPacket* packet =
        command.getPacket<NodeDisconnectPacket>();

    NodePtr node = static_cast<Node*>( 
        _requestHandler.getRequestData( packet->requestID ));
    EQASSERT( node.isValid( ));

    ConnectionPtr connection = node->_outgoing;
    if( connection.isValid( ))
    {
        node->_state    = STATE_STOPPED;
        node->_outgoing = 0;

        _removeConnection( connection );
        EQASSERT( _connectionNodes.find( connection )!=_connectionNodes.end( ));

        _connectionNodes.erase( connection );
        {
            base::ScopedMutex mutex( _nodes );
            _nodes->erase( node->_id );
        }

        EQINFO << node << " disconnected from " << this << " connection used " 
               << connection->getRefCount() << std::endl;
    }

    EQASSERT( node->_state == STATE_STOPPED );
    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetNodeData( Command& command)
{
    const NodeGetNodeDataPacket* packet = 
        command.getPacket<NodeGetNodeDataPacket>();
    EQVERB << "cmd get node data: " << packet << std::endl;

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

    NodePtr node = getNode( nodeID );
    NodeGetNodeDataReplyPacket reply( packet );

    std::string nodeData;
    if( node.isValid( ))
    {
        reply.type = node->getType();
        nodeData   = node->serialize();
    }
    else
    {
        EQVERB << "Node " << nodeID << " unknown" << std::endl;
        reply.type = TYPE_EQNET_INVALID;
    }

    NodePtr toNode = command.getNode();
    toNode->send( reply, nodeData );
    EQINFO << "Sent node data " << nodeData << " to " << toNode << std::endl;
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetNodeDataReply( Command& command )
{
    NodeGetNodeDataReplyPacket* packet = 
        command.getPacket<NodeGetNodeDataReplyPacket>();
    EQVERB << "cmd get node data reply: " << packet << std::endl;

    const uint32_t requestID = packet->requestID;
    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();
    {
        base::ScopedMutex mutex( _nodes );
        NodeHash::const_iterator i = _nodes->find( nodeID );
        if( i != _nodes->end( ))
        {
            // Requested node connected to us in the meantime
            NodePtr node = i->second;

            node.ref();
            _requestHandler.serveRequest( requestID, node.get( ));
            return COMMAND_HANDLED;
        }
    }

    if( packet->type == TYPE_EQNET_INVALID )
    {
        _requestHandler.serveRequest( requestID, (void*)0 );
        return COMMAND_HANDLED;
    }
        
    NodePtr node = createNode( packet->type );
    EQASSERT( node.isValid( ));

    std::string data = packet->nodeData;
    if( !node->deserialize( data ))
        EQWARN << "Failed to initialize node data" << std::endl;
    EQASSERT( data.empty( ));

    node->setAutoLaunch( false );
    node.ref();
    _requestHandler.serveRequest( requestID, node.get( ));
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdAcquireSendToken( Command& command )
{
    NodeAcquireSendTokenPacket* packet = 
        command.getPacket<NodeAcquireSendTokenPacket>();

    if( !_hasSendToken ) // no token available
        // HACK: returning not COMMAND_HANDLED causes redispatch, see dispatcher
        return COMMAND_ERROR;

    _hasSendToken = false;

    NodeAcquireSendTokenReplyPacket reply( packet );
    command.getNode()->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdAcquireSendTokenReply( Command& command )
{
    NodeAcquireSendTokenReplyPacket* packet = 
        command.getPacket<NodeAcquireSendTokenReplyPacket>();

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdReleaseSendToken( Command& command )
{
    EQASSERT( !_hasSendToken );
    _hasSendToken = true;
    flushCommands(); // redispatch pending commands
    return COMMAND_HANDLED;
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Node::State state)
{
    os << ( state == Node::STATE_STOPPED ? "stopped" :
            state == Node::STATE_LAUNCHED ? "launched" :
            state == Node::STATE_CONNECTED ? "connected" :
            state == Node::STATE_LISTENING ? "listening" : "ERROR" );
    return os;
}

}
}
