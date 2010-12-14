
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "objectStore.h"

#include "barrier.h"
#include "command.h"
#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "instanceCache.h" // member
#include "log.h"

#include "objectCM.h"
#include "objectDataIStream.h"
#include "objectPackets.h"
#include "objectStorePackets.h"

//#define DEBUG_DISPATCH
#ifdef DEBUG_DISPATCH
#  include <set>
#endif

namespace eq
{
namespace net
{
#define MIN_ID_RANGE 1024

typedef CommandFunc<ObjectStore> CmdFunc;

ObjectStore::ObjectStore( LocalNode* localNode )
        : _localNode( localNode )
        , _instanceIDs( std::numeric_limits< long >::min( )) 
        , _instanceCache( new InstanceCache( Global::getIAttribute( 
                              Global::IATTR_INSTANCE_CACHE_SIZE ) * EQ_1MB ) )
{
    EQVERB << "New ObjectStore @" << (void*)this << std::endl;
    CommandQueue* queue = _localNode->getCommandThreadQueue();

    registerCommand( CMD_OBJECTSTORE_ACK_REQUEST, 
                     CmdFunc( this, &ObjectStore::_cmdAckRequest ), 0 );
    registerCommand( CMD_OBJECTSTORE_FIND_MASTER_NODE_ID,
                     CmdFunc( this, &ObjectStore::_cmdFindMasterNodeID ),
                     queue );
    registerCommand( CMD_OBJECTSTORE_FIND_MASTER_NODE_ID_REPLY,
                     CmdFunc( this, &ObjectStore::_cmdFindMasterNodeIDReply ),
                     0 );
    registerCommand( CMD_OBJECTSTORE_ATTACH_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdAttachObject ), 0 );
    registerCommand( CMD_OBJECTSTORE_DETACH_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdDetachObject ), 0 );
    registerCommand( CMD_OBJECTSTORE_REGISTER_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdRegisterObject ), queue );
    registerCommand( CMD_OBJECTSTORE_DEREGISTER_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdDeregisterObject ),
                     queue );
    registerCommand( CMD_OBJECTSTORE_MAP_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdMapObject ), queue );
    registerCommand( CMD_OBJECTSTORE_MAP_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdMapObject ), queue );
    registerCommand( CMD_OBJECTSTORE_MAP_OBJECT_SUCCESS,
                     CmdFunc( this, &ObjectStore::_cmdMapObjectSuccess ), 0 );
    registerCommand( CMD_OBJECTSTORE_MAP_OBJECT_REPLY,
                     CmdFunc( this, &ObjectStore::_cmdMapObjectReply ), queue );
    registerCommand( CMD_OBJECTSTORE_UNMAP_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdUnmapObject ), 0 );
    registerCommand( CMD_OBJECTSTORE_UNSUBSCRIBE_OBJECT,
                     CmdFunc( this, &ObjectStore::_cmdUnsubscribeObject ),
                     queue );
    registerCommand( CMD_OBJECTSTORE_OBJECT_INSTANCE,
                     CmdFunc( this, &ObjectStore::_cmdInstance ), 0 );
    registerCommand( CMD_OBJECTSTORE_INSTANCE,
                     CmdFunc( this, &ObjectStore::_cmdInstance ), 0 );
}

ObjectStore::~ObjectStore()
{
    EQVERB << "Delete ObjectStore @" << (void*)this << std::endl;
    
#ifndef NDEBUG
    if( !_objects->empty( ))
    {
        EQWARN << _objects->size() << " attached objects in destructor"
               << std::endl;
        
        for( ObjectsHash::const_iterator i = _objects->begin();
             i != _objects->end(); ++i )
        {
            const Objects& objects = i->second;
            EQWARN << "  " << objects.size() << " objects with id " 
                   << i->first << std::endl;
            
            for( Objects::const_iterator j = objects.begin();
                 j != objects.end(); ++j )
            {
                const Object* object = *j;
                EQINFO << "    object type " << base::className( object )
                       << std::endl;
            }
        }
    }
    //EQASSERT( _objects->empty( ))
#endif
   clear();
}

