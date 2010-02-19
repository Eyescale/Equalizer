
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

#include "session.h"

#include "barrier.h"
#include "command.h"
#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "log.h"
#include "objectCM.h"
#include "objectInstanceDataIStream.h"
#include "packets.h"
#include "session.h"

#ifndef WIN32
#  include <alloca.h>
#endif

#include <limits>

namespace eq
{
namespace net
{
#define MIN_ID_RANGE 1024

typedef CommandFunc<Session> CmdFunc;

Session::Session()
        : _id( SessionID::ZERO )
        , _isMaster( false )
        , _idPool( 0 ) // Master pool is filled in Node::registerSession
        , _instanceIDs( std::numeric_limits< long >::min( )) 
        , _instanceCache( Global::getIAttribute( 
                              Global::IATTR_INSTANCE_CACHE_SIZE ) * EQ_1MB )
{
    EQINFO << "New Session @" << (void*)this << std::endl;
}

Session::~Session()
{
    EQINFO << "Delete Session @" << (void*)this << std::endl;
    EQASSERTINFO( _id == SessionID::ZERO,
                  "Session still mapped during deletion");

    _id        = SessionID::ZERO;
    _isMaster  = false;
    _localNode = 0;
    _server    = 0;
    
#ifndef NDEBUG
    if( !_objects->empty( ))
    {
        EQWARN << _objects->size() << " attached objects in destructor"
               << std::endl;
        
        for( ObjectVectorHash::const_iterator i = _objects->begin();
             i != _objects->end(); ++i )
        {
            const ObjectVector& objects = i->second;
            EQWARN << "  " << objects.size() << " objects with id " 
                   << i->first << std::endl;
            
            for( ObjectVector::const_iterator j = objects.begin();
                 j != objects.end(); ++j )
            {
                const Object* object = *j;
                EQINFO << "    object type " << typeid(*object).name() 
                       << std::endl;
            }
        }
    }
#endif
    _objects->clear();
}

void Session::_setLocalNode( NodePtr node )
{
    _localNode = node;
    if( !_localNode )
        return; // TODO deregister command functions?

    notifyMapped( node );
}

CommandQueue* Session::getCommandThreadQueue()
{
    EQASSERT( _localNode.isValid( ));
    if( !_localNode )
        return 0;

    return _localNode->getCommandThreadQueue();
}

void Session::notifyMapped( NodePtr node )
{
    EQASSERT( node.isValid( ));

    CommandQueue* queue = node->getCommandThreadQueue();

    registerCommand( CMD_SESSION_ACK_REQUEST, 
                     CmdFunc( this, &Session::_cmdAckRequest ), 0 );
    registerCommand( CMD_SESSION_GEN_IDS, 
                     CmdFunc( this, &Session::_cmdGenIDs ), queue );
    registerCommand( CMD_SESSION_GEN_IDS_REPLY,
                     CmdFunc( this, &Session::_cmdGenIDsReply ), queue );
    registerCommand( CMD_SESSION_SET_ID_MASTER,
                     CmdFunc( this, &Session::_cmdSetIDMaster ), queue );
    registerCommand( CMD_SESSION_GET_ID_MASTER, 
                     CmdFunc( this, &Session::_cmdGetIDMaster ), queue );
    registerCommand( CMD_SESSION_GET_ID_MASTER_REPLY,
                     CmdFunc( this, &Session::_cmdGetIDMasterReply ), queue );
    registerCommand( CMD_SESSION_ATTACH_OBJECT,
                     CmdFunc( this, &Session::_cmdAttachObject ), 0 );
    registerCommand( CMD_SESSION_DETACH_OBJECT,
                     CmdFunc( this, &Session::_cmdDetachObject ), 0 );
    registerCommand( CMD_SESSION_MAP_OBJECT,
                     CmdFunc( this, &Session::_cmdMapObject ), queue );
    registerCommand( CMD_SESSION_UNMAP_OBJECT,
                     CmdFunc( this, &Session::_cmdUnmapObject ), 0 );
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT,
                     CmdFunc( this, &Session::_cmdSubscribeObject ), queue );
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS,
                     CmdFunc( this, &Session::_cmdSubscribeObjectSuccess ), 0 );
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT_REPLY,
                     CmdFunc( this, &Session::_cmdSubscribeObjectReply ),
                     queue );
    registerCommand( CMD_SESSION_UNSUBSCRIBE_OBJECT,
                     CmdFunc( this, &Session::_cmdUnsubscribeObject ), queue );
    registerCommand( CMD_SESSION_INSTANCE,
                     CmdFunc( this, &Session::_cmdInstance ), queue );
}

