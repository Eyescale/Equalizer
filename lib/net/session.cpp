/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"

#include "barrier.h"
#include "command.h"
#include "connection.h"
#include "connectionDescription.h"
#include "log.h"
#include "packets.h"
#include "session.h"

#ifndef WIN32
#  include <alloca.h>
#endif

using namespace eqBase;
using namespace eqNet;
using namespace std;

#define MIN_ID_RANGE 1024

Session::Session( const bool threadSafe )
        : _id(EQ_ID_INVALID),
          _server(0),
          _isMaster(false),
          _requestHandler( threadSafe ),
          _masterPool( IDPool::MAX_CAPACITY ),
          _localPool( 0 ),
          _instanceIDs( IDPool::MAX_CAPACITY ) 
{
    registerCommand( CMD_SESSION_GEN_IDS, 
                     CommandFunc<Session>( this, &Session::_cmdGenIDs ));
    registerCommand( CMD_SESSION_GEN_IDS_REPLY,
                     CommandFunc<Session>( this, &Session::_cmdGenIDsReply ));
    registerCommand( CMD_SESSION_SET_ID_MASTER,
                     CommandFunc<Session>( this, &Session::_cmdSetIDMaster ));
    registerCommand( CMD_SESSION_GET_ID_MASTER, 
                     CommandFunc<Session>( this, &Session::_cmdGetIDMaster ));
    registerCommand( CMD_SESSION_GET_ID_MASTER_REPLY,
                  CommandFunc<Session>( this, &Session::_cmdGetIDMasterReply ));
    registerCommand( CMD_SESSION_ATTACH_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdAttachObject ));
    registerCommand( CMD_SESSION_DETACH_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdDetachObject ));
    registerCommand( CMD_SESSION_MAP_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdMapObject ));
    registerCommand( CMD_SESSION_MAP_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdMapObject ));
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT,
                   CommandFunc<Session>( this, &Session::_cmdSubscribeObject ));
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS,
            CommandFunc<Session>( this, &Session::_cmdSubscribeObjectSuccess ));
    registerCommand( CMD_SESSION_SUBSCRIBE_OBJECT_REPLY,
              CommandFunc<Session>( this, &Session::_cmdSubscribeObjectReply ));
    registerCommand( CMD_SESSION_UNMAP_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdUnmapObject ));
    registerCommand( CMD_SESSION_UNSUBSCRIBE_OBJECT,
                 CommandFunc<Session>( this, &Session::_cmdUnsubscribeObject ));
    registerCommand( CMD_SESSION_UNSUBSCRIBE_OBJECT_REPLY,
            CommandFunc<Session>( this, &Session::_cmdUnsubscribeObjectReply ));

    EQINFO << "New Session @" << (void*)this << endl;
}

Session::~Session()
{
    EQINFO << "Delete Session @" << (void*)this << endl;

    _localNode = 0;
    _server    = 0;
        
    if( !_objects.empty( ))
    {
        if( eqBase::Log::level >= eqBase::LOG_WARN ) // OPT
        {
            EQWARN << _objects.size()
                   << " attached objects in destructor" << endl;

            for( IDHash< vector<Object*> >::const_iterator i =
                     _objects.begin(); 
                 i != _objects.end(); ++i )
            {
                const vector<Object*>& objects = i->second;
                EQWARN << "  " << objects.size() << " objects with id " 
                       << i->first << endl;

                for( vector<Object*>::const_iterator j = objects.begin();
                     j != objects.end(); ++j )
                {
                    const Object* object = *j;
                    EQINFO << "    Object of type " << typeid(*object).name() 
                           << endl;
                }
            }
        }
    }
    _objects.clear();
}

//---------------------------------------------------------------------------
// identifier generation
//---------------------------------------------------------------------------
uint32_t Session::genIDs( const uint32_t range )
{
    if( _isMaster && _server->inReceiverThread( ))
        return _masterPool.genIDs( range );

    uint32_t id = _localPool.genIDs( range );
    if( id != EQ_ID_INVALID )
        return id;

    SessionGenIDsPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.range     = MAX(range, MIN_ID_RANGE);

    send( packet );
    _requestHandler.waitRequest( packet.requestID, id );

    if( id == EQ_ID_INVALID || range >= MIN_ID_RANGE )
        return id;

    // We allocated more IDs than requested - let the pool handle the details
    _localPool.freeIDs( id, MIN_ID_RANGE );
    return _localPool.genIDs( range );
}