void ObjectStore::clear( )
{
   // EQASSERT( _objects->empty( ));
    expireInstanceData( 0 );
    _objects->clear();
    _sendQueue.clear();
}

void ObjectStore::disableInstanceCache()
{
    EQASSERT( _localNode->isClosed( ));
    delete _instanceCache;
    _instanceCache = 0;
}

void ObjectStore::expireInstanceData( const int64_t age )
{
    if( _instanceCache )
        _instanceCache->expire( age ); 
}

//---------------------------------------------------------------------------
// identifier master node mapping
//---------------------------------------------------------------------------
NodeID ObjectStore::_findMasterNodeID( const base::UUID& identifier )
{
    // OPT: look up locally first?
    Nodes nodes;
    _localNode->getNodes( nodes );
    
    // OPT: send to multiple nodes at once?
    for( Nodes::iterator i = nodes.begin(); i != nodes.end(); i++ )
    {
        NodePtr node = *i;

        ObjectStoreFindMasterNodeID packet;
        packet.requestID = _localNode->registerRequest();
        packet.identifier = identifier;
        node->send( packet );
        NodeID masterNodeID = base::UUID::ZERO;
        _localNode->waitRequest( packet.requestID, masterNodeID );
        if( masterNodeID != base::UUID::ZERO )
            return masterNodeID;
    }

    return base::UUID::ZERO;

}

//---------------------------------------------------------------------------
// object mapping
//---------------------------------------------------------------------------
void ObjectStore::attachObject( Object* object, const base::UUID& id, 
                                const uint32_t instanceID )
{
    EQASSERT( object );
    EQ_TS_NOT_THREAD( _receiverThread );

    ObjectStoreAttachObjectPacket packet;
    packet.requestID = _localNode->registerRequest( object );
    packet.objectID = id;
    packet.objectInstanceID = instanceID;

    _localNode->send( packet );
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

void ObjectStore::_attachObject( Object* object, const base::UUID& id, 
                             const uint32_t inInstanceID )
{
    EQASSERT( object );
    EQ_TS_THREAD( _receiverThread );

    uint32_t instanceID = inInstanceID;
    if( inInstanceID == EQ_ID_INVALID )
        instanceID = _genNextID( _instanceIDs );

    object->attach( id, instanceID, _localNode );
    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        Objects& objects = _objects.data[ id ];
        objects.push_back( object );
    }

    _localNode->flushCommands(); // redispatch pending commands

    EQLOG( LOG_OBJECTS ) << "attached " << object << " cm "
                         << base::className( object->_cm ) << " @" 
                         << static_cast< void* >( object ) << std::endl;
}

void ObjectStore::detachObject( Object* object )
{
    EQASSERT( object );
    EQ_TS_NOT_THREAD( _receiverThread );

    ObjectStoreDetachObjectPacket packet;
    packet.requestID = _localNode->registerRequest();
    packet.objectID  = object->getID();
    packet.objectInstanceID  = object->getInstanceID();

    _localNode->send( packet );
    _localNode->waitRequest( packet.requestID );
}

void ObjectStore::swapObject( Object* oldObject, Object* newObject )
{
    EQASSERT( newObject );
    EQASSERT( oldObject );
    EQASSERT( oldObject->isMaster() );
    EQ_TS_THREAD( _receiverThread );
    base::ScopedMutex< base::SpinLock > mutex( _objects );

    if( !oldObject->isAttached() )
        return;

    const base::UUID& id = oldObject->getID();

    EQLOG( LOG_OBJECTS ) << "Swap " << base::className( oldObject )
                         << std::endl;

    ObjectsHash::iterator i = _objects->find( id );
    EQASSERT( i != _objects->end( ));
    if( i == _objects->end( ))
        return;

    Objects& objects = i->second;
    Objects::iterator j = find( objects.begin(),objects.end(), oldObject );
    EQASSERT( j != objects.end( ));
    if( j == objects.end( ))
        return;

    newObject->_id           = id;
    newObject->_instanceID   = oldObject->getInstanceID();
    newObject->_cm           = oldObject->_cm; 
    newObject->_localNode    = oldObject->_localNode;
    newObject->_cm->setObject( newObject );

    oldObject->_cm = ObjectCM::ZERO;
    oldObject->_localNode = 0;
    oldObject->_instanceID = EQ_ID_INVALID;

    *j = newObject;
}

