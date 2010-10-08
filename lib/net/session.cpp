
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

//#define DEBUG_DISPATCH
#ifdef DEBUG_DISPATCH
#  include <set>
#endif

namespace eq
{
namespace net
{
#define MIN_ID_RANGE 1024

typedef CommandFunc<Session> CmdFunc;

Session::Session()
        : _id( true )
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
    EQASSERTINFO( !_server, "Session still mapped during deletion");

    _id        = SessionID::ZERO;
    _isMaster  = false;
    _localNode = 0;
    _server    = 0;
    
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
    registerCommand( CMD_SESSION_UNSET_ID_MASTER,
                     CmdFunc( this, &Session::_cmdUnsetIDMaster ), queue );
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
    EQ_TS_NOT_THREAD( _commandThread );
    _setIDMasterSync( _setIDMasterNB( id, master ));
}

uint32_t Session::_setIDMasterNB( const uint32_t identifier,
                                  const NodeID& master )
{
    EQ_TS_NOT_THREAD( _commandThread );

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

void Session::unsetIDMaster( const uint32_t identifier )
{
    _unsetIDMasterSync( _unsetIDMasterNB( identifier ));
}


uint32_t Session::_unsetIDMasterNB( const uint32_t identifier )
{
    EQ_TS_NOT_THREAD( _commandThread );
    SessionUnsetIDMasterPacket packet;
    packet.identifier = identifier;

    if( !_isMaster ) // unset on our slave instance
    {
        base::ScopedMutex< base::SpinLock > mutex( _idMasters );
        _idMasters->erase( identifier );
    }
    
    packet.requestID = _localNode->registerRequest();
    send( packet );       // unset on master instance (need to wait for ack)
    return packet.requestID;
}

void Session::_unsetIDMasterSync( const uint32_t requestID )
{
    _localNode->waitRequest( requestID );
}

const NodeID Session::_pollIDMaster( const uint32_t id ) const 
{
    base::ScopedMutex< base::SpinLock > mutex( _idMasters );
    NodeIDHash::const_iterator i = _idMasters->find( id );
    if( i == _idMasters->end( ))
        return NodeID::ZERO;

    return i->second;
}

const NodeID Session::getIDMaster( const uint32_t identifier )
{
    const NodeID master = _pollIDMaster( identifier );
        
    if( master != NodeID::ZERO || _isMaster )
        return master;

    // ask session master instance
    SessionGetIDMasterPacket packet;
    packet.requestID = _localNode->registerRequest();
    packet.identifier = identifier;

    send( packet );
    _localNode->waitRequest( packet.requestID );

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
    EQ_TS_NOT_THREAD( _receiverThread );

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
    EQ_TS_THREAD( _receiverThread );

    uint32_t instanceID = inInstanceID;
    if( inInstanceID == EQ_ID_INVALID )
        instanceID = _genNextID( _instanceIDs );

    object->attachToSession( id, instanceID, this );

    {
        base::ScopedMutex< base::SpinLock > mutex( _objects );
        Objects& objects = _objects.data[ id ];
        objects.push_back( object );
    }

    getLocalNode()->flushCommands(); // redispatch pending commands

    EQLOG( LOG_OBJECTS ) << "attached " << base::className( object ) << " id "
                         << object->getID() << '.' << object->getInstanceID()
                         << " cm " << base::className( object->_cm ) << " @" 
                         << static_cast< void* >( object ) << std::endl;
}

void Session::detachObject( Object* object )
{
    EQASSERT( object );
    EQ_TS_NOT_THREAD( _receiverThread );

    SessionDetachObjectPacket packet;
    packet.requestID = _localNode->registerRequest();
    packet.objectID  = object->getID();
    packet.objectInstanceID  = object->getInstanceID();

    _sendLocal( packet );
    _localNode->waitRequest( packet.requestID );
}

void Session::swapObject( Object* oldObject, Object* newObject )
{
    EQASSERT( newObject );
    EQASSERT( oldObject );
    EQASSERT( oldObject->isMaster() );
    EQ_TS_THREAD( _receiverThread );
    base::ScopedMutex< base::SpinLock > mutex( _objects );
    const uint32_t id = oldObject->getID();
    if( id == EQ_ID_INVALID )
        return;

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
    newObject->_session      = oldObject->_session;
    newObject->_cm->setObject( newObject );

    oldObject->_cm = ObjectCM::ZERO;
    oldObject->_session = 0;
    oldObject->_id = EQ_ID_INVALID;
    oldObject->_instanceID = EQ_ID_INVALID;

    *j = newObject;
}

void Session::_detachObject( Object* object )
{
    // check also _cmdUnmapObject when modifying!
    EQASSERT( object );
    EQ_TS_THREAD( _receiverThread );

    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID )
        return;

    EQASSERT( _objects->find( id ) != _objects->end( ));
    EQLOG( LOG_OBJECTS ) << "Detach " << base::className( object )
                         << " from id " << id << std::endl;

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
    EQ_TS_NOT_THREAD( _commandThread );
    EQASSERT( object );
    EQASSERT( id <= EQ_ID_MAX );

    EQLOG( LOG_OBJECTS ) << "Mapping " << base::className( object ) << " to id "
                         << id << " version " << version << std::endl;

    EQASSERT( object->getID() == EQ_ID_INVALID );
    EQASSERT( !object->isMaster( ));
    EQASSERT( !_localNode->inCommandThread( ));
        
    // Connect master node, can't do that from the command thread!
    NodeID masterNodeID = _pollIDMaster( id );
    NodePtr master;

    if( masterNodeID != NodeID::ZERO )
    {
        master = _localNode->connect( masterNodeID );
        if( !master || master->isClosed( ))
        {
            if( !_isMaster ) // clear local cache
            {
                SessionUnsetIDMasterPacket packet;
                packet.identifier = id;
                packet.requestID = _localNode->registerRequest();
                _sendLocal( packet );
                _localNode->waitRequest( packet.requestID );
            }
            master = 0;
        }
    }

    if( !master )
    {
        masterNodeID = getIDMaster( id );
        if( masterNodeID == NodeID::ZERO )
        {
            EQWARN << "Can't find master node for object id " << id <<std::endl;
            return EQ_ID_INVALID;
        }

        master = _localNode->connect( masterNodeID );
        if( !master || master->isClosed( ))
        {
            EQWARN << "Can't connect master node with id " << masterNodeID
                   << " for object id " << id << std::endl;
            return EQ_ID_INVALID;
        }
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

    object->notifyAttached();
    EQLOG( LOG_OBJECTS ) << "Mapped " << base::className( object ) << " to " 
                         << ObjectVersion( object ) << std::endl;
    return mapped;
}

void Session::unmapObject( Object* object )
{
    EQASSERT( object );

    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID ) // not registered
        return;

    EQLOG( LOG_OBJECTS ) << "Unmap " << base::className( object ) << " from id "
        << object->getID() << std::endl;

    object->notifyDetach();

    // send unsubscribe to master, master will send detach packet.
    EQASSERT( !object->isMaster( ));
    EQ_TS_NOT_THREAD( _commandThread );
    
    const uint32_t masterInstanceID = object->getMasterInstanceID();
    if( masterInstanceID != EQ_ID_INVALID )
    {
        const NodeID masterNodeID = _pollIDMaster( id );
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
    object->_setChangeManager( ObjectCM::ZERO );
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
    object->notifyAttached();

    EQLOG( LOG_OBJECTS ) << "Registered " << base::className( object )
                         << " to id " << id << std::endl;
    return true;
}

void Session::deregisterObject( Object* object )
{
    EQASSERT( object )
    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID ) // not registered
        return;

    EQLOG( LOG_OBJECTS ) << "Deregister " << base::className( object ) 
                         << " from id " << id << std::endl;
    EQASSERT( object->isMaster( ));

    object->notifyDetach();
    const uint32_t requestID = _unsetIDMasterNB( id );

    // unmap slaves
    const Nodes* slaves = object->_getSlaveNodes();
    if( slaves && !slaves->empty( ))
    {
        EQWARN << slaves->size() 
               << " slave nodes subscribed during deregisterObject of "
               << base::className( object ) << " id " << object->getID()
               << std::endl;

        SessionUnmapObjectPacket packet;
        packet.sessionID = _id;
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
    _unsetIDMasterSync( requestID );
    freeIDs( id, 1 );
}

void Session::releaseObject( Object* object )
{
    EQASSERT( object );

    if( !object || !object->isAttached( ))
        return;

    if( object->isMaster( ))
        deregisterObject( object );
    else
        unmapObject( object );
}

void Session::ackRequest( NodePtr node, const uint32_t requestID )
{
    if( requestID == EQ_ID_INVALID ) // no need to ack operation
        return;

    if( node == _localNode ) // OPT
        _localNode->serveRequest( requestID );
    else
    {
        SessionAckRequestPacket reply( requestID );
        send( node, reply );
    }
}

//===========================================================================
// Packet handling
//===========================================================================
bool Session::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << std::endl;
    EQASSERT( command.isValid( ));
    EQ_TS_THREAD( _receiverThread );

    switch( command->type )
    {
        case PACKETTYPE_EQNET_SESSION:
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

bool Session::_dispatchObjectCommand( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const ObjectPacket* packet = command.getPacket< ObjectPacket >();
    const uint32_t id = packet->objectID;
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
        EQASSERTINFO( clone.getDispatchID() != command.getDispatchID(),
                      command );
    }
    return true;
}

bool Session::invokeCommand( Command& command )
{
    EQVERB << "invoke " << command << std::endl;
    EQASSERT( command.isValid( ));

    switch( command->type )
    {
        case PACKETTYPE_EQNET_SESSION:
            return Dispatcher::invokeCommand( command );

        case PACKETTYPE_EQNET_OBJECT:
            return _invokeObjectCommand( command );

        default:
            EQWARN << "Unhandled command " << command << std::endl;
            return false;
    }
}

bool Session::_invokeObjectCommand( Command& command )
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

Object* Session::_findObject( Command& command )
{
    EQASSERT( command.isValid( ));
    EQASSERT( command->type == PACKETTYPE_EQNET_OBJECT );

    const ObjectPacket* packet = command.getPacket< ObjectPacket >();
    const uint32_t id = packet->objectID;
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

bool Session::_cmdAckRequest( Command& command )
{
    const SessionAckRequestPacket* packet = 
        command.getPacket<SessionAckRequestPacket>();
    EQASSERT( packet->requestID != EQ_ID_INVALID );

    _localNode->serveRequest( packet->requestID );
    return true;
}


bool Session::_cmdGenIDs( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionGenIDsPacket* packet =command.getPacket<SessionGenIDsPacket>();
    EQVERB << "Cmd gen IDs: " << packet << std::endl;

    SessionGenIDsReplyPacket reply( packet );
    const uint32_t range = EQ_MAX( packet->range, MIN_ID_RANGE );

    reply.firstID = _idPool.genIDs( range );
    reply.allocated = range;
    send( command.getNode(), reply );
    return true;
}

bool Session::_cmdGenIDsReply( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionGenIDsReplyPacket* packet =
        command.getPacket<SessionGenIDsReplyPacket>();
    EQVERB << "Cmd gen IDs reply: " << packet << std::endl;

    _localNode->serveRequest( packet->requestID, packet->firstID );

    const size_t additional = packet->allocated - packet->requested;
    if( packet->firstID != EQ_ID_INVALID && additional > 0 )
        // Merge additional identifiers into local pool
        _idPool.freeIDs( packet->firstID + packet->requested, additional );

    return true;
}

bool Session::_cmdSetIDMaster( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionSetIDMasterPacket* packet = 
        command.getPacket<SessionSetIDMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd set ID master: " << packet << std::endl;

    const NodeID& nodeID = packet->masterID;
    EQASSERT( nodeID != NodeID::ZERO );

    base::ScopedMutex< base::SpinLock > mutex( _idMasters );
    _idMasters.data[ packet->identifier ] = nodeID;

    ackRequest( command.getNode(), packet->requestID );
    return true;
}

bool Session::_cmdUnsetIDMaster( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionUnsetIDMasterPacket* packet = 
        command.getPacket<SessionUnsetIDMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unset ID master: " << packet << std::endl;

    {
        base::ScopedMutex< base::SpinLock > mutex( _idMasters );
        _idMasters->erase( packet->identifier );
    }

    ackRequest( command.getNode(), packet->requestID );
    return true;
}

bool Session::_cmdGetIDMaster( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionGetIDMasterPacket* packet =
        command.getPacket<SessionGetIDMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "handle get idMaster " << packet << std::endl;

    SessionGetIDMasterReplyPacket reply( packet );
    reply.masterID = _pollIDMaster( packet->identifier );
    send( command.getNode(), reply );

    return true;
}

bool Session::_cmdGetIDMasterReply( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionGetIDMasterReplyPacket* packet = 
        command.getPacket<SessionGetIDMasterReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "handle get idMaster reply " << packet << std::endl;

    const NodeID& nodeID = packet->masterID;

    if( nodeID != NodeID::ZERO )
    {
        base::ScopedMutex< base::SpinLock > mutex( _idMasters );
        _idMasters.data[ packet->identifier ] = nodeID;
    }
    // else not found

    _localNode->serveRequest( packet->requestID );
    return true;
}

bool Session::_cmdAttachObject( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const SessionAttachObjectPacket* packet = 
        command.getPacket<SessionAttachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd attach object " << packet << std::endl;

    Object* object = static_cast< Object* >( _localNode->getRequestData( 
                                                 packet->requestID ));
    _attachObject( object, packet->objectID, packet->objectInstanceID );
    _localNode->serveRequest( packet->requestID );
    return true;
}

bool Session::_cmdDetachObject( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const SessionDetachObjectPacket* packet = 
        command.getPacket<SessionDetachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd detach object " << packet << std::endl;

    const uint32_t id = packet->objectID;
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

bool Session::_cmdMapObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
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
        const ObjectInstanceDataIStreamDeque& versions = cached.versions;
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
    return true;
}

bool Session::_cmdSubscribeObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd subscribe object " << packet << std::endl;

    NodePtr        node = command.getNode();
    const uint32_t id   = packet->objectID;

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
    
    SessionSubscribeObjectReplyPacket reply( packet );
    reply.nodeID = node->getNodeID();

    if( master )
    {
        // Check requested version
        const uint32_t version = packet->requestedVersion;
        const uint32_t oldestVersion = master->getOldestVersion();
        if( version == VERSION_OLDEST || version == VERSION_NONE ||
            version >= oldestVersion)
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
            EQWARN
                << "Version " << version << " of " << base::className( master )
                << " " << id << " no longer available (have v"
                << master->getOldestVersion() << ".." << master->getVersion()
                << ")" << std::endl;
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
    return true;
}

bool Session::_cmdSubscribeObjectSuccess( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const SessionSubscribeObjectSuccessPacket* packet = 
        command.getPacket<SessionSubscribeObjectSuccessPacket>();

    // Subscribe success packets are potentially multicasted (see above)
    // verify that we are the intended receiver
    const NodeID& nodeID = packet->nodeID;
    if( nodeID != _localNode->getNodeID( ))
        return true;

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
    return true;
}

bool Session::_cmdSubscribeObjectReply( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    const SessionSubscribeObjectReplyPacket* packet = 
        command.getPacket<SessionSubscribeObjectReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd subscribe object reply " << packet
                         << std::endl;

    // Subscribe reply packets are potentially multicasted (see above)
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
    return true;
}

bool Session::_cmdUnsubscribeObject( Command& command )
{
    EQ_TS_THREAD( _commandThread );
    SessionUnsubscribeObjectPacket* packet =
        command.getPacket<SessionUnsubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unsubscribe object  " << packet << std::endl;

    NodePtr        node = command.getNode();
    const uint32_t id   = packet->objectID;

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

    SessionDetachObjectPacket detachPacket( packet );
    send( node, detachPacket );
    return true;
}

bool Session::_cmdUnmapObject( Command& command )
{
    EQ_TS_THREAD( _receiverThread );
    const SessionUnmapObjectPacket* packet = 
        command.getPacket< SessionUnmapObjectPacket >();

    EQLOG( LOG_OBJECTS ) << "Cmd unmap object " << packet << std::endl;

    _instanceCache.erase( packet->objectID );

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
        object->detachFromSession();
    }

    return true;
}

bool Session::_cmdInstance( Command& command )
{
    ObjectInstancePacket* packet = command.getPacket< ObjectInstancePacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd instance " << packet << std::endl;

    EQ_TS_THREAD( _commandThread );
    EQASSERT( _localNode.isValid( ));

    packet->type = PACKETTYPE_EQNET_OBJECT;
    packet->command = CMD_OBJECT_INSTANCE;

    uint32_t usage = 0;
    bool result = true;

    if( packet->nodeID == _localNode->getNodeID( ))
    {
        EQASSERT( packet->instanceID <= EQ_ID_MAX );

        usage = 1; // TODO correct usage count
        command.setDispatchID( packet->instanceID );
        result = _invokeObjectCommand( command );
    }

    const ObjectVersion rev( packet->objectID, packet->version ); 
    _instanceCache.add( rev, packet->masterInstanceID, command, usage );
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
