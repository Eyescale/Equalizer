
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

#include "session.h"

#include "barrier.h"
#include "command.h"
#include "connection.h"
#include "connectionDescription.h"
#include "log.h"
#include "objectCM.h"
#include "packets.h"
#include "session.h"

#ifndef WIN32
#  include <alloca.h>
#endif

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
#define MIN_ID_RANGE 1024

Session::Session()
        : _requestHandler( true /* threadSafe */ )
        , _id(EQ_ID_INVALID)
        , _isMaster(false)
        , _idPool( 0 ) // Master pool is filled in Node::registerSession
        , _instanceIDs( 0 ) 
{
    EQINFO << "New Session @" << (void*)this << endl;
}

Session::~Session()
{
    EQINFO << "Delete Session @" << (void*)this << endl;
    EQASSERTINFO( _id == EQ_ID_INVALID, "Session still mapped during deletion");

    _id        = EQ_ID_INVALID;
    _isMaster  = false;
    _localNode = 0;
    _server    = 0;
    
#ifndef NDEBUG
    if( !_objects.empty( ))
    {
        EQWARN << _objects.size() << " attached objects in destructor" << endl;
        
        for( ObjectVectorHash::const_iterator i = _objects.begin();
             i != _objects.end(); ++i )
        {
            const ObjectVector& objects = i->second;
            EQWARN << "  " << objects.size() << " objects with id " 
                   << i->first << endl;
            
            for( ObjectVector::const_iterator j = objects.begin();
                 j != objects.end(); ++j )
            {
                const Object* object = *j;
                EQINFO << "    object type " << typeid(*object).name() 
                       << endl;
            }
        }
    }
#endif
    _objects.clear();
}

void Session::_setLocalNode( NodePtr node )
{
    EQASSERT( _requestHandler.isEmpty( ));

    _localNode = node;
    if( !_localNode.isValid( ))
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
                     CommandFunc<Session>( this, &Session::_cmdAckRequest ), 0);
    registerCommand( CMD_SESSION_GEN_IDS, 
                     CommandFunc<Session>( this, &Session::_cmdGenIDs ),
                     queue );
    registerCommand( CMD_SESSION_GEN_IDS_REPLY,
                     CommandFunc<Session>( this, &Session::_cmdGenIDsReply ),
                     queue );
    registerCommand( CMD_SESSION_SET_ID_MASTER,
                     CommandFunc<Session>( this, &Session::_cmdSetIDMaster ),
                     queue );
    registerCommand( CMD_SESSION_GET_ID_MASTER, 
                     CommandFunc<Session>( this, &Session::_cmdGetIDMaster ),
                     queue );
    registerCommand( CMD_SESSION_GET_ID_MASTER_REPLY,
                  CommandFunc<Session>( this, &Session::_cmdGetIDMasterReply ),
                     queue );
    registerCommand( CMD_SESSION_ATTACH_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdAttachObject ),
                     0 );
    registerCommand( CMD_SESSION_DETACH_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdDetachObject ),
                     0 );
    registerCommand( CMD_SESSION_MAP_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdMapObject ), 0 );
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT,
                   CommandFunc<Session>( this, &Session::_cmdSubscribeObject ),
                     queue );
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS,
            CommandFunc<Session>( this, &Session::_cmdSubscribeObjectSuccess ),
                     0 );
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT_REPLY,
              CommandFunc<Session>( this, &Session::_cmdSubscribeObjectReply ),
                     queue );
    registerCommand( CMD_SESSION_UNSUBSCRIBE_OBJECT,
                  CommandFunc<Session>( this, &Session::_cmdUnsubscribeObject ),
                     queue );
}

//---------------------------------------------------------------------------
// identifier generation
//---------------------------------------------------------------------------
uint32_t Session::genIDs( const uint32_t range )
{
    uint32_t id = _idPool.genIDs( range );
    if( id != EQ_ID_INVALID || _isMaster )
    {
        if( id == EQ_ID_INVALID )
            EQWARN << "Out of session identifiers" << std::endl;
        return id;
    }

    SessionGenIDsPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.range     = range;

    send( packet );
    _requestHandler.waitRequest( packet.requestID, id );
    
    if( id == EQ_ID_INVALID )
        EQWARN << "Out of session identifiers" << std::endl;

    return id;
}