//---------------------------------------------------------------------------
// identifier generation
//---------------------------------------------------------------------------
uint32_t Session::genIDs( const uint32_t range )
{
    // TODO: retrieve ID's from non-master nodes?

    uint32_t id = _idPool.genIDs( range );
    if( id != EQ_ID_INVALID || _isMaster )
    {
        if( id == EQ_ID_INVALID )
            EQWARN << "Out of session identifiers" << std::endl;
        return id;
    }

    SessionGenIDsPacket packet;
    packet.requestID = _localNode->registerRequest();
    packet.range     = range;

    send( packet );
    _localNode->waitRequest( packet.requestID, id );
    
    if( id == EQ_ID_INVALID )
        EQWARN << "Out of session identifiers" << std::endl;

    return id;
}

void Session::freeIDs( const uint32_t start, const uint32_t range )
{
    _idPool.freeIDs( start, range );
}

//---------------------------------------------------------------------------
// identifier master node mapping
//---------------------------------------------------------------------------
void Session::setIDMaster( const uint32_t id, const NodeID& master )
{
    CHECK_NOT_THREAD( _commandThread );
    _setIDMasterSync( _setIDMasterNB( id, master ));
}

uint32_t Session::_setIDMasterNB( const uint32_t identifier,
                                  const NodeID& master )
{
    CHECK_NOT_THREAD( _commandThread );

    SessionSetIDMasterPacket packet;
    packet.identifier = identifier;
    packet.masterID = master;
    
    if( !_isMaster )
        _sendLocal( packet ); // set on our slave instance (fire&forget)

    packet.requestID = _localNode->registerRequest();
    send( packet );       // set on master instance (need to wait for ack)
    return packet.requestID;
}

void Session::_setIDMasterSync( const uint32_t requestID )
{
    _localNode->waitRequest( requestID );
}

const NodeID& Session::_pollIDMaster( const uint32_t id ) const 
{
    NodeIDHash::const_iterator i = _idMasters.find( id );
    if( i == _idMasters.end( ))
        return NodeID::ZERO;

    return i->second;
}

const NodeID& Session::getIDMaster( const uint32_t identifier )
{
    _idMasterMutex.set();
    const NodeID& master = _pollIDMaster( identifier );
    _idMasterMutex.unset();
        
    if( master != NodeID::ZERO || _isMaster )
        return master;

    // ask session master instance
    SessionGetIDMasterPacket packet;
    packet.requestID = _localNode->registerRequest();
    packet.identifier = identifier;

    send( packet );
    _localNode->waitRequest( packet.requestID );

    base::ScopedMutex<> mutex( _idMasterMutex );
    EQLOG( LOG_OBJECTS ) << "Master node for id " << identifier << ": " 
        << _pollIDMaster( identifier ) << std::endl;
    return _pollIDMaster( identifier );
}

//---------------------------------------------------------------------------
// object mapping
//---------------------------------------------------------------------------
void Session::attachObject( Object* object, const uint32_t id, 
                            const uint32_t instanceID )
{
    EQASSERT( object );
    CHECK_NOT_THREAD( _receiverThread );

    SessionAttachObjectPacket packet;
    packet.objectID = id;
    packet.objectInstanceID = instanceID;
    packet.requestID = _localNode->registerRequest( object );

    _sendLocal( packet );
    _localNode->waitRequest( packet.requestID );
}