void ObjectStore::_detachObject( Object* object )
{
    // check also _cmdUnmapObject when modifying!
    EQASSERT( object );
    EQ_TS_THREAD( _receiverThread );

    if( !object->isAttached() )
        return;

    const base::UUID& id = object->getID();

    EQASSERT( _objects->find( id ) != _objects->end( ));
    EQLOG( LOG_OBJECTS ) << "Detach " << object << std::endl;

    Objects& objects = _objects.data[ id ];
    Objects::iterator i = find( objects.begin(),objects.end(), object );
    EQASSERT( i != objects.end( ));

    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        objects.erase( i );
        if( objects.empty( ))
            _objects->erase( id );
    }

    EQASSERT( object->getInstanceID() != EQ_ID_INVALID );
    object->detach();
    return;
}

uint32_t ObjectStore::mapObjectNB( Object* object, const base::UUID& id,
                               const uint128_t& version )
{
    EQ_TS_NOT_THREAD( _commandThread );
    EQ_TS_NOT_THREAD( _receiverThread );
    EQLOG( LOG_OBJECTS ) << "Mapping " << base::className( object ) << " to id "
                         << id << " version " << version << std::endl;
    EQASSERT( object );
    EQASSERT( !object->isAttached( ));
    EQASSERT( !object->isMaster( ));
    EQASSERT( !_localNode->inCommandThread( ));
    EQASSERTINFO( id.isGenerated(), id );

    if( !id.isGenerated( ))
        return EQ_ID_INVALID;

    NodePtr master = _connectMaster( id );
    EQASSERT( master );
    if( !master )
        return EQ_ID_INVALID;

    ObjectStoreMapObjectPacket packet;
    packet.requestID        = _localNode->registerRequest( object );
    packet.objectID         = id;
    packet.requestedVersion = version;
    packet.instanceID       = _genNextID( _instanceIDs );

    if( _instanceCache )
    {
        const InstanceCache::Data& cached = (*_instanceCache)[ id ];
        if( cached != InstanceCache::Data::NONE )
        {
            const ObjectDataIStreamDeque& versions = cached.versions;
            EQASSERT( !cached.versions.empty( ));
            packet.useCache = true;
            packet.masterInstanceID = cached.masterInstanceID;
            packet.minCachedVersion = versions.front()->getVersion();
            packet.maxCachedVersion = versions.back()->getVersion();
            EQLOG( LOG_OBJECTS ) << "Object " << id << " have v"
                                 << packet.minCachedVersion << ".."
                                 << packet.maxCachedVersion << std::endl;
        }
    }
    master->send( packet );
    return packet.requestID;
}

bool ObjectStore::mapObjectSync( const uint32_t requestID )
{
    if( requestID == EQ_ID_INVALID )
        return false;

    void* data = _localNode->getRequestData( requestID );    
    if( data == 0 )
        return false;

    Object* object = EQSAFECAST( Object*, data );
    uint128_t version = VERSION_NONE;
    _localNode->waitRequest( requestID, version );

    const bool mapped = ( object->isAttached() );
    if( mapped )
        object->_cm->applyMapData( version ); // apply initial instance data

    object->notifyAttached();
    EQLOG( LOG_OBJECTS ) << "Mapped " << object << std::endl;
    return mapped;
}