void Session::freeIDs( const uint32_t start, const uint32_t range )
{
    _idPool.freeIDs( start, range );
    // TODO: could return IDs to master sometimes ?
}

//---------------------------------------------------------------------------
// identifier master node mapping
//---------------------------------------------------------------------------
void Session::setIDMaster( const uint32_t id, const NodeID& master )
{
    CHECK_NOT_THREAD( _commandThread );
    _setIDMasterSync( _setIDMasterNB( id, master ));
}

uint32_t Session::_setIDMasterNB( const uint32_t id, const NodeID& master )
{
    CHECK_NOT_THREAD( _commandThread );

    SessionSetIDMasterPacket packet;
    packet.id       = id;
    packet.masterID = master;
    packet.masterID.convertToNetwork();
    
    if( !_isMaster )
        _sendLocal( packet ); // set on our slave instance (fire&forget)

    packet.requestID = _requestHandler.registerRequest();
    send( packet );       // set on master instance (need to wait for ack)
    return packet.requestID;
}

void Session::_setIDMasterSync( const uint32_t requestID )
{
    _requestHandler.waitRequest( requestID );
}

const NodeID& Session::_pollIDMaster( const uint32_t id ) const 
{
    NodeIDHash::const_iterator i = _idMasters.find( id );
    if( i == _idMasters.end( ))
        return NodeID::ZERO;

    return i->second;
}

const NodeID& Session::getIDMaster( const uint32_t id )
{
    _idMasterMutex.set();
    const NodeID& master = _pollIDMaster( id );
    _idMasterMutex.unset();
        
    if( master != NodeID::ZERO || _isMaster )
        return master;

    // ask session master instance
    SessionGetIDMasterPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.id        = id;

    send( packet );
    _requestHandler.waitRequest( packet.requestID );

    ScopedMutex mutex( _idMasterMutex );
    EQLOG( LOG_OBJECTS ) << "Master node for id " << id << ": " 
        << _pollIDMaster( id ) << endl;
    return _pollIDMaster( id );
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
    packet.requestID = _requestHandler.registerRequest( object );

    _sendLocal( packet );
    _requestHandler.waitRequest( packet.requestID );
}

void Session::_attachObject( Object* object, const uint32_t id, 
                             const uint32_t inInstanceID )
{
    EQASSERT( object );
    CHECK_THREAD( _receiverThread );

    uint32_t instanceID = inInstanceID;
    if( inInstanceID == EQ_ID_INVALID )
    {
        _instanceIDs = ( _instanceIDs + 1 ) % IDPool::MAX_CAPACITY;
        instanceID = _instanceIDs;
    }
    EQASSERT( instanceID <= _instanceIDs );

    object->attachToSession( id, instanceID, this );

    _objectsMutex.set();
    ObjectVector& objects = _objects[ id ];
    objects.push_back( object );
    _objectsMutex.unset();

    getLocalNode()->flushCommands(); // redispatch pending commands
    EQLOG( LOG_OBJECTS ) << "Attached " << typeid( *object ).name()
                         << " to id " << id << endl;
}

void Session::detachObject( Object* object )
{
    EQASSERT( object );
    CHECK_NOT_THREAD( _receiverThread );

    SessionDetachObjectPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.objectID  = object->getID();
    packet.objectInstanceID  = object->getInstanceID();

    _sendLocal( packet );
    _requestHandler.waitRequest( packet.requestID );
}