void Session::freeIDs( const uint32_t start, const uint32_t range )
{
    _localPool.freeIDs( start, range );
    // TODO: could return IDs to master sometimes ?
}

//---------------------------------------------------------------------------
// identifier master node mapping
//---------------------------------------------------------------------------
void Session::setIDMaster( const uint32_t start, const uint32_t range, 
                           const NodeID& master )
{
    IDMasterInfo info = { start, start+range, master };
    _idMasterInfos.push_back( info );

    if( _isMaster )
        return;

    SessionSetIDMasterPacket packet;
    packet.start    = start;
    packet.range    = range;
    packet.masterID = master;

    send( packet );
}

const NodeID& Session::_pollIDMaster( const uint32_t id ) const 
{
    for( vector<IDMasterInfo>::const_iterator i = _idMasterInfos.begin();
         i != _idMasterInfos.end(); ++i )
    {
        const IDMasterInfo& info = *i;
        if( id >= info.start && id < info.end )
            return info.master;
    }
    return NodeID::ZERO;
}

RefPtr<Node> Session::_pollIDMasterNode( const uint32_t id ) const
{
    const NodeID& nodeID = _pollIDMaster( id );
    return _localNode->getNode( nodeID );
}

const NodeID& Session::getIDMaster( const uint32_t id )
{
    const NodeID& master = _pollIDMaster( id );
        
    if( master != NodeID::ZERO || _isMaster )
        return master;

    // ask session master instance
    SessionGetIDMasterPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.id        = id;

    send( packet );
    _requestHandler.waitRequest( packet.requestID );
    EQINFO << "Master node for id " << id << ": " << _pollIDMaster( id ) <<endl;
    return _pollIDMaster( id );
}

//---------------------------------------------------------------------------
// object mapping
//---------------------------------------------------------------------------
struct MapObjectData
{
    Object*      object;
    uint32_t     objectID;
    RefPtr<Node> master;
};

void Session::attachObject( Object* object, const uint32_t id )
{
    EQASSERT( object );

    if( _localNode->inReceiverThread( ))
    {
        CHECK_THREAD( _receiverThread );

        object->_id         = id;
        object->_instanceID = _instanceIDs.genIDs(1);
        object->_session    = this;

        _objectsMutex.set();
        vector<Object*>& objects = _objects[ id ];
        _objectsMutex.unset();
        objects.push_back( object );

        EQINFO << "Attached " << typeid( *object ).name() << " to id " << id
               << endl;
        return;
    }
    // else

    SessionAttachObjectPacket packet;
    packet.objectID  = id;
    packet.requestID = _requestHandler.registerRequest( object );
    _sendLocal( packet );
    _requestHandler.waitRequest( packet.requestID );
}

void Session::detachObject( Object* object )
{
    EQASSERT( object );
    if( _localNode->inReceiverThread( ))
    {
        CHECK_THREAD( _receiverThread );

        const uint32_t            id      = object->getID();
        EQASSERT( id != EQ_ID_INVALID );
        EQASSERT( _objects.find( id ) != _objects.end( ));

        EQINFO << "Detach " << typeid( *object ).name() << " from id " << id
               << endl;

        vector<Object*>&          objects = _objects[ id ];
        vector<Object*>::iterator iter    = find( objects.begin(),objects.end(),
                                                  object );
        EQASSERT( iter != objects.end( ));
        objects.erase( iter );

        if( objects.empty( ))
        {
            _objectsMutex.set();
            _objects.erase( id );
            _objectsMutex.unset();
        }

        EQASSERT( object->_instanceID != EQ_ID_INVALID );
        _instanceIDs.freeIDs( object->_instanceID, 1 );

        object->_id         = EQ_ID_INVALID;
        object->_instanceID = EQ_ID_INVALID;
        object->_session    = 0;
        object->_setChangeManager( ObjectCM::ZERO );
        return;
    }
    // else

    SessionDetachObjectPacket packet;
    packet.requestID = _requestHandler.registerRequest( object );
    _sendLocal( packet );
    _requestHandler.waitRequest( packet.requestID );    
}