namespace
{
uint32_t _genNextID( base::a_int32_t& val )
{
    uint32_t result;
    do
    {
        const long id = ++val;
        result = static_cast< uint32_t >(
            static_cast< int64_t >( id ) + 0x7FFFFFFFu );
    }
    while( result > EQ_ID_MAX );

    return result;
}
}

void Session::_attachObject( Object* object, const uint32_t id, 
                             const uint32_t inInstanceID )
{
    EQASSERT( object );
    CHECK_THREAD( _receiverThread );

    uint32_t instanceID = inInstanceID;
    if( inInstanceID == EQ_ID_INVALID )
        instanceID = _genNextID( _instanceIDs );

    object->attachToSession( id, instanceID, this );

    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        ObjectVector& objects = _objects.data[ id ];
        objects.push_back( object );
    }

    getLocalNode()->flushCommands(); // redispatch pending commands

    EQLOG( LOG_OBJECTS ) << "attached " << typeid( *object ).name() << " id "
                         << object->getID() << '.' << object->getInstanceID()
                         << " cm " << typeid( *(object->_cm)).name() << " @" 
                         << static_cast< void* >( object ) << std::endl;
}

void Session::detachObject( Object* object )
{
    EQASSERT( object );
    CHECK_NOT_THREAD( _receiverThread );

    SessionDetachObjectPacket packet;
    packet.requestID = _localNode->registerRequest();
    packet.objectID  = object->getID();
    packet.objectInstanceID  = object->getInstanceID();

    _sendLocal( packet );
    _localNode->waitRequest( packet.requestID );
}

void Session::_detachObject( Object* object )
{
    // check also _cmdUnmapObject when modifying!
    EQASSERT( object );
    CHECK_THREAD( _receiverThread );

    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID )
        return;

    EQASSERT( _objects->find( id ) != _objects->end( ));
    EQLOG( LOG_OBJECTS ) << "Detach " << typeid( *object ).name() 
                         << " from id " << id << std::endl;

    ObjectVector& objects = _objects.data[ id ];
    ObjectVector::iterator i = find( objects.begin(),objects.end(), object );
    EQASSERT( i != objects.end( ));

    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        objects.erase( i );
        if( objects.empty( ))
            _objects->erase( id );
    }

    EQASSERT( object->getInstanceID() != EQ_ID_INVALID );
    object->detachFromSession();
    return;
}

bool Session::mapObject( Object* object, const uint32_t id,
                         const uint32_t version )
{
    const uint32_t requestID = mapObjectNB( object, id, version );
    return mapObjectSync( requestID );
}

uint32_t Session::mapObjectNB( Object* object, const uint32_t id,
                               const uint32_t version )
{
    CHECK_NOT_THREAD( _commandThread );
    EQASSERT( object );
    EQLOG( LOG_OBJECTS ) << "Mapping " << typeid( *object ).name() << " to id "
                         << id << " version " << version << std::endl;

    EQASSERT( object->getID() == EQ_ID_INVALID );
    EQASSERT( !object->isMaster( ));
    EQASSERT( id != EQ_ID_INVALID );
    EQASSERT( !_localNode->inCommandThread( ));
        
    // Connect master node, can't do that from the command thread!
    NodeID masterNodeID = getIDMaster( id );
    if( masterNodeID == NodeID::ZERO )
    {
        EQWARN << "Can't find master node for object id " << id <<std::endl;
        return EQ_ID_INVALID;
    }

    NodePtr master = _localNode->connect( masterNodeID );
    if( !master || master->getState() == Node::STATE_STOPPED )
    {
        EQWARN << "Can't connect master node with id " << masterNodeID
               << " for object id " << id << std::endl;
        return EQ_ID_INVALID;
    }

    SessionMapObjectPacket packet;
    packet.requestID    = _localNode->registerRequest( object );
    packet.objectID     = id;
    packet.version      = version;
    packet.masterNodeID = masterNodeID;

    _sendLocal( packet );
    return packet.requestID;
}