void Session::_detachObject( Object* object )
{
    EQASSERT( object );
    CHECK_THREAD( _receiverThread );

    const uint32_t id = object->getID();
    EQASSERT( id != EQ_ID_INVALID );
    EQASSERT( _objects.find( id ) != _objects.end( ));

    EQLOG( LOG_OBJECTS ) << "Detach " << typeid( *object ).name() 
                         << " from id " << id << endl;

    ObjectVector&          objects = _objects[ id ];
    ObjectVector::iterator iter = find( objects.begin(),objects.end(), object );
    EQASSERT( iter != objects.end( ));

    _objectsMutex.set();

    objects.erase( iter );
    if( objects.empty( ))
        _objects.erase( id );

    _objectsMutex.unset();
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
    EQASSERT( object );
    EQLOG( LOG_OBJECTS ) << "Mapping " << typeid( *object ).name() << " to id "
                         << id << " version " << version << endl;

    EQASSERT( object->getID() == EQ_ID_INVALID );
    EQASSERT( id != EQ_ID_INVALID );
    EQASSERT( !_localNode->inCommandThread( ));
        
    NodeID masterNodeID;
    if( !object->isMaster( ))
    {
        // Connect master node, can't do that from the command thread!
        masterNodeID = getIDMaster( id );
        if( masterNodeID == NodeID::ZERO )
        {
            EQWARN << "Can't find master node for object id " << id << endl;
            return EQ_ID_INVALID;
        }

        NodePtr master = _localNode->connect( masterNodeID );
        if( !master || master->getState() == Node::STATE_STOPPED )
        {
            EQWARN << "Can't connect master node with id " << masterNodeID
                   << " for object id " << id << endl;
            return EQ_ID_INVALID;
        }
    }

    SessionMapObjectPacket packet;
    packet.requestID    = _requestHandler.registerRequest( object );
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

    void* data = _requestHandler.getRequestData( requestID );    
    if( data == 0 )
        return false;

    Object* object = EQSAFECAST( Object*, data );

    uint32_t version = Object::VERSION_NONE;
    _requestHandler.waitRequest( requestID, version );

    const bool mapped = ( object->getID() != EQ_ID_INVALID );
    if( mapped && !object->isMaster( ))
    {
        object->_cm->applyMapData(); // apply instance data on slave instances
        if( version != Object::VERSION_OLDEST && 
            version != Object::VERSION_NONE )
        {
            object->sync( version );
        }
    }

    EQLOG( LOG_OBJECTS ) << "Mapped " << typeid( *object ).name() << " to id " 
                         << object->getID() << endl;
    return mapped;
}

void Session::unmapObject( Object* object )
{
    const uint32_t id = object->getID();
    if( id == EQ_ID_INVALID ) // not registered
        return;

    EQLOG( LOG_OBJECTS ) << "Unmap " << typeid( *object ).name() << " from id "
        << object->getID() << endl;

    // Slave: send unsubscribe to master. Master will send detach packet.
    if( !object->isMaster( ))
    {
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
            if( master.isValid( ))
            {
                SessionUnsubscribeObjectPacket packet;
                packet.requestID = _requestHandler.registerRequest();
                packet.objectID  = id;
                packet.masterInstanceID = masterInstanceID;
                packet.slaveInstanceID  = object->getInstanceID();
                send( master, packet );

                _requestHandler.waitRequest( packet.requestID );
                return;
            }
            EQERROR << "Master node for object id " << id << " not connected"
                    << endl;
        }
    }

    // Master (or no unsubscribe sent): Detach directly
    detachObject( object );
}

bool Session::registerObject( Object* object )
{
    EQASSERT( object->getID() == EQ_ID_INVALID );

    const uint32_t id = genIDs( 1 );
    EQASSERT( id != EQ_ID_INVALID );
    if( id == EQ_ID_INVALID )
        return false;

    const uint32_t requestID = _setIDMasterNB( id, _localNode->getNodeID( ));
    object->setupChangeManager( object->getChangeType(), true );

    EQCHECK( mapObject( object, id ));
    _setIDMasterSync( requestID ); // sync, master knows our ID now
    EQLOG( LOG_OBJECTS ) << "Registered " << typeid( *object ).name()
                         << " to id " << id << endl;
    return true;
}