void ObjectStore::unmapObject( Object* object )
{
    EQASSERT( object );

    if( !object->isAttached() ) // not registered
        return;

    const base::UUID& id = object->getID();
    
    EQLOG( LOG_OBJECTS ) << "Unmap " << object << std::endl;

    object->notifyDetach();

    // send unsubscribe to master, master will send detach packet.
    EQASSERT( !object->isMaster( ));
    EQ_TS_NOT_THREAD( _commandThread );
    
    const uint32_t masterInstanceID = object->getMasterInstanceID();
    if( masterInstanceID != EQ_ID_INVALID )
    {
        // TO-DO : may be we can take master node directly from the object
        const NodeID masterNodeID = _findMasterNodeID( id );
        NodePtr master = _localNode->getNode( masterNodeID );
            
        if( master.isValid() && master->isConnected( ))
        {
            ObjectStoreUnsubscribeObjectPacket packet;
            packet.requestID = _localNode->registerRequest();
            packet.objectID  = id;
            packet.masterInstanceID = masterInstanceID;
            packet.slaveInstanceID  = object->getInstanceID();
            master->send( packet );

            _localNode->waitRequest( packet.requestID );
            //if( _instanceCache )
            //    _instanceCache->erase( id );
            return;
        }
        EQERROR << "Master node for object id " << id << " not connected"
                << std::endl;
    }

    // no unsubscribe sent: Detach directly
    detachObject( object );
    object->_setChangeManager( ObjectCM::ZERO );
    //if( _instanceCache )
    //    _instanceCache->erase( id );
}

bool ObjectStore::registerObject( Object* object )
{
    EQASSERT( object );
    EQASSERT( !object->isAttached() );

    const base::UUID& id = object->getID( );
    EQASSERTINFO( id.isGenerated(), id );

    object->setupChangeManager( object->getChangeType(), true );
    attachObject( object, id, EQ_ID_INVALID );

    if( Global::getIAttribute( Global::IATTR_NODE_SEND_QUEUE_SIZE )> 0 )
    {
        ObjectStoreRegisterObjectPacket packet;
        packet.object = object;
        _localNode->send( packet );
    }

    object->notifyAttached();

    EQLOG( LOG_OBJECTS ) << "Registered " << object << std::endl;
    return true;
}

void ObjectStore::deregisterObject( Object* object )
{
    EQASSERT( object );
    if( !object->isAttached() ) // not registered
        return;

    EQLOG( LOG_OBJECTS ) << "Deregister " << object << std::endl;
    EQASSERT( object->isMaster( ));

    const base::UUID& id = object->getID();
    object->notifyDetach();

    if( Global::getIAttribute( Global::IATTR_NODE_SEND_QUEUE_SIZE )> 0 )
    {
        // remove from send queue
        ObjectStoreDeregisterObjectPacket packet;
        packet.requestID = _localNode->registerRequest( object );
        _localNode->send( packet );
        _localNode->waitRequest( packet.requestID );
    }

    // unmap slaves
    const Nodes* slaves = object->_getSlaveNodes();
    if( slaves && !slaves->empty( ))
    {
        EQWARN << slaves->size() 
               << " slave nodes subscribed during deregisterObject of "
               << base::className( object ) << " id " << object->getID()
               << std::endl;

        ObjectStoreUnmapObjectPacket packet;
        packet.objectID = id;

        for( Nodes::const_iterator i = slaves->begin();
             i != slaves->end(); ++i )
        {
            NodePtr node = *i;
            node->send( packet );
        }
    }

    detachObject( object );
    object->_setChangeManager( ObjectCM::ZERO );
    
    if( _instanceCache )
        _instanceCache->erase( id );
}



NodePtr ObjectStore::_connectMaster( const base::UUID& id )
{
    const NodeID masterNodeID = _findMasterNodeID( id );
    if( masterNodeID == base::UUID::ZERO )
    {
        EQWARN << "Can't find master node for object id " << id <<std::endl;
        return 0;
    }

    NodePtr master = _localNode->connect( masterNodeID );
    if( master.isValid() && !master->isClosed( ))
        return master;

    EQWARN << "Can't connect master node with id " << masterNodeID
           << " for object id " << id << std::endl;
    return 0;
}

void ObjectStore::ackRequest( NodePtr node, const uint32_t requestID )
{
    if( requestID == EQ_ID_INVALID ) // no need to ack operation
        return;

    if( node == _localNode ) // OPT
        _localNode->serveRequest( requestID );
    else
    {
        ObjectStoreAckRequestPacket reply( requestID );
        node->send( reply );
    }
}