bool Session::mapObjectSync( const uint32_t requestID )
{
    if( requestID == EQ_ID_INVALID )
        return false;

    void* data = _localNode->getRequestData( requestID );    
    if( data == 0 )
        return false;

    Object* object = EQSAFECAST( Object*, data );
    uint32_t version = VERSION_NONE;

    _localNode->waitRequest( requestID, version );

    const bool mapped = ( object->getID() != EQ_ID_INVALID );
    if( mapped )
    {
        object->_cm->applyMapData(); // apply instance data on slave instances
        if( version != VERSION_OLDEST && version != VERSION_NONE )
            object->sync( version );
    }

    EQLOG( LOG_OBJECTS ) << "Mapped " << typeid( *object ).name() << " to id " 
                         << object->getID() << std::endl;
    return mapped;
}

void Session::unmapObject( Object* object )
{
    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID ) // not registered
        return;

    EQLOG( LOG_OBJECTS ) << "Unmap " << typeid( *object ).name() << " from id "
        << object->getID() << std::endl;

    // send unsubscribe to master, master will send detach packet.
    EQASSERT( !object->isMaster( ));
    CHECK_NOT_THREAD( _commandThread );
    
    const uint32_t masterInstanceID = object->getMasterInstanceID();
    if( masterInstanceID != EQ_ID_INVALID )
    {
        _idMasterMutex.set();
        const NodeID& masterNodeID = _pollIDMaster( id );
        _idMasterMutex.unset();

        NodePtr localNode = _localNode;
        NodePtr master    = localNode.isValid() ? 
                                localNode->getNode( masterNodeID ) : 0;
        if( master.isValid() && master->isConnected( ))
        {
            SessionUnsubscribeObjectPacket packet;
            packet.requestID = _localNode->registerRequest();
            packet.objectID  = id;
            packet.masterInstanceID = masterInstanceID;
            packet.slaveInstanceID  = object->getInstanceID();
            send( master, packet );

            _localNode->waitRequest( packet.requestID );
            return;
        }
        EQERROR << "Master node for object id " << id << " not connected"
                << std::endl;
    }

    // no unsubscribe sent: Detach directly
    detachObject( object );
}

bool Session::registerObject( Object* object )
{
    EQASSERT( object );
    EQASSERT( object->getID() == EQ_ID_INVALID );

    const uint32_t id = genIDs( 1 );
    EQASSERT( id != EQ_ID_INVALID );
    if( id == EQ_ID_INVALID )
        return false;

    const uint32_t requestID = _setIDMasterNB( id, _localNode->getNodeID( ));
    object->setupChangeManager( object->getChangeType(), true );
    attachObject( object, id, EQ_ID_INVALID );

    _setIDMasterSync( requestID ); // sync, master knows our ID now
    EQLOG( LOG_OBJECTS ) << "Registered " << typeid( *object ).name()
                         << " to id " << id << std::endl;
    return true;
}

void Session::deregisterObject( Object* object )
{
    EQASSERT( object )
    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID ) // not registered
        return;

    EQLOG( LOG_OBJECTS ) << "Deregister " << typeid( *object ).name() 
                         << " from id " << id << std::endl;
    EQASSERT( object->isMaster( ));

    // unmap slaves
    const NodeVector* slaves = object->_getSlaveNodes();
    if( slaves && !slaves->empty( ))
    {
        SessionUnmapObjectPacket packet;
        packet.sessionID = _id;
        packet.objectID = id;

        for( NodeVector::const_iterator i = slaves->begin();
             i != slaves->end(); ++i )
        {
            NodePtr node = *i;
            node->send( packet );
        }
    }

    detachObject( object );
    object->_setChangeManager( ObjectCM::ZERO );

    // TODO unsetIDMaster ?
    freeIDs( id, 1 );
}