void Session::deregisterObject( Object* object )
{
    const uint32_t id = object->getID();

    EQLOG( LOG_OBJECTS ) << "Deregister " << typeid( *object ).name() 
                         << " from id " << id << endl;

    // TODO unsetIDMaster ?
    unmapObject( object );
    freeIDs( id, 1 );
}

//===========================================================================
// Packet handling
//===========================================================================
bool Session::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << endl;
    EQASSERT( command.isValid( ));
    CHECK_THREAD( _receiverThread );

    switch( command->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            return Dispatcher::dispatchCommand( command );

        case DATATYPE_EQNET_OBJECT:
        {
            EQASSERT( command.isValid( ));
            const ObjectPacket* objPacket = command.getPacket<ObjectPacket>();
            const uint32_t      id        = objPacket->objectID;


            if( _objects.find( id ) == _objects.end( ))
            {
                EQVERB << "no objects to dispatch command, redispatching " 
                       << objPacket << endl;
                return false;
            }
            EQASSERTINFO( !_objects[id].empty(), id );

            Object* object = _objects[id][0];
            EQASSERT( object );

            return object->dispatchCommand( command );
        }

        default:
            EQABORT( "Unknown datatype " << command->datatype << " for "
                     << command );
            return true;
    }
}

CommandResult Session::invokeCommand( Command& command )
{
    EQVERB << "invoke " << command << endl;
    EQASSERT( command.isValid( ));

    switch( command->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            return Dispatcher::invokeCommand( command );

        case DATATYPE_EQNET_OBJECT:
            return _invokeObjectCommand( command );

        default:
            EQWARN << "Unhandled command " << command << endl;
            return COMMAND_ERROR;
    }
}