bool ObjectStore::notifyCommandThreadIdle()
{
    EQ_TS_THREAD( _commandThread );
    if( _sendQueue.empty( ))
        return false;

    Nodes nodes;
    _localNode->getNodes( nodes );

    SendQueueItem& object = _sendQueue.front();
    if( object.age > _clock.getTime64( ))
        object.object->_cm->sendInstanceDatas( nodes );

    _sendQueue.pop_front();
    return !_sendQueue.empty();
}

//===========================================================================
// Packet handling
//===========================================================================
bool ObjectStore::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << std::endl;
    EQASSERT( command.isValid( ));
    EQ_TS_THREAD( _receiverThread );

    switch( command->type )
    {
        case PACKETTYPE_EQNET_OBJECTSTORE:
            EQCHECK( Dispatcher::dispatchCommand( command ));
            return true;

        case PACKETTYPE_EQNET_OBJECT:
            return _dispatchObjectCommand( command );

        default:
            EQABORT( "Unknown packet type " << command->type << " for "
                     << command );
            return true;
    }
}

bool ObjectStore::_dispatchObjectCommand( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const ObjectPacket* packet = command.getPacket< ObjectPacket >();
    const base::UUID& id = packet->objectID;
    const uint32_t instanceID = packet->instanceID;

    ObjectsHash::const_iterator i = _objects->find( id );

    if( i == _objects->end( ))
        // When the instance ID is set to none, we only care about the packet
        // when we have an object of the given ID (multicast)
        return ( instanceID == EQ_ID_NONE );

    const Objects& objects = i->second;
    EQASSERTINFO( !objects.empty(), packet );

    if( instanceID <= EQ_ID_MAX )
    {
        for( Objects::const_iterator j = objects.begin(); j!=objects.end(); ++j)
        {
            Object* object = *j;
            if( instanceID == object->getInstanceID( ))
            {
                EQCHECK( object->dispatchCommand( command ));
                return true;
            }
        }
        EQUNREACHABLE;
        return false;
    }

    Objects::const_iterator j = objects.begin();
    Object* object = *j;
    EQCHECK( object->dispatchCommand( command ));
    EQASSERTINFO( command.getDispatchID() <= EQ_ID_MAX, command );
    EQASSERTINFO( command.getDispatchID() == object->getInstanceID(),
                  command.getDispatchID() << " != " << object->getInstanceID());

#ifdef DEBUG_DISPATCH
    std::set< uint32_t > instances;
    instances.insert( object->getInstanceID( ));
#endif

    for( ++j; j != objects.end(); ++j )
    {
        object = *j;
        Command& clone = _localNode->cloneCommand( command );

#ifdef DEBUG_DISPATCH
        const uint32_t instance = object->getInstanceID();
        EQASSERT( instances.find( instance ) == instances.end( ));
        instances.insert( instance );
#endif
        EQCHECK( object->dispatchCommand( clone ));
        EQASSERTINFO( clone.getDispatchID() <= EQ_ID_MAX, clone );
        EQASSERTINFO( &clone == &command || // command already reused
                      clone.getDispatchID() != command.getDispatchID(),
                      command );
    }
    return true;
}

bool ObjectStore::invokeCommand( Command& command )
{
    EQVERB << "invoke " << command << std::endl;
    EQASSERT( command.isValid( ));

    switch( command->type )
    {
        case PACKETTYPE_EQNET_OBJECTSTORE:
            return Dispatcher::invokeCommand( command );

        case PACKETTYPE_EQNET_OBJECT:
            return _invokeObjectCommand( command );

        default:
            EQWARN << "Unhandled command " << command << std::endl;
            return false;
    }
}

bool ObjectStore::_invokeObjectCommand( Command& command )
{
    EQASSERT( command.isValid( ));
    EQASSERT( command->type == PACKETTYPE_EQNET_OBJECT );

    Object* object = _findObject( command );
    if( !object )
        return false;
    
    if( !object->invokeCommand( command ))
    {
        EQERROR << "Error handling " << command << " for object of type "
                << base::className( object ) << std::endl;
        return false;
    }
    return true;
}