//===========================================================================
// Packet handling
//===========================================================================
bool Session::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << std::endl;
    EQASSERT( command.isValid( ));
    CHECK_THREAD( _receiverThread );

    switch( command->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            EQCHECK( Dispatcher::dispatchCommand( command ));
            return true;

        case DATATYPE_EQNET_OBJECT:
        {
            EQASSERT( command.isValid( ));
            const ObjectPacket* objPacket = command.getPacket<ObjectPacket>();
            const uint32_t      id        = objPacket->objectID;

            if( _objects->find( id ) == _objects->end( ))
                // When the instance ID is set to none, we only care about the
                // packet when we have an object of the given ID (multicast)
                return ( objPacket->instanceID == EQ_ID_NONE ? true : false );

            EQASSERTINFO( !_objects.data[id].empty(), id );

            Object* object = _objects.data[id].front();
            EQASSERT( object );

            EQCHECK( object->dispatchCommand( command ));
            return true;
        }

        default:
            EQABORT( "Unknown datatype " << command->datatype << " for "
                     << command );
            return true;
    }
}

CommandResult Session::invokeCommand( Command& command )
{
    EQVERB << "invoke " << command << std::endl;
    EQASSERT( command.isValid( ));

    switch( command->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            return Dispatcher::invokeCommand( command );

        case DATATYPE_EQNET_OBJECT:
            return _invokeObjectCommand( command );

        default:
            EQWARN << "Unhandled command " << command << std::endl;
            return COMMAND_ERROR;
    }
}

CommandResult Session::_invokeObjectCommand( Command& command )
{
    EQASSERT( command.isValid( ));
    EQASSERT( command->datatype == DATATYPE_EQNET_OBJECT );

    const ObjectPacket* objPacket = command.getPacket<ObjectPacket>();
    const uint32_t      id        = objPacket->objectID;
    const bool ignoreInstance = ( objPacket->instanceID == EQ_ID_ANY ||
                                  objPacket->instanceID == EQ_ID_NONE );

    _objects.lock.set();

    EQASSERTINFO( _objects->find( id ) != _objects->end(), 
                  "No objects to handle command " << objPacket );

    // create copy of objects vector for thread-safety
    ObjectVector objects = _objects.data[id];
    EQASSERTINFO( !objects.empty(), objPacket );

    _objects.lock.unset();

    for( ObjectVector::const_iterator i = objects.begin();
         i != objects.end(); ++i )
    {
        Object* object = *i;
        const bool isInstance = objPacket->instanceID ==object->getInstanceID();
        if( ignoreInstance || isInstance )
        {
            EQASSERT( command.isValid( ))

            const CommandResult result = object->invokeCommand( command );
            switch( result )
            {
                case COMMAND_DISCARD:
                    return COMMAND_DISCARD;

                case COMMAND_ERROR:
                    EQERROR << "Error handling command " << objPacket
                            << " for object of type " << typeid(*object).name()
                            << std::endl;
                    return COMMAND_ERROR;

                case COMMAND_HANDLED:
                    if( isInstance )
                        return result;
                    break;

                default:
                    EQUNREACHABLE;
            }
        }
    }
    if( ignoreInstance )
        return COMMAND_HANDLED;

    EQUNREACHABLE;
    EQWARN << "instance not found for " << objPacket << std::endl;
    return COMMAND_ERROR;
}

CommandResult Session::_cmdAckRequest( Command& command )
{
    const SessionAckRequestPacket* packet = 
        command.getPacket<SessionAckRequestPacket>();
    EQASSERT( packet->requestID != EQ_ID_INVALID );

    _localNode->serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}