CommandResult Session::_invokeObjectCommand( Command& command )
{
    EQASSERT( command.isValid( ));
    const ObjectPacket* objPacket = command.getPacket<ObjectPacket>();
    const uint32_t      id        = objPacket->objectID;

    _objectsMutex.set();

    EQASSERTINFO( _objects.find( id ) != _objects.end(), 
                  "No objects to handle command " << objPacket );

    // create copy of objects vector for thread-safety
    ObjectVector objects = _objects[id];
    EQASSERTINFO( !objects.empty(), objPacket );

    _objectsMutex.unset();

    for( ObjectVector::const_iterator i = objects.begin();
         i != objects.end(); ++ i )
    {
        Object* object = *i;
        const bool isInstance = 
            ( objPacket->instanceID == object->getInstanceID( ));

        if( objPacket->instanceID == EQ_ID_ANY || isInstance )
        {
            if( !command.isValid( ))
            {
                // NOTE: command got invalidated (last object was a pushed
                // command to another thread) . Object should push copy of
                // command, or we should make it clear what invalidating a
                // command means here (same as discard?)
                EQERROR << "Object of type " << typeid(*object).name()
                        << " invalidated command send all instances" << endl;
                return COMMAND_ERROR;
            }

            const CommandResult result = object->invokeCommand( command );
            switch( result )
            {
                case COMMAND_DISCARD:
                    return COMMAND_DISCARD;

                case COMMAND_ERROR:
                    EQERROR << "Error handling command " << objPacket
                            << " for object of type " << typeid(*object).name()
                            << endl;
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
    if( objPacket->instanceID == EQ_ID_ANY )
        return COMMAND_HANDLED;

    EQWARN << "instance not found for " << objPacket << endl;
    return COMMAND_ERROR;
}

CommandResult Session::_cmdAckRequest( Command& command )
{
    const SessionAckRequestPacket* packet = 
        command.getPacket<SessionAckRequestPacket>();
    EQASSERT( packet->requestID != EQ_ID_INVALID );

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}


CommandResult Session::_cmdGenIDs( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGenIDsPacket* packet =command.getPacket<SessionGenIDsPacket>();
    EQVERB << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );
    const uint32_t range = EQ_MAX( packet->range, MIN_ID_RANGE );

    reply.id = _idPool.genIDs( range );
    reply.allocated = range;
    send( command.getNode(), reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGenIDsReplyPacket* packet =
        command.getPacket<SessionGenIDsReplyPacket>();
    EQVERB << "Cmd gen IDs reply: " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, packet->id );

    const size_t additional = packet->allocated - packet->requested;
    if( packet->id != EQ_ID_INVALID && additional > 0 )
        // Merge additional identifiers into local pool
        _idPool.freeIDs( packet->id + packet->requested, additional );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionSetIDMasterPacket* packet = 
        command.getPacket<SessionSetIDMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd set ID master: " << packet << endl;

    NodeID nodeID = packet->masterID;
    nodeID.convertToHost();
    EQASSERT( nodeID != NodeID::ZERO );

    ScopedMutex mutex( _idMasterMutex );
    _idMasters[ packet->id ] = nodeID;

    if( packet->requestID != EQ_ID_INVALID ) // need to ack set operation
    {
        NodePtr node = command.getNode();

        if( node == _localNode ) // OPT
            _requestHandler.serveRequest( packet->requestID );
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
    EQLOG( LOG_OBJECTS ) << "handle get idMaster " << packet << endl;

    SessionGetIDMasterReplyPacket reply( packet );
    reply.masterID = _pollIDMaster( packet->id );

    send( command.getNode(), reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMasterReply( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionGetIDMasterReplyPacket* packet = 
        command.getPacket<SessionGetIDMasterReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "handle get idMaster reply " << packet << endl;

    NodeID nodeID = packet->masterID;
    nodeID.convertToHost();

    if( nodeID != NodeID::ZERO )
    {
        ScopedMutex mutex( _idMasterMutex );
        _idMasters[ packet->id ] = packet->masterID;
    }
    // else not found

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdAttachObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionAttachObjectPacket* packet = 
        command.getPacket<SessionAttachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd attach object " << packet << endl;

    Object* object =  static_cast<Object*>( _requestHandler.getRequestData( 
                                                packet->requestID ));
    _attachObject( object, packet->objectID, packet->objectInstanceID );
    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdDetachObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionDetachObjectPacket* packet = 
        command.getPacket<SessionDetachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd detach object " << packet << endl;

    const uint32_t id = packet->objectID;
    if( _objects.find( id ) != _objects.end( ))
    {
        ObjectVector& objects = _objects[id];

        for( ObjectVector::const_iterator i = objects.begin();
            i != objects.end(); ++i )
        {
            Object* object = *i;
            if( object->getInstanceID() == packet->objectInstanceID )
            {
                _detachObject( object );
                break;
            }
        }
    }

    if( packet->requestID != EQ_ID_INVALID )
        _requestHandler.serveRequest( packet->requestID );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdMapObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionMapObjectPacket* packet = 
        command.getPacket< SessionMapObjectPacket >();
    EQLOG( LOG_OBJECTS ) << "Cmd map object " << packet << endl;

    Object* object = static_cast<Object*>( _requestHandler.getRequestData( 
                                               packet->requestID ));    
    EQASSERT( object );
    const uint32_t id = packet->objectID;

    if( !object->isMaster( ))
    { 
        EQASSERT( packet->masterNodeID != NodeID::ZERO );
        NodePtr master = _localNode->getNode( packet->masterNodeID );

        EQASSERTINFO( master.isValid(), "Master node for object id " << id
                      << " not connected" );

        _instanceIDs = ( _instanceIDs + 1 ) % IDPool::MAX_CAPACITY;

        // slave instantiation - subscribe first
        SessionSubscribeObjectPacket subscribePacket( packet );
        subscribePacket.instanceID = _instanceIDs;

        send( master, subscribePacket );
        return COMMAND_HANDLED;
    }

    _attachObject( object, id, EQ_ID_INVALID );

    EQLOG( LOG_OBJECTS ) << "mapped object id " << object->getID() << " @" 
                         << (void*)object << " is " << typeid(*object).name()
                         << endl;

    _requestHandler.serveRequest( packet->requestID, packet->version );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObject( Command& command )
{
    CHECK_THREAD( _commandThread );
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd subscribe object " << packet << endl;

    NodePtr        node = command.getNode();
    const uint32_t id   = packet->objectID;

    Object* master = 0;

    _objectsMutex.set();
    if( _objects.find( id ) != _objects.end( ))
    {
        ObjectVector& objects = _objects[id];

        for( ObjectVector::const_iterator i = objects.begin();
             i != objects.end(); ++i )
        {
            Object* object = *i;
            if( object->isMaster( ))
            {
                master = object;
                break;
            }
        }
    }
    _objectsMutex.unset();
    
    SessionSubscribeObjectReplyPacket reply( packet );

    if( master )
    {
        // Check requested version
        const uint32_t version = packet->version;
        if( version == Object::VERSION_OLDEST || 
            version >= master->getOldestVersion( ))
        {
            SessionSubscribeObjectSuccessPacket successPacket( packet );
            successPacket.changeType       = master->getChangeType();
            successPacket.masterInstanceID = master->getInstanceID();
            send( node, successPacket );
        
            master->addSlave( node, packet->instanceID, version );
            reply.result = true;
        }
        else
        {
            EQWARN << "Version " << version << " no longer available" << endl;
            reply.result = false;
        }
    }
    else
    {
        EQWARN << "Can't find master object to subscribe " << id << endl;
        reply.result = false;
    }

    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObjectSuccess( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionSubscribeObjectSuccessPacket* packet = 
        command.getPacket<SessionSubscribeObjectSuccessPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd object subscribe success " << packet << endl;

    Object* object = static_cast<Object*>( _requestHandler.getRequestData( 
                                               packet->requestID ));    
    EQASSERT( object );
    EQASSERT( !object->isMaster( ));

    object->setupChangeManager( 
        static_cast< Object::ChangeType >( packet->changeType ), false, 
        packet->masterInstanceID );

    _attachObject( object, packet->objectID, packet->instanceID );

    EQLOG( LOG_OBJECTS ) << "subscribed object id " << object->getID() << '.'
                         << object->getInstanceID() << " cm " 
                         << typeid( *(object->_cm)).name() << " @" 
                         << static_cast< void* >( object ) << " is "
                         << typeid(*object).name() << endl;
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObjectReply( Command& command )
{
    CHECK_THREAD( _commandThread );
    const SessionSubscribeObjectReplyPacket* packet = 
        command.getPacket<SessionSubscribeObjectReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd object subscribe reply " << packet << endl;

    EQASSERT( _requestHandler.getRequestData( packet->requestID ));

    if( !packet->requestID )
        EQWARN << "Could not subscribe object " << packet->objectID << endl;

    _requestHandler.serveRequest( packet->requestID, packet->version );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnsubscribeObject( Command& command )
{
    CHECK_THREAD( _commandThread );
    SessionUnsubscribeObjectPacket* packet =
        command.getPacket<SessionUnsubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unsubscribe object  " << packet << endl;

    NodePtr        node = command.getNode();
    const uint32_t id   = packet->objectID;

    _objectsMutex.set();
    if( _objects.find( id ) != _objects.end( ))
    {
        ObjectVector& objects = _objects[id];

        for( ObjectVector::const_iterator i = objects.begin();
             i != objects.end(); ++ i )
        {
            Object* object = *i;
            if( object->isMaster() && 
                object->getInstanceID() == packet->masterInstanceID )
            {
                object->removeSlave( node );
                break;
            }
        }   
    }
    _objectsMutex.unset();

    SessionDetachObjectPacket detachPacket( packet );
    send( node, detachPacket );
    return COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, Session* session )
{
    if( !session )
    {
        os << "NULL session";
        return os;
    }
    
    os << "session " << session->getID() << "(" << (void*)session
       << ")";

    return os;
}
}
}