Object* ObjectStore::_findObject( Command& command )
{
    EQASSERT( command.isValid( ));
    EQASSERT( command->type == PACKETTYPE_EQNET_OBJECT );

    const ObjectPacket* packet = command.getPacket< ObjectPacket >();
    const base::UUID& id = packet->objectID;
    const uint32_t instanceID = command.getDispatchID();
    EQASSERTINFO( instanceID <= EQ_ID_MAX, command );

    base::ScopedMutex< base::SpinLock > mutex( _objects );
    ObjectsHash::const_iterator i = _objects->find( id );

    if( i == _objects->end( ))
    {
        EQASSERTINFO( false,
                      "no objects to handle command " << packet << " instance "
                      << instanceID << " in " << base::className( this ));
        return 0;
    }

    const Objects& objects = i->second;
    EQASSERTINFO( !objects.empty(), packet );

    for( Objects::const_iterator j = objects.begin(); j != objects.end(); ++j )
    {
        Object* object = *j;
        if( instanceID == object->getInstanceID( ))
            return object;
    }

    EQASSERTINFO( false, "object instance " << instanceID << " not found for "<<
                  packet );
    return 0;
}

bool ObjectStore::_cmdAckRequest( Command& command )
{
    const ObjectStoreAckRequestPacket* packet = 
        command.getPacket<ObjectStoreAckRequestPacket>();
    EQASSERT( packet->requestID != EQ_ID_INVALID );

    _localNode->serveRequest( packet->requestID );
    return true;
}

bool ObjectStore::_cmdFindMasterNodeID( Command& command )
{
    EQ_TS_THREAD( _commandThread );

    const ObjectStoreFindMasterNodeID* packet = 
          command.getPacket<ObjectStoreFindMasterNodeID>();

    const base::UUID& id = packet->identifier;
    EQASSERT( id.isGenerated() );

    ObjectStoreFindMasterNodeIDReply reply( packet );
    base::ScopedMutex< base::SpinLock > mutex( _objects );
    ObjectsHash::const_iterator i = _objects->find( id );
    
    if( i == _objects->end( ))
    {
        command.getNode()->send( reply );
        return true;
    }
    
    const Objects& objects = i->second;
    EQASSERTINFO( !objects.empty(), packet );

    for( Objects::const_iterator j = objects.begin(); j != objects.end(); ++j )
    {
        Object* object = *j;
        const NodePtr masterNode = object->_cm->getMasterNode();
        if( masterNode.isValid() && masterNode->getNodeID()!=base::UUID::ZERO )
        {
            reply.masterNodeID = masterNode->getNodeID();
            break;
        }
    }
    
    command.getNode()->send( reply );
    return true;
}

bool ObjectStore::_cmdFindMasterNodeIDReply( Command& command )
{
    const ObjectStoreFindMasterNodeIDReply* packet = 
          command.getPacket<ObjectStoreFindMasterNodeIDReply>();

    _localNode->serveRequest( packet->requestID, packet->masterNodeID );

    return true;
}

bool ObjectStore::_cmdAttachObject( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const ObjectStoreAttachObjectPacket* packet = 
        command.getPacket<ObjectStoreAttachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd attach object " << packet << std::endl;

    Object* object = static_cast< Object* >( _localNode->getRequestData( 
                                                 packet->requestID ));
    _attachObject( object, packet->objectID, packet->objectInstanceID );
    _localNode->serveRequest( packet->requestID );
    return true;
}