CommandResult Session::_cmdGenIDs( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGenIDsPacket* packet =command.getPacket<SessionGenIDsPacket>();
    EQVERB << "Cmd gen IDs: " << packet << std::endl;

    SessionGenIDsReplyPacket reply( packet );
    const uint32_t range = EQ_MAX( packet->range, MIN_ID_RANGE );

    reply.firstID = _idPool.genIDs( range );
    reply.allocated = range;
    send( command.getNode(), reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGenIDsReplyPacket* packet =
        command.getPacket<SessionGenIDsReplyPacket>();
    EQVERB << "Cmd gen IDs reply: " << packet << std::endl;

    _localNode->serveRequest( packet->requestID, packet->firstID );

    const size_t additional = packet->allocated - packet->requested;
    if( packet->firstID != EQ_ID_INVALID && additional > 0 )
        // Merge additional identifiers into local pool
        _idPool.freeIDs( packet->firstID + packet->requested, additional );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionSetIDMasterPacket* packet = 
        command.getPacket<SessionSetIDMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd set ID master: " << packet << std::endl;

    const NodeID& nodeID = packet->masterID;
    EQASSERT( nodeID != NodeID::ZERO );

    base::ScopedMutex<> mutex( _idMasterMutex );
    _idMasters[ packet->identifier ] = nodeID;

    if( packet->requestID != EQ_ID_INVALID ) // need to ack set operation
    {
        NodePtr node = command.getNode();

        if( node == _localNode ) // OPT
            _localNode->serveRequest( packet->requestID );
        else
        {
            SessionAckRequestPacket reply( packet->requestID );
            send( node, reply );
        }
    }
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMaster( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGetIDMasterPacket* packet =
        command.getPacket<SessionGetIDMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "handle get idMaster " << packet << std::endl;

    SessionGetIDMasterReplyPacket reply( packet );
    reply.masterID = _pollIDMaster( packet->identifier );
    send( command.getNode(), reply );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMasterReply( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGetIDMasterReplyPacket* packet = 
        command.getPacket<SessionGetIDMasterReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "handle get idMaster reply " << packet << std::endl;

    const NodeID& nodeID = packet->masterID;

    if( nodeID != NodeID::ZERO )
    {
        base::ScopedMutex<> mutex( _idMasterMutex );
        _idMasters[ packet->identifier ] = nodeID;
    }
    // else not found

    _localNode->serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdAttachObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionAttachObjectPacket* packet = 
        command.getPacket<SessionAttachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd attach object " << packet << std::endl;

    Object* object =  static_cast<Object*>( _localNode->getRequestData( 
                                                packet->requestID ));
    _attachObject( object, packet->objectID, packet->objectInstanceID );
    _localNode->serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdDetachObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionDetachObjectPacket* packet = 
        command.getPacket<SessionDetachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd detach object " << packet << std::endl;

    const uint32_t id = packet->objectID;
    ObjectVectorHash::const_iterator i = _objects->find( id );
    if( i != _objects->end( ))
    {
        const ObjectVector& objects = i->second;

        for( ObjectVector::const_iterator j = objects.begin();
            j != objects.end(); ++j )
        {
            Object* object = *j;
            if( object->getInstanceID() == packet->objectInstanceID )
            {
                _detachObject( object );
                break;
            }
        }
    }

    _localNode->serveRequest( packet->requestID );    
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdMapObject( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionMapObjectPacket* packet = 
        command.getPacket< SessionMapObjectPacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd map object " << packet << std::endl;

#ifndef NDEBUG
    Object* object = static_cast<Object*>(
        _localNode->getRequestData( packet->requestID ));    

    EQASSERT( object );
    EQASSERT( !object->isMaster( ));
    EQASSERT( packet->masterNodeID != NodeID::ZERO );
#endif

    const uint32_t id = packet->objectID;
    NodePtr master = _localNode->getNode( packet->masterNodeID );

    EQASSERTINFO( master.isValid() && master->isConnected(),
                  "Master node for object id " << id << " not connected" );

    // slave instantiation - subscribe first
    SessionSubscribeObjectPacket subscribePacket( packet );
    subscribePacket.instanceID = _genNextID( _instanceIDs );

    const InstanceCache::Data& cached = _instanceCache[ id ];
    if( cached != InstanceCache::Data::NONE )
    {
        const InstanceDataDeque& versions = cached.versions;
        EQASSERT( !cached.versions.empty( ));
        subscribePacket.useCache = true;
        subscribePacket.masterInstanceID = cached.masterInstanceID;
        subscribePacket.minCachedVersion = versions.front()->getVersion();
        subscribePacket.maxCachedVersion = versions.back()->getVersion();
        EQLOG( LOG_OBJECTS ) << "Object " << id << " have v"
                             << subscribePacket.minCachedVersion << ".."
                             << subscribePacket.maxCachedVersion << std::endl;
    }

    send( master, subscribePacket );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObject( Command& command )
{
    CHECK_THREAD( _commandThread );
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd subscribe object " << packet << std::endl;

    NodePtr        node = command.getNode();
    const uint32_t id   = packet->objectID;

    Object* master = 0;
    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        ObjectVectorHash::const_iterator i = _objects->find( id );
        if( i != _objects->end( ))
        {
            const ObjectVector& objects = i->second;

            for( ObjectVector::const_iterator j = objects.begin();
                 j != objects.end(); ++j )
            {
                Object* object = *j;
                if( object->isMaster( ))
                {
                    master = object;
                    break;
                }
            }
        }
    }
    
    SessionSubscribeObjectReplyPacket reply( packet );
    reply.nodeID = node->getNodeID();

    if( master )
    {
        // Check requested version
        const uint32_t version = packet->requestedVersion;
        if( version == VERSION_OLDEST || version >= master->getOldestVersion( ))
        {
            SessionSubscribeObjectSuccessPacket successPacket( packet );
            successPacket.changeType       = master->getChangeType();
            successPacket.masterInstanceID = master->getInstanceID();
            successPacket.nodeID = node->getNodeID();

            // Prefer multicast connection, since this will be used by the CM as
            // well. If we send the packet on another connection, it might
            // arrive after the packets below
            if( !node->multicast( successPacket ))
                node->send( successPacket );
        
            reply.cachedVersion = master->_cm->addSlave( command );
            reply.result = true;
        }
        else
        {
            EQWARN << "Version " << version << " no longer available"
                   << std::endl;
            reply.result = false;
        }
    }
    else
    {
        EQWARN << "Can't find master object to subscribe " << id << std::endl;
        reply.result = false;
    }

    if( !node->multicast( reply ))
        node->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObjectSuccess( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionSubscribeObjectSuccessPacket* packet = 
        command.getPacket<SessionSubscribeObjectSuccessPacket>();

    // Subscribe success packets are potentially multicasted (see above)
    // verify that we are the intended receiver
    const NodeID& nodeID = packet->nodeID;
    if( nodeID != _localNode->getNodeID( ))
        return COMMAND_HANDLED;

    EQLOG( LOG_OBJECTS ) << "Cmd subscribe object success " << packet
                         << std::endl;

    // set up change manager and attach object to dispatch table
    Object* object = static_cast<Object*>( _localNode->getRequestData( 
                                               packet->requestID ));    
    EQASSERT( object );
    EQASSERT( !object->isMaster( ));

    object->setupChangeManager( 
        static_cast< Object::ChangeType >( packet->changeType ), false, 
        packet->masterInstanceID );

    _attachObject( object, packet->objectID, packet->instanceID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObjectReply( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionSubscribeObjectReplyPacket* packet = 
        command.getPacket<SessionSubscribeObjectReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd object subscribe reply " << packet
                         << std::endl;

    // Subscribe reply packets are potentially multicasted (see above)
    // verify that we are the intended receiver
    const NodeID& nodeID = packet->nodeID;
    if( nodeID != _localNode->getNodeID( ))
        return COMMAND_HANDLED;

    EQASSERT( _localNode->getRequestData( packet->requestID ));

    if( packet->result )
    {
        Object* object = static_cast<Object*>( 
            _localNode->getRequestData( packet->requestID ));    
        EQASSERT( object );
        EQASSERT( !object->isMaster( ));

        object->_cm->setMasterNode( command.getNode( ));

        if( packet->useCache )
        {
            const uint32_t id = packet->objectID;
            const uint32_t start = packet->cachedVersion;
            if( start != VERSION_INVALID )
            {
                const InstanceCache::Data& cached = _instanceCache[ id ];
                EQASSERT( cached != InstanceCache::Data::NONE );
                EQASSERT( !cached.versions.empty( ));
            
                object->_cm->addInstanceDatas( cached.versions, start );
                EQCHECK( _instanceCache.release( id, 2 ));
            }
            else
            {
                EQCHECK( _instanceCache.release( id ));
            }
        }
    }
    else
        EQWARN << "Could not subscribe object " << packet->objectID
               << std::endl;

    _localNode->serveRequest( packet->requestID, packet->version );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnsubscribeObject( Command& command )
{
    CHECK_THREAD( _commandThread );
    SessionUnsubscribeObjectPacket* packet =
        command.getPacket<SessionUnsubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unsubscribe object  " << packet << std::endl;

    NodePtr        node = command.getNode();
    const uint32_t id   = packet->objectID;

    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        ObjectVectorHash::const_iterator i = _objects->find( id );
        if( i != _objects->end( ))
        {
            const ObjectVector& objects = i->second;

            for( ObjectVector::const_iterator j = objects.begin();
                 j != objects.end(); ++j )
            {
                Object* object = *j;
                if( object->isMaster() && 
                    object->getInstanceID() == packet->masterInstanceID )
                {
                    object->removeSlave( node );
                    break;
                }
            }   
        }
    }

    SessionDetachObjectPacket detachPacket( packet );
    send( node, detachPacket );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnmapObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionUnmapObjectPacket* packet = 
        command.getPacket< SessionUnmapObjectPacket >();

    EQLOG( LOG_OBJECTS ) << "Cmd unmap object " << packet << std::endl;

    _instanceCache.erase( packet->objectID );

    ObjectVectorHash::iterator i = _objects->find( packet->objectID );
    if( i == _objects->end( )) // nothing to do
        return COMMAND_HANDLED;

    const ObjectVector objects = i->second;
    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        _objects->erase( i );
    }

    for( ObjectVector::const_iterator j = objects.begin();
         j != objects.end(); ++j )
    {
        Object* object = *j;
        object->detachFromSession();
    }

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdInstance( Command& command )
{
    ObjectInstancePacket* packet = command.getPacket< ObjectInstancePacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd instance  " << packet << std::endl;

    CHECK_THREAD( _commandThread );
    EQASSERT( _localNode.isValid( ));

    packet->datatype = DATATYPE_EQNET_OBJECT;
    packet->command = CMD_OBJECT_INSTANCE;

    uint32_t usage = 0;
    CommandResult result = COMMAND_HANDLED;

    if( packet->nodeID == _localNode->getNodeID( ))
    {
        usage = 1; // TODO correct usage count
        result = _invokeObjectCommand( command );
    }

    if( packet->dataSize > 0 )
    {
        const ObjectVersion rev( packet->objectID, packet->version ); 
        _instanceCache.add( rev, packet->masterInstanceID, command, usage );
    }

    return result;
}

std::ostream& operator << ( std::ostream& os, Session* session )
{
    if( !session )
    {
        os << "NULL session";
        return os;
    }
    
    os << "session " << session->getID() << " (" << (void*)session
       << ")";

    return os;
}
}
}