bool Session::mapObject( Object* object, const uint32_t id )
{
    EQASSERT( object );

    EQINFO << "Mapping " << typeid( *object ).name() << " to id " << id << endl;

    EQASSERT( object->_id == EQ_ID_INVALID );
    EQASSERT( id != EQ_ID_INVALID );
    EQASSERT( !_localNode->inReceiverThread( ));
        
    RefPtr<Node> master;
    if( !object->isMaster( ))
    {
        EQASSERTINFO( object->_cm == ObjectCM::ZERO, typeid( *object ).name( ));

        const NodeID& masterID = getIDMaster( id );
        if( masterID == NodeID::ZERO )
        {
            EQWARN << "Can't find master node for object id " << id << endl;
            return false;
        }

        master = _localNode->connect( masterID, getServer( ));
        if( !master || master->getState() == Node::STATE_STOPPED )
        {
            EQWARN << "Can't connect master node with id " << masterID
                   << " for object id " << id << endl;
            return false;
        }
    }

    MapObjectData data = { object, id, master };
    SessionMapObjectPacket packet;
    packet.requestID = _requestHandler.registerRequest( &data );

    _sendLocal( packet );
    _requestHandler.waitRequest( packet.requestID );

    EQINFO << "Mapped " << typeid( *object ).name() << " to id " << id << endl;
    return( object->getID() != EQ_ID_INVALID );
}

void Session::unmapObject( Object* object )
{
    EQASSERT( object->getID() != EQ_ID_INVALID );
    EQASSERT( !_localNode->inReceiverThread( ));

    SessionUnmapObjectPacket packet;
    packet.requestID = _requestHandler.registerRequest( object );

    EQINFO << "Unmap " << typeid( *object ).name() << " from id "
           << object->getID() << endl;
    _sendLocal( packet );
    _requestHandler.waitRequest( packet.requestID );
}

void Session::registerObject( Object* object )
{
    EQASSERT( object->_id == EQ_ID_INVALID );

    const uint32_t id = genIDs( 1 );
    EQASSERT( id != EQ_ID_INVALID );

    setIDMaster( id, 1, _localNode->getNodeID( ));

    object->_setupChangeManager( object->_getChangeManagerType(), true );
    mapObject( object, id );
    EQINFO << "Registered " << typeid( *object ).name() << " to id " << id 
           << endl;
}

void Session::deregisterObject( Object* object )
{
    const uint32_t id = object->getID();

    EQINFO << "Deregister " << typeid( *object ).name() << " from id " << id
           << endl;

    // TODO unsetIDMaster ?
    unmapObject( object );
    freeIDs( id, 1 );
}

//===========================================================================
// Packet handling
//===========================================================================
CommandResult Session::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << command << endl;
    EQASSERT( command.isValid( ));

    switch( command->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            return invokeCommand( command );

        case DATATYPE_EQNET_OBJECT:
            return _handleObjectCommand( command );

        default:
            EQWARN << "Undispatched command " << command << endl;
            return COMMAND_ERROR;
    }
}

CommandResult Session::_handleObjectCommand( Command& command )
{
    EQASSERT( command.isValid( ));
    const ObjectPacket* objPacket = command.getPacket<ObjectPacket>();
    const uint32_t      id        = objPacket->objectID;

    if( _objects.find( id ) == _objects.end( ))
    {
        EQWARN << "no objects to handle command, redispatching " << objPacket
               << endl;
        return COMMAND_REDISPATCH;
    }

    _objectsMutex.set();

    // create copy of objects vector for thread-safety
    vector<Object*>& objectsTmp = _objects[id];
    EQASSERT( !objectsTmp.empty( ));

    const size_t nObjects = objectsTmp.size();
    Object* objects[ nObjects ];
    memcpy( objects, &objectsTmp[0], nObjects * sizeof( Object* ));

    _objectsMutex.unset();


    for( size_t i=0; i<nObjects; ++i )
    {
        Object* object = objects[i];

        if( objPacket->instanceID == EQ_ID_ANY ||
            objPacket->instanceID == object->getInstanceID( ))
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

                case COMMAND_REDISPATCH:
                case COMMAND_PUSH:
                    // Not sure if we should ever allow these functions on
                    // packets which are sent to all object instances
                    // Note: if the first object returns one of these results,
                    // we assume for now that it applies to all.
                    if( i != 0 && objPacket->instanceID == EQ_ID_ANY )
                        EQUNIMPLEMENTED;

                    return result;

                case COMMAND_HANDLED:
                    if( objPacket->instanceID == object->getInstanceID( ))
                        return result;
                    break;

                default:
                    EQUNREACHABLE;
            }
        }
    }
    if( objPacket->instanceID == EQ_ID_ANY )
        return COMMAND_HANDLED;

    EQWARN << "instance not found, redispatching " << objPacket << endl;
    return COMMAND_REDISPATCH;
}