bool ObjectStore::_cmdDetachObject( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const ObjectStoreDetachObjectPacket* packet = 
        command.getPacket<ObjectStoreDetachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd detach object " << packet << std::endl;

    const base::UUID& id = packet->objectID;
    ObjectsHash::const_iterator i = _objects->find( id );
    if( i != _objects->end( ))
    {
        const Objects& objects = i->second;

        for( Objects::const_iterator j = objects.begin();
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

    EQASSERT( packet->requestID != EQ_ID_INVALID );
    _localNode->serveRequest( packet->requestID );
    return true;
}

bool ObjectStore::_cmdRegisterObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const ObjectStoreRegisterObjectPacket* packet = 
        command.getPacket< ObjectStoreRegisterObjectPacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd register object " << packet << std::endl;

    const uint32_t size = Global::getIAttribute( 
                             Global::IATTR_NODE_SEND_QUEUE_SIZE );
    const int32_t age = Global::getIAttribute(Global::IATTR_NODE_SEND_AGE );
#if 0
    if( _sendQueue.size() >= size )
        return true;
#endif
    
    SendQueueItem item;
    item.age = age ? age + _clock.getTime64() :
                     std::numeric_limits< int64_t >::max();
    item.object = packet->object;
    _sendQueue.push_back( item );
    while( _sendQueue.size() > size )
        _sendQueue.pop_front();

    return true;
}

bool ObjectStore::_cmdDeregisterObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const ObjectStoreDeregisterObjectPacket* packet = 
        command.getPacket< ObjectStoreDeregisterObjectPacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd deregister object " << packet << std::endl;

    Object* object = static_cast<Object*>(
        _localNode->getRequestData( packet->requestID ));    

    for( SendQueue::iterator i = _sendQueue.begin(); i < _sendQueue.end(); ++i )
    {
        if( i->object == object )
        {
            _sendQueue.erase( i );
            break;
        }
    }

    _localNode->serveRequest( packet->requestID );
    return true;
}

bool ObjectStore::_cmdMapObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    ObjectStoreMapObjectPacket* packet =
        command.getPacket<ObjectStoreMapObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd map object " << packet << std::endl;

    NodePtr        node = command.getNode();
    const base::UUID& id   = packet->objectID;

    Object* master = 0;
    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        ObjectsHash::const_iterator i = _objects->find( id );
        if( i != _objects->end( ))
        {
            const Objects& objects = i->second;

            for( Objects::const_iterator j = objects.begin();
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
    
    ObjectStoreMapObjectReplyPacket reply( packet );
    reply.nodeID = node->getNodeID();

    if( master )
    {
        // Check requested version
        const uint128_t version = packet->requestedVersion;
        const uint128_t oldestVersion = master->getOldestVersion();
        if( version == VERSION_OLDEST || version == VERSION_NONE ||
            version >= oldestVersion)
        {
            ObjectStoreMapObjectSuccessPacket successPacket( packet );
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

            if( reply.version == VERSION_OLDEST )
                reply.version = master->getOldestVersion();
            else if( reply.version == VERSION_NONE )
                reply.version = master->getVersion();
        }
        else
        {
            EQWARN
                << "Version " << version << " of " << base::className( master )
                << " " << id << " no longer available (have v"
                << master->getOldestVersion() << ".." << master->getVersion()
                << " [" << master->getAutoObsolete() << "])" << std::endl;
            reply.result = false;
        }
    }
    else
    {
        EQWARN << "Can't find master object to map " << id << std::endl;
        reply.result = false;
    }

    if( !node->multicast( reply ))
        node->send( reply );
    return true;
}

bool ObjectStore::_cmdMapObjectSuccess( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const ObjectStoreMapObjectSuccessPacket* packet = 
        command.getPacket<ObjectStoreMapObjectSuccessPacket>();

    // Map success packets are potentially multicasted (see above)
    // verify that we are the intended receiver
    const NodeID& nodeID = packet->nodeID;
    if( nodeID != _localNode->getNodeID( ))
        return true;

    EQLOG( LOG_OBJECTS ) << "Cmd map object success " << packet
                         << std::endl;

    // set up change manager and attach object to dispatch table
    Object* object = static_cast<Object*>( _localNode->getRequestData( 
                                               packet->requestID ));    
    EQASSERT( object );
    EQASSERT( !object->isMaster( ));

    object->setupChangeManager( Object::ChangeType( packet->changeType ), false,
                                packet->masterInstanceID );
    _attachObject( object, packet->objectID, packet->instanceID );
    return true;
}

bool ObjectStore::_cmdMapObjectReply( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const ObjectStoreMapObjectReplyPacket* packet = 
        command.getPacket<ObjectStoreMapObjectReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd map object reply " << packet
                         << std::endl;

    // Map reply packets are potentially multicasted (see above)
    // verify that we are the intended receiver
    const NodeID& nodeID = packet->nodeID;
    if( nodeID != _localNode->getNodeID( ))
        return true;

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
            const base::UUID& id = packet->objectID;
            const uint128_t& start = packet->cachedVersion;
            
            EQASSERT( _instanceCache );
            if( start != VERSION_INVALID )
            {
                const InstanceCache::Data& cached = (*_instanceCache)[ id ];
                EQASSERT( cached != InstanceCache::Data::NONE );
                EQASSERT( !cached.versions.empty( ));
            
                object->_cm->addInstanceDatas( cached.versions, start );
               EQCHECK( _instanceCache->release( id, 2 ));
            }
            else
            {
                EQCHECK( _instanceCache->release( id ));
            }
        }
    }
    else
        EQWARN << "Could not map object " << packet->objectID
               << std::endl;

    _localNode->serveRequest( packet->requestID, packet->version );
    return true;
}

