
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

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
//----------------------------------------------------------------------
// State management
//----------------------------------------------------------------------
Node::Node()
        : _requestHandler( true )
        , _autoLaunch( false )
        , _id( true )
        , _state( STATE_STOPPED )
        , _launchID( EQ_ID_INVALID )
        , _programName( Global::getProgramName( ))
        , _workDir( Global::getWorkDir( ))
        , _hasSendToken( true )
{
    _receiverThread = new ReceiverThread( this );
    _commandThread  = new CommandThread( this );

    registerCommand( CMD_NODE_STOP, 
                     CommandFunc<Node>( this, &Node::_cmdStop ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_REGISTER_SESSION, 
                     CommandFunc<Node>( this, &Node::_cmdRegisterSession ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_REGISTER_SESSION_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdRegisterSessionReply ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_MAP_SESSION, 
                     CommandFunc<Node>( this, &Node::_cmdMapSession ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_MAP_SESSION_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdMapSessionReply ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_UNMAP_SESSION, 
                     CommandFunc<Node>( this, &Node::_cmdUnmapSession ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_UNMAP_SESSION_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdUnmapSessionReply ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_CONNECT,
                     CommandFunc<Node>( this, &Node::_cmdConnect ), 0 );
    registerCommand( CMD_NODE_CONNECT_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdConnectReply ), 0 );
    registerCommand( CMD_NODE_DISCONNECT,
                     CommandFunc<Node>( this, &Node::_cmdDisconnect ), 0 );
    registerCommand( CMD_NODE_GET_NODE_DATA,
                     CommandFunc<Node>( this, &Node::_cmdGetNodeData),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_GET_NODE_DATA_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdGetNodeDataReply ),
                     &_commandThreadQueue );
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN,
                     CommandFunc<Node>( this, &Node::_cmdAcquireSendToken ), 0);
    registerCommand( CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY,
                     CommandFunc<Node>( this, &Node::_cmdAcquireSendTokenReply )
                     , 0 );
    registerCommand( CMD_NODE_RELEASE_SEND_TOKEN,
                     CommandFunc<Node>( this, &Node::_cmdReleaseSendToken ), 0);

    EQINFO << "New Node @" << (void*)this << " " << _id << endl;
}

Node::~Node()
{
    EQINFO << "Delete Node @" << (void*)this << " " << _id << endl;
    EQASSERT( _connection == 0 );
    EQASSERT( _connectionSet.empty( ));
    EQASSERT( _connectionNodes.empty( ));
    EQASSERT( _pendingCommands.empty( ));
    EQASSERT( _nodes.empty( ));
    EQASSERT( _requestHandler.isEmpty( ));

#ifndef NDEBUG
    if( !_sessions.empty( ))
    {
        EQINFO << _sessions.size() << " mapped sessions" << endl;
        
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

bool Node::initLocal( const int argc, char** argv )
{
#ifndef NDEBUG
    EQINFO << disableFlush << "args: ";
    for( int i=0; i<argc; i++ )
         EQINFO << argv[i] << ", ";
    EQINFO << endl << enableFlush;
#endif

    // We do not use getopt_long because it really does not work due to the
    // following aspects:
    // - reordering of arguments
    // - different behaviour of GNU and BSD implementations
    // - incomplete man pages
    bool   isClient   = false;
    bool   isResident = false;
    string clientOpts;

    for( int i=1; i<argc; ++i )
    {
        if( string( "--eq-listen" ) == argv[i] )
        {
            if( i<argc && argv[i+1][0] != '-' )
            {
                string                        data = argv[++i];
                ConnectionDescriptionPtr desc = new ConnectionDescription;
                desc->TCPIP.port = Global::getDefaultPort();

                if( desc->fromString( data ))
                {
                    addConnectionDescription( desc );
                    EQASSERTINFO( data.empty(), data );
                }
                else
                    EQWARN << "Ignoring listen option: " << argv[i] << endl;
            }
        }
        else if( string( "--eq-client" ) == argv[i] )
        {
            isClient = true;
            if( i < argc-1 && argv[i+1][0] != '-' ) // server-started client
            {
                clientOpts = argv[++i];

                if( !deserialize( clientOpts ))
                    EQWARN << "Failed to parse client listen port parameters"
                           << endl;
                EQASSERT( !clientOpts.empty( ));
            }
            else // resident render client
                isResident = true;
        }
    }
    
    if( _connectionDescriptions.empty( )) // add default listener
    {
        ConnectionDescriptionPtr connDesc = new ConnectionDescription;
        connDesc->type       = CONNECTIONTYPE_TCPIP;
        connDesc->TCPIP.port = Global::getDefaultPort();
        addConnectionDescription( connDesc );
    }

    EQINFO << "Listener data: " << serialize() << endl;

    if( !listen( ))
    {
        EQWARN << "Can't setup listener(s)" << endl; 
        return false;
    }
    
    if( isClient )
    {
        EQINFO << "Client node started from command line with option " 
               << clientOpts << endl;

        bool ret = (isResident ? clientLoop() : runClient( clientOpts ));

        EQINFO << "Exit node process " << getRefCount() << endl;
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
                   << endl;
            return false;
        }

        _connectionNodes[ connection ] = this;
        _connectionSet.addConnection( connection );
        EQVERB << "Added node " << _id << " using " << connection << std::endl;
        
        connection->acceptNB();
    }

    _state = STATE_LISTENING;

    EQVERB << typeid(*this).name() << " starting command and receiver thread "
           << endl;
    _receiverThread->start();

    EQINFO << this << " listening." << endl;
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

    EQINFO << _connectionSet.size() << " connections open after stopListening"
           << endl;
#ifndef NDEBUG
    const ConnectionVector& connections = _connectionSet.getConnections();
    for( ConnectionVector::const_iterator i = connections.begin();
         i != connections.end(); ++i )
    {
        EQINFO << "    " << *i << endl;
    }
#endif

    EQASSERT( _requestHandler.isEmpty( ));
    return true;
}

void Node::_addConnection( ConnectionPtr connection )
{
    _connectionSet.addConnection( connection );
    connection->recvNB( new uint64_t, sizeof( uint64_t ));
}

void Node::_removeConnection( ConnectionPtr connection )
{
    EQASSERT( connection.isValid( ));

    _connectionSet.removeConnection( connection );

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
    EQVERB << "Clean up stopped node" << endl;
    EQASSERTINFO( _state == STATE_STOPPED, _state );

    _removeConnection( _connection );
    _connectionNodes.erase( _connection );
    _connection = 0;

    const ConnectionVector& connections = _connectionSet.getConnections();
    while( !connections.empty( ))
    {
        ConnectionPtr connection = connections.back();
        NodePtr       node       = _connectionNodes[ connection ];

        node->_state      = STATE_STOPPED;
        node->_connection = 0;
        _connectionNodes.erase( connection );
        _nodes.erase( node->_id );

        _removeConnection( connection );
    }

    if( !_connectionNodes.empty( ))
        EQINFO << _connectionNodes.size() << " open connections during cleanup"
               << endl;
#ifndef NDEBUG
    for( ConnectionNodeHash::const_iterator i = _connectionNodes.begin();
         i != _connectionNodes.end(); ++i )
    {
        NodePtr node = i->second;
        EQINFO << "    " << i->first << " : " << node << endl;
        EQINFO << "    Node ref count " << node->getRefCount() - 1 
               << ' ' << node->_connection << ' ' << node->_state
               << ( node == this ? " self" : "" ) << endl;
    }
#endif

    _connectionNodes.clear();

    if( !_nodes.empty( ))
        EQINFO << _nodes.size() << " nodes connected during cleanup" << endl;

#ifndef NDEBUG
    for( base::UUIDHash< NodePtr >::const_iterator i = _nodes.begin();
         i != _nodes.end(); ++i )
    {
        NodePtr node = i->second;
        EQINFO << "    " << node << " ref count " << node->getRefCount() - 1 
               << ' ' << node->_connection << ' ' << node->_state
               << ( node == this ? " self" : "" ) << endl;
    }
#endif

    _nodes.clear();
}

bool Node::_connectSelf()
{
    // setup local connection to myself
    _connection = new PipeConnection;
    if( !_connection->connect())
    {
        EQERROR << "Could not create local connection to receiver thread."
                << endl;
        _connection = 0;
        return false;
    }

    // add to connection set
    EQASSERT( _connection->getDescription().isValid( )); 
    EQASSERT( _connectionNodes.find( _connection ) == _connectionNodes.end( ));

    _connectionNodes[ _connection ] = this;
    _nodes[ _id ] = this;

    _addConnection( _connection );

    EQVERB << "Added node " << _id << " using " << _connection << std::endl;
    return true;
}

bool Node::connect( NodePtr node, ConnectionPtr connection )
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
    EQINFO << node << " connected to " << this << endl;
    return true;
}

NodePtr Node::getNode( const NodeID& id ) const
{ 
    base::UUIDHash< NodePtr >::const_iterator iter = _nodes.find( id );
    if( iter == _nodes.end( ))
        return 0;
    return iter->second;
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
        session->_idPool.freeIDs( 1, IDPool::MAX_CAPACITY );

    EQASSERTINFO( _sessions.find( sessionID ) == _sessions.end(),
                  "Session " << sessionID << " @" << (void*)session
                  << " already mapped on " << this << " @"
                  <<  (void*)_sessions[ sessionID ] );
    _sessions[ sessionID ] = session;

    EQINFO << (session->_isMaster ? "master" : "client") << " session, id "
           << sessionID << ", served by " << server.get() << ", mapped on "
           << this << endl;
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
    RNG      rng;
    uint32_t id  = rng.get<uint32_t>();

    while( id == EQ_ID_INVALID || _sessions.find( id ) != _sessions.end( ))
        id = rng.get<uint32_t>();

    return id;  
}

#define SEPARATOR '#'

std::string Node::serialize() const
{
    ostringstream data;
    data << _id << SEPARATOR << _connectionDescriptions.size() << SEPARATOR;

    for( vector< ConnectionDescriptionPtr >::const_iterator i = 
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

    EQINFO << "Node data: " << data << endl;
    if( !_connectionDescriptions.empty( ))
        EQWARN << "Node already holds data while deserializing it" << endl;

    // node id
    size_t nextPos = data.find( SEPARATOR );
    if( nextPos == string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse node data" << endl;
        return false;
    }

    _id = data.substr( 0, nextPos );
    data = data.substr( nextPos + 1 );

    // num connection descriptions
    nextPos = data.find( SEPARATOR );
    if( nextPos == string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse node data" << endl;
        return false;
    }

    const string sizeStr = data.substr( 0, nextPos );
    if( !isdigit( sizeStr[0] ))
    {
        EQERROR << "Could not parse node data" << endl;
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
            EQERROR << "Error during node connection data parsing" << endl;
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
// receiver thread functions
//----------------------------------------------------------------------
void* Node::_runReceiverThread()
{
    EQINFO << "Entered receiver thread of " << typeid( *this ).name() << endl;
    _commandThread->start();

    int nErrors = 0;
    while( _state == STATE_LISTENING )
    {
        const ConnectionSet::Event result = _connectionSet.select();
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
            {
                _handleDisconnect();
                EQVERB << &_connectionSet << endl;
                break;
            } 

            case ConnectionSet::EVENT_TIMEOUT:   
                EQINFO << "select timeout" << endl;
                break;

            case ConnectionSet::EVENT_ERROR:      
                ++nErrors;
                EQWARN << "Connection signalled error during select" << endl;
                if( nErrors > 100 )
                {
                    EQWARN << "Too many errors in a row, capping connection" 
                           << endl;
                    _handleDisconnect();
                }
                break;

            case ConnectionSet::EVENT_SELECT_ERROR:      
                EQWARN << "Error during select" << endl;
                ++nErrors;
                if( nErrors > 10 )
                {
                    EQWARN << "Too many errors in a row" << endl;
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
               << " commands pending while leaving command thread" << endl;

    for( CommandList::const_iterator i = _pendingCommands.begin();
         i != _pendingCommands.end(); ++i )
    {
        Command* command = *i;
        command->release();
    }

    EQCHECK( _commandThread->join( ));
    _pendingCommands.clear();
    _commandCache.flush();

    EQINFO << "Leaving receiver thread of " << typeid( *this ).name() << endl;
    return EXIT_SUCCESS;
}

void Node::_handleConnect()
{
    ConnectionPtr connection = _connectionSet.getConnection();
    ConnectionPtr newConn    = connection->acceptSync();
    connection->acceptNB();

    if( !newConn )
    {
        EQINFO << "Received connect event, but accept() failed" << endl;
        return;
    }

    _addConnection( newConn );
    // Node will be created when receiving NodeConnectPacket from other side
}

void Node::_handleDisconnect()
{
    while( _handleData( )) ; // read remaining data off connection

    ConnectionPtr connection = _connectionSet.getConnection();
    NodePtr node;
    ConnectionNodeHash::const_iterator i = _connectionNodes.find( connection );
    if( i != _connectionNodes.end( ))
        node = i->second;

    if( node.isValid( ))
    {
        node->_state      = STATE_STOPPED;
        node->_connection = 0;
        _connectionNodes.erase( connection );
        _nodes.erase( node->_id );
    }

    _removeConnection( connection );

    EQINFO << node << " disconnected from " << this << " connection used " 
           << connection->getRefCount() << endl;
}

bool Node::_handleData()
{
    ConnectionPtr connection = _connectionSet.getConnection();
    EQASSERT( connection.isValid( ));

    NodePtr node;
    ConnectionNodeHash::const_iterator i = _connectionNodes.find( connection );
    if( i != _connectionNodes.end( ))
        node = i->second;
    EQASSERTINFO( !node || node->_connection == connection, 
                  typeid( *node.get( )).name( ));

    EQVERB << "Handle data from " << node << endl;

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
                    command->command == CMD_NODE_CONNECT_REPLY)),
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
    EQVERB << "dispatch " << command << " by " << _id << endl;
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
        EQVERB << _pendingCommands.size() << " undispatched commands" << endl;
    EQASSERT( _pendingCommands.size() < 1000 );
#endif
}

//----------------------------------------------------------------------
// command thread functions
//----------------------------------------------------------------------
void* Node::_runCommandThread()
{
    EQINFO << "Entered command thread of " << typeid( *this ).name() << endl;

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
    EQINFO << "Leaving command thread of " << typeid( *this ).name() << endl;
    return EXIT_SUCCESS;
}

CommandResult Node::invokeCommand( Command& command )
{
    EQVERB << "dispatch " << command << " by " << _id << endl;
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
    EQINFO << "Cmd stop " << this << endl;
    EQASSERT( _state == STATE_LISTENING );

    _state = STATE_STOPPED;
    _connectionSet.interrupt();

    return COMMAND_HANDLED;
}

CommandResult Node::_cmdRegisterSession( Command& command )
{
    EQASSERT( getState() == STATE_LISTENING );

    const NodeRegisterSessionPacket* packet = 
        command.getPacket<NodeRegisterSessionPacket>();
    EQVERB << "Cmd register session: " << packet << endl;
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
    EQVERB << "Cmd register session reply: " << packet << endl;
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
    EQVERB << "Cmd map session: " << packet << endl;
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
    EQVERB << "Cmd map session reply: " << packet << endl;
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
    EQVERB << "Cmd unmap session: " << packet << endl;
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
    EQVERB << "Cmd unmap session reply: " << packet << endl;
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
    ConnectionPtr        connection = _connectionSet.getConnection();

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

    EQINFO << "handle connect " << packet << endl;
    EQASSERT( nodeID != _id );
    EQASSERT( _connectionNodes.find( connection ) == _connectionNodes.end( ));

    if( _nodes.find( nodeID ) != _nodes.end( ))
    {   // Node exists, probably simultaneous connect from peer
        EQASSERT( packet->launchID == EQ_ID_INVALID );
        EQINFO << "Already got node " << nodeID << ", refusing connect"
               << endl;

        // refuse connection
        NodeConnectReplyPacket reply( packet );
        connection->send( reply, serialize( ));

        // NOTE: There used to be no close() here. If deadlocks occur, it is
        // likely that the reply packet above can't be received by the peer
        // because the connection is closed by us. In that case, do not close
        // the connection here. Take care to cancel the pending IO for the next
        // packet size, and to delete the memory for the packet size.
        _removeConnection( connection );
        return COMMAND_HANDLED;
    }

    // create and add connected node
    NodePtr remoteNode;
    if( packet->launchID != EQ_ID_INVALID )
    {
        void* ptr = _requestHandler.getRequestData( packet->launchID );
        EQASSERT( dynamic_cast< Node* >( (Dispatcher*)ptr ));
        remoteNode = static_cast< Node* >( ptr );
        remoteNode->_connectionDescriptions.clear(); //use actual data from peer
    }
    else
        remoteNode = createNode( packet->type );

    string data = packet->nodeData;
    if( !remoteNode->deserialize( data ))
        EQWARN << "Error during node initialization" << endl;
    EQASSERT( data.empty( ));
    EQASSERTINFO( remoteNode->_id == nodeID,
                  remoteNode->_id << "!=" << nodeID );

    remoteNode->_connection = connection;
    remoteNode->_state      = STATE_CONNECTED;
    
    _connectionNodes[ connection ] = remoteNode;
    _nodes[ remoteNode->_id ]      = remoteNode;
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
    ConnectionPtr connection = _connectionSet.getConnection();

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

    EQINFO << "handle connect reply " << packet << endl;
    EQASSERT( _connectionNodes.find( connection ) == _connectionNodes.end( ));

    if( nodeID == NodeID::ZERO ||               // connection refused
        _nodes.find( nodeID ) != _nodes.end( )) // Node exists, probably
                                                // simultaneous connect
    {
        EQINFO << "ignoring connect reply, node already connected" << endl;
        _removeConnection( connection );
        
        if( packet->requestID != EQ_ID_INVALID )
            _requestHandler.serveRequest( packet->requestID, false );
        
        return COMMAND_HANDLED;
    }

    // create and add node
    NodePtr remoteNode;
    if( packet->requestID != EQ_ID_INVALID )
    {
        void* ptr = _requestHandler.getRequestData( packet->requestID );
        EQASSERT( dynamic_cast< Node* >( (Dispatcher*)ptr ));
        remoteNode = static_cast< Node* >( ptr );
        remoteNode->_connectionDescriptions.clear(); //get actual data from peer
    }

    if( !remoteNode )
        remoteNode = createNode( packet->type );

    EQASSERT( remoteNode->getType() == packet->type );
    EQASSERT( remoteNode->getState() == STATE_STOPPED );

    string data = packet->nodeData;
    if( !remoteNode->deserialize( data ))
        EQWARN << "Error during node initialization" << endl;
    EQASSERT( data.empty( ));
    EQASSERT( remoteNode->_id == nodeID );

    remoteNode->_connection = connection;
    remoteNode->_state      = STATE_CONNECTED;
    
    _connectionNodes[ connection ] = remoteNode;
    _nodes[ remoteNode->_id ]      = remoteNode;
    EQVERB << "Added node " << nodeID << " using " << connection << std::endl;

    if( packet->requestID != EQ_ID_INVALID )
        _requestHandler.serveRequest( packet->requestID, true );
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

    ConnectionPtr connection = node->_connection;
    if( connection.isValid( ))
    {
        node->_state      = STATE_STOPPED;
        node->_connection = 0;

        _removeConnection( connection );
        EQASSERT( _connectionNodes.find( connection )!=_connectionNodes.end( ));

        _connectionNodes.erase( connection );
        _nodes.erase( node->_id );

        EQINFO << node << " disconnected from " << this << " connection used " 
               << connection->getRefCount() << endl;
    }

    EQASSERT( node->_state == STATE_STOPPED );
    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetNodeData( Command& command)
{
    const NodeGetNodeDataPacket* packet = 
        command.getPacket<NodeGetNodeDataPacket>();
    EQINFO << "cmd get node data: " << packet << endl;

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

    NodePtr descNode = getNode( nodeID );
    
    NodeGetNodeDataReplyPacket reply( packet );

    string nodeData;
    if( descNode.isValid( ))
    {
        reply.type = descNode->getType();
        nodeData   = descNode->serialize();
    }
    else
    {
        EQINFO << "Node " << nodeID << " unknown" << endl;
        reply.type = TYPE_EQNET_INVALID;
    }

    NodePtr node = command.getNode();
    node->send( reply, nodeData );
    EQINFO << "Sent node data " << nodeData << " to " << node << endl;
    return COMMAND_HANDLED;
}

CommandResult Node::_cmdGetNodeDataReply( Command& command )
{
    NodeGetNodeDataReplyPacket* packet = 
        command.getPacket<NodeGetNodeDataReplyPacket>();
    EQINFO << "cmd get node data reply: " << packet << endl;

    const uint32_t requestID = packet->requestID;

    NodeID nodeID = packet->nodeID;
    nodeID.convertToHost();

    if( _nodes.find( nodeID ) != _nodes.end( ))
    {   
        // Requested node connected to us in the meantime
        NodePtr node = _nodes[ nodeID ];
        EQASSERT( node->isConnected( ));

        node.ref();
        _requestHandler.serveRequest( requestID, node.get( ));
        return COMMAND_HANDLED;
    }

    if( packet->type == TYPE_EQNET_INVALID )
    {
        _requestHandler.serveRequest( requestID, (void*)0 );
        return COMMAND_HANDLED;
    }
        
    NodePtr node = createNode( packet->type );
    EQASSERT( node.isValid( ));

    string data = packet->nodeData;
    if( !node->deserialize( data ))
        EQWARN << "Failed to initialize node data" << endl;
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
        // HACK: returning not COMMAND_HANDLED causes redispatch, see base.cpp
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
                << endl;
        return false;
    }

    return syncConnect( node );
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

    // try connecting first
    const ConnectionDescriptionVector& cds = node->getConnectionDescriptions();
    for( ConnectionDescriptionVector::const_iterator i = cds.begin();
        i != cds.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;
        ConnectionPtr connection = Connection::create( description );

        if( !connection->connect( ))
            continue;

        if( !connect( node, connection ))
            return false;

        return true;
    }

    EQINFO << "Node could not be connected." << endl;
    if( !node->_autoLaunch )
        return false;
    
    EQINFO << "Attempting to launch node." << endl;
    for( ConnectionDescriptionVector::const_iterator i = cds.begin();
        i != cds.end(); ++i )
    {
        ConnectionDescriptionPtr description = *i;

        if( _launch( node, description ))
            return true;
    }

    return false;
}

bool Node::syncConnect( NodePtr node )
{
    if( node->_launchID == EQ_ID_INVALID )
        return ( node->getState() == STATE_CONNECTED );

    void*          ret;
    const float    time    = -( node->_launchTimeout.getTimef( )); 
    const uint32_t timeout = static_cast< uint32_t >((time > 0) ? time : 0);

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
    
    // Extract all node pointers - _nodes hash might be modified later
    NodeVector nodes;
    for( base::UUIDHash< NodePtr >::const_iterator i = _nodes.begin();
         i != _nodes.end(); ++i )
    {
        NodePtr node = i->second;

        if( node->getNodeID() == nodeID ) // early out
            return node;

        nodes.push_back( node );
    }

    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        NodePtr result = _connect( nodeID, *i );
        if( result.isValid( ))
            return result;
    }

    return 0;
}

NodePtr Node::_connect( const NodeID& nodeID, NodePtr server )
{
    EQASSERT( nodeID != NodeID::ZERO );

    base::UUIDHash< NodePtr >::const_iterator iter = _nodes.find( nodeID );
    if( iter != _nodes.end( ))
        return iter->second;

    // Make sure that only one connection request based on the node identifier
    // is pending at a given time. Otherwise a node with the same id might be
    // instantiated twice in _cmdGetNodeDataReply(). The alternative to this
    // mutex is to register connecting nodes with this local node, and handle
    // all cases correctly, which is far more complex. Node connections only
    // happen a lot during initialization, and are therefore not time-critical.
    ScopedMutex mutex( _connectMutex );
    EQINFO << "Connecting node " << nodeID << endl;

    iter = _nodes.find( nodeID );
    if( iter != _nodes.end( ))
        return iter->second;
 
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
        EQWARN << "Node not found on server" << endl;
        return 0;
    }

    EQASSERT( dynamic_cast< Node* >( (Dispatcher*)result ));
    NodePtr node = static_cast< Node* >( result );
    node.unref(); // ref'd before serveRequest()
    
    if( node->isConnected( ))
        return node;

    if( !connect( node ))
    {
        // connect failed - maybe simultaneous connect from peer?
        iter = _nodes.find( nodeID );
        if( iter != _nodes.end( ))
        {
            node = iter->second;
            EQASSERT( node->isConnected( ));
            return node;
        }
        
        EQWARN << "Node connection failed" << endl;
        return 0;
    }

    return node;
}

bool Node::_launch( NodePtr node,
                    ConnectionDescriptionPtr description )
{
    EQASSERT( node->getState() == STATE_STOPPED );

    node->_launchID = _requestHandler.registerRequest( node.get() );
    node->_launchTimeout.setAlarm( 
        static_cast< float >( description->launchTimeout ));

    const string launchCommand = _createLaunchCommand( node, description );

    if( !Launcher::run( launchCommand ))
    {
        EQWARN << "Could not launch node using '" << launchCommand << "'" 
               << endl;
        _requestHandler.unregisterRequest( node->_launchID );
        node->_launchID = EQ_ID_INVALID;
        return false;
    }
    
    node->_state = STATE_LAUNCHED;
    return true;
}

string Node::_createLaunchCommand( NodePtr node,
                                   ConnectionDescriptionPtr description )
{
    const string& launchCommand    = description->getLaunchCommand();
    const size_t  launchCommandLen = launchCommand.size();
    const char    quote            = description->launchCommandQuote;

    bool          commandFound     = false;
    size_t        lastPos          = 0;
    string        result;

    for( size_t percentPos = launchCommand.find( '%' );
         percentPos != string::npos; 
         percentPos = launchCommand.find( '%', percentPos+1 ))
    {
        ostringstream replacement;
        switch( launchCommand[percentPos+1] )
        {
            case 'c':
            {
                replacement << _createRemoteCommand( node, quote );
                commandFound = true;
                break;
            }
            case 'h':
                replacement << description->getHostname();
                break;

            case 'n':
                replacement << node->getNodeID();
                break;

            default:
                EQWARN << "Unknown token " << launchCommand[percentPos+1] 
                       << endl;
        }

        result += launchCommand.substr( lastPos, percentPos-lastPos );
        if( !replacement.str().empty( ))
            result += replacement.str();

        lastPos  = percentPos+2;
    }

    result += launchCommand.substr( lastPos, launchCommandLen-lastPos );

    if( !commandFound )
        result += " " + _createRemoteCommand( node, quote );

    EQINFO << "Launch command: " << result << endl;
    return result;
}

string Node::_createRemoteCommand( NodePtr node, const char quote )
{
    if( getState() != STATE_LISTENING )
    {
        EQERROR << "Node is not listening, can't launch " << this << endl;
        return "";
    }

    ostringstream stringStream;

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

    stringStream << "EQ_LOG_LEVEL=" << Log::getLogLevelString() << " ";
    if( eq::base::Log::topics != 0 )
        stringStream << "EQ_LOG_TOPICS=" << Log::topics << " ";
#endif // WIN32

    //----- program + args
    string program = node->_programName;
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

    const string ownData    = serialize();
    const string remoteData = node->serialize();

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
    if( nextPos == string::npos )
    {
        EQERROR << "Could not parse request identifier: " << clientArgs << endl;
        return false;
    }

    const string   request     = clientArgs.substr( 0, nextPos );
    string         description = clientArgs.substr( nextPos + 1 );
    const uint32_t launchID    = strtoul( request.c_str(), 0, 10 );

    nextPos = description.find( SEPARATOR );
    if( nextPos == string::npos )
    {
        EQERROR << "Could not parse working directory: " << description 
                << " is left from " << clientArgs << endl;
        return false;
    }

    const string workDir = description.substr( 0, nextPos );
    description          = description.substr( nextPos + 1 );

    Global::setWorkDir( workDir );
    if( !workDir.empty() && chdir( workDir.c_str( )) == -1 )
        EQWARN << "Can't change working directory to " << workDir << ": "
               << strerror( errno ) << endl;
    
    EQINFO << "Launching node with launch ID=" << launchID << ", cwd="
           << workDir << endl;

    nextPos = description.find( SEPARATOR );
    if( nextPos == string::npos )
    {
        EQERROR << "Could not parse node identifier: " << description
                << " is left from " << clientArgs << endl;
        return false;
    }
    _id         = description.substr( 0, nextPos );
    description = description.substr( nextPos + 1 );

    nextPos = description.find( SEPARATOR );
    if( nextPos == string::npos )
    {
        EQERROR << "Could not parse server node type: " << description
                << " is left from " << clientArgs << endl;
        return false;
    }
    const string nodeType = description.substr( 0, nextPos );
    description           = description.substr( nextPos + 1 );
    const uint32_t   type = atoi( nodeType.c_str( ));

    RefPtr< Node >   node = createNode( type );
    if( !node )
    {
        EQERROR << "Can't create server node" << endl;
        return false;
    }
    
    node->setAutoLaunch( false );
    node->_launchID = launchID;

    if( !node->deserialize( description ))
        EQWARN << "Can't parse node data" << endl;

    if( !connect( node ))
    {
        EQERROR << "Can't connect node" << endl;
        return false;
    }

    return clientLoop();
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