CommandResult Session::_cmdGenIDs( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionGenIDsPacket* packet =command.getPacket<SessionGenIDsPacket>();
    EQINFO << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );

    reply.id = _masterPool.genIDs( packet->range );
    send( command.getNode(), reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionGenIDsReplyPacket* packet =
        command.getPacket<SessionGenIDsReplyPacket>();
    EQINFO << "Cmd gen IDs reply: " << packet << endl;
    _requestHandler.serveRequest( packet->requestID, packet->id );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionSetIDMasterPacket* packet = 
        command.getPacket<SessionSetIDMasterPacket>();
    EQINFO << "Cmd set ID master: " << packet << endl;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->start + packet->range, 
                          packet->masterID };
    _idMasterInfos.push_back( info );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMaster( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionGetIDMasterPacket* packet =
        command.getPacket<SessionGetIDMasterPacket>();
    EQINFO << "handle get idMaster " << packet << endl;

    SessionGetIDMasterReplyPacket reply( packet );
    reply.start = 0;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    const uint32_t id     = packet->id;
    const uint32_t nInfos = _idMasterInfos.size();
    RefPtr<Node>   node   = command.getNode();
    for( uint32_t i=0; i<nInfos; ++i )
    {
        IDMasterInfo& info = _idMasterInfos[i];
        if( id >= info.start && id < info.end )
        {
            reply.start    = info.start;
            reply.end      = info.end;
            reply.masterID = info.master;
            break;
        }
    }

    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMasterReply( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionGetIDMasterReplyPacket* packet = 
        command.getPacket<SessionGetIDMasterReplyPacket>();
    EQINFO << "handle get idMaster reply " << packet << endl;

    if( packet->start != 0 )
    {
        // TODO thread-safety: _idMasterInfos is also read & written by app
        IDMasterInfo info = { packet->start, packet->end, packet->masterID };
        _idMasterInfos.push_back( info );
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
    attachObject( object, packet->objectID );
    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdDetachObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionDetachObjectPacket* packet = 
        command.getPacket<SessionDetachObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd detach object " << packet << endl;

    Object* object =  static_cast<Object*>( _requestHandler.getRequestData( 
                                                packet->requestID ));
    detachObject( object );
    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdMapObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionMapObjectPacket* packet = 
        command.getPacket<SessionMapObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd map object " << packet << endl;

    MapObjectData* data   = static_cast<MapObjectData*>( 
        _requestHandler.getRequestData( packet->requestID ));    
    Object*             object = data->object;
    EQASSERT( object );

    if( !object->isMaster( ))
    { 
        // slave instanciation - subscribe first
        SessionSubscribeObjectPacket subscribePacket;
        subscribePacket.requestID  = packet->requestID;
        subscribePacket.objectID   = data->objectID;
        subscribePacket.instanceID = _instanceIDs.genIDs(1);

        send( data->master, subscribePacket );
        return COMMAND_HANDLED;
    }

    attachObject( object, data->objectID );

    EQLOG( LOG_OBJECTS ) << "mapped object id " << object->getID() << " @" 
                         << (void*)object << " is " << typeid(*object).name()
                         << endl;

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    SessionSubscribeObjectPacket* packet =
        command.getPacket<SessionSubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd subscribe object " << packet << endl;

    SessionSubscribeObjectReplyPacket reply( packet );

    RefPtr<Node>   node = command.getNode();
    const uint32_t id   = packet->objectID;

    if( _objects.find( id ) != _objects.end( ))
    {
        vector<Object*>& objects = _objects[id];

        for( vector<Object*>::const_iterator i = objects.begin();
             i != objects.end(); ++i )
        {
            Object* object = *i;
            if( object->isMaster( ))
            {
                SessionSubscribeObjectSuccessPacket successPacket( packet );
                successPacket.cmType = object->_getChangeManagerType();

                const void* data = object->_cm->getInitialData( 
                    &successPacket.dataSize, &successPacket.version  );

                send( node, successPacket, data, successPacket.dataSize );
                object->addSlave( node, packet->instanceID );
                send( node, reply );
                return COMMAND_HANDLED;
            }
        }   
    }
    
    EQWARN << "Can't find master object for subscribe" << endl;
    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObjectSuccess( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionSubscribeObjectSuccessPacket* packet = 
        command.getPacket<SessionSubscribeObjectSuccessPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd object subscribe success " << packet << endl;

    MapObjectData* data   = static_cast<MapObjectData*>( 
        _requestHandler.getRequestData( packet->requestID ));
    Object*        object = data->object;

    EQASSERT( object );
    EQASSERT( !object->isMaster( ));

    // Don't use 'attachObject( object, data->objectID )' here - we already have
    // an instance id.
    object->_id         = data->objectID;
    object->_instanceID = packet->instanceID;
    object->_session    = this;

    object->_setupChangeManager( packet->cmType, false );
    object->_cm->applyInitialData( packet->data, packet->dataSize,
                                   packet->version );

    _objectsMutex.set();
    vector<Object*>& objects = _objects[ data->objectID ];
    _objectsMutex.unset();
    objects.push_back( object );
    
    EQLOG( LOG_OBJECTS ) << "subscribed object id " << object->getID() << " @" 
                         << (void*)object << " is " << typeid(*object).name()
                         << endl;
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSubscribeObjectReply( Command& command )
{
    CHECK_THREAD( _receiverThread );
    const SessionSubscribeObjectReplyPacket* packet = 
        command.getPacket<SessionSubscribeObjectReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd object subscribe reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnmapObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    SessionUnmapObjectPacket* packet =
        command.getPacket<SessionUnmapObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unmap object " << packet << endl;

    Object* object = static_cast<Object*>(
        _requestHandler.getRequestData( packet->requestID ));
    EQASSERT( object );

    const uint32_t id = object->getID();
    if( _objects.find( id ) == _objects.end( ))
    {
        EQASSERT( id == EQ_ID_INVALID );
        EQWARN << "trying to remove unmapped object" << endl;
        return COMMAND_HANDLED;
    }

    if( !object->isMaster( ))
    {
        // unsubscribe first
        RefPtr<Node> master = _pollIDMasterNode( id );
        if( master.isValid( ))
        {
            SessionUnsubscribeObjectPacket unsubscribePacket;
            unsubscribePacket.requestID = packet->requestID;
            unsubscribePacket.objectID  = id;

            send( master, unsubscribePacket );
            return COMMAND_HANDLED;
        }
        EQERROR << "Master node for object id " << id << " not connected"
                << endl;
    }

    detachObject( object );

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnsubscribeObject( Command& command )
{
    CHECK_THREAD( _receiverThread );
    SessionUnsubscribeObjectPacket* packet =
        command.getPacket<SessionUnsubscribeObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unsubscribe object  " << packet << endl;

    SessionUnsubscribeObjectReplyPacket reply( packet );

    RefPtr<Node>   node = command.getNode();
    const uint32_t id   = packet->objectID;

    if( _objects.find( id ) != _objects.end( ))
    {
        vector<Object*>& objects = _objects[id];

        for( vector<Object*>::const_iterator i = objects.begin();
             i != objects.end(); ++ i )
        {
            Object* object = *i;
            if( object->isMaster( ))
            {
                object->removeSlave( node );
                send( node, reply );
                return COMMAND_HANDLED;
            }
        }   
    }

    EQWARN << "Can't find master object for unsubscribe" << endl;
    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnsubscribeObjectReply( Command& command )
{
    CHECK_THREAD( _receiverThread );
    SessionUnsubscribeObjectReplyPacket* packet =
        command.getPacket<SessionUnsubscribeObjectReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unsubscribe object reply " << packet << endl;

    Object* object = static_cast<Object*>(
        _requestHandler.getRequestData( packet->requestID ));
    EQASSERT( object );

    EQASSERT( !object->isMaster( ));
    detachObject( object );

    EQLOG( LOG_OBJECTS ) << "unmapped object @" << (void*)object << " type "
                         << typeid(*object).name() << endl;

    _requestHandler.serveRequest( packet->requestID );
    return COMMAND_HANDLED;
}

std::ostream& eqNet::operator << ( std::ostream& os, Session* session )
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