bool ObjectStore::_cmdUnsubscribeObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    ObjectStoreUnsubscribeObjectPacket* packet =
        command.getPacket<ObjectStoreUnsubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unsubscribe object  " << packet << std::endl;

    NodePtr        node = command.getNode();
    const base::UUID& id   = packet->objectID;

    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        ObjectsHash::const_iterator i = _objects->find( id );
        if( i != _objects->end( ))
        {
            const Objects& objects = i->second;

            for( Objects::const_iterator j = objects.begin();
                 j != objects.end(); ++j )
            {
                Object* object = *j;
                if( object->isMaster() && 
                    object->getInstanceID() == packet->masterInstanceID )
                {
                    object->_cm->removeSlave( node );
                    break;
                }
            }   
        }
    }

    ObjectStoreDetachObjectPacket detachPacket( packet );
    node->send( detachPacket );
    return true;
}

bool ObjectStore::_cmdUnmapObject( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const ObjectStoreUnmapObjectPacket* packet = 
        command.getPacket< ObjectStoreUnmapObjectPacket >();

    EQLOG( LOG_OBJECTS ) << "Cmd unmap object " << packet << std::endl;
    if( _instanceCache )
        _instanceCache->erase( packet->objectID );

    ObjectsHash::iterator i = _objects->find( packet->objectID );
    if( i == _objects->end( )) // nothing to do
        return true;

    const Objects objects = i->second;
    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        _objects->erase( i );
    }

    for( Objects::const_iterator j = objects.begin(); j != objects.end(); ++j )
    {
        Object* object = *j;
        object->detach();
    }

    return true;
}

bool ObjectStore::_cmdInstance( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    EQASSERT( _localNode );

    ObjectInstancePacket* packet = command.getPacket< ObjectInstancePacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd instance " << packet << std::endl;

    packet->type = PACKETTYPE_EQNET_OBJECT;
    packet->command = CMD_OBJECT_INSTANCE;

    uint32_t usage = 0;
    bool result = true;

    // Packet is send due to:
    //  map request:      concrete nodeID and instanceID, dispatch
    //  master commit:    instanceID == ANY, dispatch
    //  send-on-register: instanceID == NONE, do not dispatch
    if( packet->nodeID == _localNode->getNodeID( ))
    {
        EQASSERT( packet->instanceID <= EQ_ID_MAX );

        usage = 1;
        result = _dispatchObjectCommand( command );
    }
    else if( packet->instanceID != EQ_ID_NONE )
    {
        EQASSERTINFO( packet->instanceID == EQ_ID_ANY, packet );
        packet->instanceID = EQ_ID_NONE; // drop if there are no local instances
        result = _dispatchObjectCommand( command );
    }

    if( _instanceCache )
    {
        const ObjectVersion rev( packet->objectID, packet->version ); 
        _instanceCache->add( rev, packet->masterInstanceID, command, usage );
    }

    return result;
}

std::ostream& operator << ( std::ostream& os, ObjectStore* objectStore )
{
    if( !objectStore )
    {
        os << "NULL objectStore";
        return os;
    }
    
    os << "objectStore (" << (void*)objectStore << ")";

    return os;
}
}
}
