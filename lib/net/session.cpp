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
        : Base( threadSafe ),
          _id(EQ_ID_INVALID),
          _server(0),
          _isMaster(false),
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
                     CommandFunc<Session>( this,
                                             &Session::_cmdGetIDMasterReply ));
    registerCommand( CMD_SESSION_GET_OBJECT_MASTER, 
                     CommandFunc<Session>( this,
                                             &Session::_cmdGetObjectMaster ));
    registerCommand( CMD_SESSION_GET_OBJECT_MASTER_REPLY,
            CommandFunc<Session>( this, &Session::_cmdGetObjectMasterReply ));
    registerCommand( CMD_SESSION_REGISTER_OBJECT,
                     CommandFunc<Session>( this,
                                             &Session::_cmdRegisterObject ));
    registerCommand( CMD_SESSION_UNREGISTER_OBJECT, 
                     CommandFunc<Session>( this,
                                             &Session::_cmdUnregisterObject ));
    registerCommand( CMD_SESSION_GET_OBJECT,
                     CommandFunc<Session>( this, &Session::_cmdGetObject ));
    registerCommand( CMD_SESSION_INIT_OBJECT, 
                     CommandFunc<Session>( this, &Session::_cmdInitObject ));
    registerCommand( CMD_SESSION_INSTANCIATE_OBJECT, 
                     CommandFunc<Session>( this,
                                             &Session::_cmdInstanciateObject ));

    EQINFO << "New Session @" << (void*)this << endl;
}

Session::~Session()
{
    EQINFO << "Delete Session @" << (void*)this << endl;

    _localNode = 0;
    _server    = 0;
        
    if( _threadObjects.get( ))
    {
        if( !_threadObjects->empty( ))
            EQWARN << _threadObjects->size()
                   << " registed thread objects in destructor" << endl;
        _threadObjects->clear();
    }

    if( !_nodeObjects.empty( ))
        EQWARN << _nodeObjects.size()
               << " registed node objects in destructor" << endl;
    _nodeObjects.clear();

    if( !_registeredObjects.empty( ))
    {
        if( eqBase::Log::level >= eqBase::LOG_WARN ) // OPT
        {
            EQWARN << _registeredObjects.size()
                   << " registered objects in destructor" << endl;

            for( IDHash< vector<Object*> >::const_iterator i =
                     _registeredObjects.begin(); 
                 i != _registeredObjects.end(); ++i )
            {
                const vector<Object*>& objects = i->second;
                EQWARN << "  " << objects.size() << " objects with id " 
                       << i->first << endl;

                for( vector<Object*>::const_iterator j = objects.begin();
                     j != objects.end(); ++j )
                {
                    const Object* object = *j;
                    EQINFO << "    Object of type " << typeid(*object).name() 
                           << ", type " << object->getTypeID() << endl;
                }
            }
        }
    }
    _registeredObjects.clear();
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
    id = (uint32_t)(long long)_requestHandler.waitRequest( packet.requestID );

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
        
    if( !master || _isMaster )
        return master;

    // ask session master instance
    SessionGetIDMasterPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.id        = id;

    send( packet );
    _requestHandler.waitRequest( packet.requestID );
    return _pollIDMaster( id );
}

//---------------------------------------------------------------------------
// object mapping
//---------------------------------------------------------------------------
void Session::addRegisteredObject( const uint32_t id, Object* object,
                                   const Object::SharePolicy policy )
{
    EQASSERT( object->_id == EQ_ID_INVALID );
    EQASSERT( id != EQ_ID_INVALID );

    if( !_localNode->inReceiverThread( ))
    {
        SessionRegisterObjectPacket packet;
        packet.requestID = _requestHandler.registerRequest( object );
        packet.objectID  = id;
        packet.policy    = ( policy == Object::SHARE_THREAD ?
                             Object::SHARE_NEVER : policy );

        _sendLocal( packet );
        _requestHandler.waitRequest( packet.requestID );

        if( policy == Object::SHARE_THREAD )
            _registerThreadObject( object, id );

        return;
    }

    object->_id         = id;
    object->_instanceID = _instanceIDs.genIDs(1);
    object->_session    = this;
    object->ref();

    EQASSERT( object->_instanceID != EQ_ID_INVALID );

    vector<Object*>& objects = _registeredObjects[id];
    objects.push_back( object );

    switch( policy )
    {
        case Object::SHARE_NODE:
            EQASSERT( _nodeObjects.find( id ) == _nodeObjects.end( ));
            _nodeObjects[id] = object;
            break;

        case Object::SHARE_THREAD:
            _registerThreadObject( object, id );
            break;

        case Object::SHARE_NEVER:
            break;
        default:
            EQUNREACHABLE;
    }

    EQLOG( LOG_OBJECTS ) << "addRegisteredObject id " << id << " @" 
                         << (void*)object << " is " << typeid(*object).name()
                         << endl;
}

void Session::removeRegisteredObject( Object* object )
{
    EQASSERT( object->_id != EQ_ID_INVALID );

    if( !_localNode->inReceiverThread( ))
    {
        SessionUnregisterObjectPacket packet;
        packet.requestID = _requestHandler.registerRequest( object );

        _sendLocal( packet );
        _requestHandler.waitRequest( packet.requestID );

        const uint32_t   id            = object->getID();
        IDHash<Object*>* threadObjects = _threadObjects.get();
        if( threadObjects && 
            threadObjects->find( id ) != threadObjects->end() &&
            (*threadObjects)[id] == object )

            _threadObjects->erase( id );

        return;
    }

    const uint32_t id = object->getID();
    if( _registeredObjects.find( id ) == _registeredObjects.end( ))
        return;

    vector<Object*>&          objects = _registeredObjects[id];
    vector<Object*>::iterator iter    = find( objects.begin(), objects.end(),
                                              object );
    if( iter == objects.end( ))
        return;

    EQLOG( LOG_OBJECTS ) << "removeRegisteredObject id " << id << " @" 
                         << (void*)object << " ref " << object->getRefCount()
                         << " type " << typeid(*object).name() << endl;
    objects.erase( iter );

    if( objects.empty( ))
        _registeredObjects.erase( id );

    if( _nodeObjects.find( id ) != _nodeObjects.end( ) &&
        _nodeObjects[id] == object )

        _nodeObjects.erase( id );

    IDHash<Object*>* threadObjects = _threadObjects.get();
    if( threadObjects && 
        threadObjects->find( id ) != threadObjects->end() &&
        (*threadObjects)[id] == object )
        
        _threadObjects->erase( id );

    EQASSERT( object->_instanceID != EQ_ID_INVALID );
    _instanceIDs.freeIDs( object->_instanceID, 1 );
    // TODO: unsetIDMaster( object->_id );
    object->_id         = EQ_ID_INVALID;
    object->_instanceID = EQ_ID_INVALID;
    object->_session    = 0;
    object->unref();
}

void Session::registerObject( Object* object, RefPtr<Node> master, 
                              const Object::SharePolicy policy,
                              const Object::ThreadSafety ts )
{
    EQASSERT( object->_id == EQ_ID_INVALID );

    const uint32_t id = genIDs( 1 );
    EQASSERT( id != EQ_ID_INVALID );

    EQASSERT( master.isValid( ));
    setIDMaster( id, 1, master->getNodeID( ));

    const bool threadSafe = ( ts==Object::CS_SAFE || 
                         ( ts==Object::CS_AUTO && policy==Object::SHARE_NODE ));
    if( threadSafe ) 
        object->makeThreadSafe();

    EQLOG( LOG_OBJECTS ) << "registerObject type " << typeid(*object).name()
                         << " id " << id << " @" 
                         << (void*)object << " ref " << object->getRefCount()
                         << " ts " << threadSafe << " master " 
                         << master->getNodeID() << endl;

    if( _localNode == master )
        object->_master = true;
    else
    {
        object->_master = false;

        // Do initial instanciation before registration to be thread-safe
        // with other requests in the recv thread. Pre-stage object fields
        // needed for sending out instancation. Ugly.
        object->_id      = id;
        object->_session = this;
        object->instanciateOnNode( master, policy, Object::VERSION_HEAD, 
                                   threadSafe );
        // reset pre-staged data to avoid safety asserts during registration
        object->_id      = EQ_ID_INVALID;
        object->_session = 0;
    }

    addRegisteredObject( id, object, policy );
}

void Session::_registerThreadObject( Object* object, const uint32_t id )
{
    EQASSERT( !_localNode->inReceiverThread( ))

    if( !_threadObjects.get() )
        _threadObjects = new IDHash<Object*>;

    EQASSERT( _threadObjects->find( id ) == _threadObjects->end( ));
    (*_threadObjects.get())[id] = object;
}

void Session::deregisterObject( Object* object )
{
    const uint32_t id = object->getID();

    EQLOG( LOG_OBJECTS ) << "deregisterObject id " << id 
                         << " @" << (void*)object << " ref " 
                         << object->getRefCount() << endl;

    removeRegisteredObject( object );
    freeIDs( id, 1 );
}

Object* Session::getObject( const uint32_t id, const Object::SharePolicy policy,
                            const uint32_t version, 
                            const Object::ThreadSafety ts )
{
    CHECK_NOT_THREAD( _receiverThread );

    EQLOG( LOG_OBJECTS ) << "getObject id " << id << " v" << version << endl;

    const bool threadSafe = ( ts==Object::CS_SAFE || 
                         ( ts==Object::CS_AUTO && policy==Object::SHARE_NODE ));

    Object* object = pollObject( id, policy );
    if( object )
    {
        EQLOG( LOG_OBJECTS ) << "getObject polled ok, id " << id << " v"
                             << object->getVersion() << " @" << (void*)object
                             << " ref " << object->getRefCount() << endl;

        EQASSERTINFO( !threadSafe || object->isThreadSafe(), 
                      "Can't make existing object thread safe." );
        EQASSERT( object->_session );

        object->sync( version );
        return object;
    }

    GetObjectState state;
    state.policy     = ( policy == Object::SHARE_THREAD ?
                         Object::SHARE_NEVER : policy );
    state.threadSafe = threadSafe;
    state.objectID   = id;
    state.version    = version;

    SessionGetObjectPacket packet;
    packet.requestID = _requestHandler.registerRequest( &state );

    EQLOG( LOG_OBJECTS ) << "getObject send request, id " << id << " v" 
                         << version << " my node id "
                         << _localNode->getNodeID() << endl;
    _sendLocal( packet );

    const void* result = _requestHandler.waitRequest( packet.requestID );
    EQASSERT( result == 0 );
    EQASSERT( state.nodeConnectRequestID == EQ_ID_INVALID );

    if( !state.object )
    {
        EQWARN << "Could not instanciate object" << endl;
        return 0;
    }

    EQLOG( LOG_OBJECTS ) << "getObject request ok, id " << id << " v" 
                         << state.object->getVersion()
                         << " @" << (void*)state.object << " ref " 
                         << state.object->getRefCount() << endl;

    switch( policy )
    {
        case Object::SHARE_THREAD:
            _registerThreadObject( state.object, id );
            break;

        case Object::SHARE_NODE:
        case Object::SHARE_NEVER:
            break;
        default:
            EQUNREACHABLE;
    }

    EQASSERT( threadSafe == state.object->isThreadSafe( ));
    state.object->sync( version );
    return state.object;
}

Object* Session::pollObject( const uint32_t id, 
                             const Object::SharePolicy policy )
{
    switch( policy )
    {
        case Object::SHARE_NODE:
            if( _nodeObjects.find( id ) == _nodeObjects.end( )) // obj not found
                return 0;
            return _nodeObjects[id];

        case Object::SHARE_THREAD:
            if( !_threadObjects.get( ) ||       // no thread objects registered
                _threadObjects->find( id ) == _threadObjects->end( )) // !found
                return 0;
            
            return (*_threadObjects.get())[id];
            
        case Object::SHARE_NEVER:
            return 0;

        default:
            EQUNREACHABLE;
    }
    return 0;
}

Object* Session::instanciateObject( const uint32_t type, const void* data, 
                                    const uint64_t dataSize )
{
    switch( type )
    {
        case Object::TYPE_BARRIER:
            return new Barrier( data );

        default:
            return 0;
    }
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

    if( _registeredObjects.find( id ) == _registeredObjects.end( ))
    {
        EQWARN << "no objects to handle command, redispatching " << objPacket
               << endl;
        return COMMAND_REDISPATCH;
    }

    vector<Object*>& objects = _registeredObjects[id];
    EQASSERT( !objects.empty( ));

    for( vector<Object*>::const_iterator i = objects.begin(); 
        i != objects.end(); ++i )
    {
        Object* object = *i;
        if( objPacket->instanceID == EQ_ID_ANY ||
            objPacket->instanceID == object->getInstanceID( ))
        {
            if( !command.isValid( ))
            {
                // NOTE: command got invalidated (last object pushed command to
                // another thread) . Object should push copy of command, or we
                // should make it clear what invalidating a command means here
                // (same as discard?)
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
                case COMMAND_PUSH_FRONT:
                    // Not sure if we should ever allow these functions on
                    // packets which are sent to all object instances
                    // Note: if the first object returns one of these results,
                    // we assume for now that it applies to all.
                    if( i != objects.begin() &&
                        objPacket->instanceID == EQ_ID_ANY )

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

CommandResult Session::_instObject( GetObjectState* state )
{
    CHECK_THREAD( _receiverThread );
    const uint32_t objectID = state->objectID;

    switch( state->instState )
    {
        case INST_UNKNOWN:
        {
            RefPtr<Node> master = _pollIDMasterNode( objectID );
            if( master.isValid( ))
            {
                EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                     << " send init req to master" << endl;
                state->instState = INST_GOTMASTER;
                _sendInitObject( state, master );
                return COMMAND_REDISPATCH;
            }

            if( _isMaster )
            {
                EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                     << " failure, id unknown to master "
                                     << endl;
                return COMMAND_ERROR;
            }

            EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                 << " send get master req to session" << endl;
            SessionGetObjectMasterPacket packet;
            packet.objectID = objectID;
            send( packet );
            state->instState = INST_GETMASTERID;
            return COMMAND_REDISPATCH;
        }
        
        case INST_GOTMASTER:
        {
            RefPtr<Node> master = _pollIDMasterNode( objectID );
            if( !master )
            {
                EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                     << " failure in get master req" << endl;
                state->instState = INST_UNKNOWN;
                return COMMAND_ERROR;
            }

            EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                 << " got master, send init req" << endl;
            _sendInitObject( state, master );
            return COMMAND_REDISPATCH;
        }
            
        case INST_ERROR:
            EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                 << " failure" << endl;
            return COMMAND_ERROR;

        case INST_GETMASTERID:
        case INST_INIT:
            return COMMAND_REDISPATCH;

        default:
            return COMMAND_ERROR;
    }
}

void Session::_sendInitObject( GetObjectState* state, RefPtr<Node> master )
{
    SessionInitObjectPacket packet;
    packet.objectID   = state->objectID;
    packet.version    = state->version;
    packet.policy     = state->policy;
    packet.threadSafe = state->threadSafe;

    EQLOG( LOG_OBJECTS ) << "send init obj " << &packet << endl;
    send( master.get(), packet );
    state->instState = INST_INIT;
}
            
CommandResult Session::_cmdGenIDs( Command& command )
{
    const SessionGenIDsPacket* packet =command.getPacket<SessionGenIDsPacket>();
    EQINFO << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );

    reply.id = _masterPool.genIDs( packet->range );
    send( command.getNode(), reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Command& command )
{
    const SessionGenIDsReplyPacket* packet =
        command.getPacket<SessionGenIDsReplyPacket>();
    EQINFO << "Cmd gen IDs reply: " << packet << endl;
    _requestHandler.serveRequest( packet->requestID, 
                                  (void*)(long long)(packet->id) );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Command& command )
{
    const SessionSetIDMasterPacket* packet = 
        command.getPacket<SessionSetIDMasterPacket>();
    EQINFO << "Cmd set ID master: " << packet << endl;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->start + packet->range, 
                          packet->masterID };
    info.slaves.push_back( command.getNode() );
    _idMasterInfos.push_back( info );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMaster( Command& command )
{
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

            info.slaves.push_back( node );
            break;
        }
    }

    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMasterReply( Command& command )
{
    const SessionGetIDMasterReplyPacket* packet = 
        command.getPacket<SessionGetIDMasterReplyPacket>();
    EQINFO << "handle get idMaster reply " << packet << endl;

    if( packet->start == 0 ) // not found
    {
        _requestHandler.serveRequest( packet->requestID, 0 );
        return COMMAND_HANDLED;
    }

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->end, packet->masterID };
    _idMasterInfos.push_back( info );

    _requestHandler.serveRequest( packet->requestID, 0 );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObjectMaster( Command& command )
{
    const SessionGetObjectMasterPacket* packet = 
        command.getPacket<SessionGetObjectMasterPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd get object master " << packet << endl;

    SessionGetObjectMasterReplyPacket reply( packet );
    reply.start = 0;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    const uint32_t id     = packet->objectID;
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

            info.slaves.push_back( node );
            break;
        }
    }

    EQLOG( LOG_OBJECTS ) << "Send get object master reply " << &reply << endl;
    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObjectMasterReply( Command& command)
{
    CHECK_THREAD( _receiverThread );
    const SessionGetObjectMasterReplyPacket* packet = 
        command.getPacket<SessionGetObjectMasterReplyPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd get object master reply " << packet << endl;
 
    const uint32_t  id    = packet->objectID;
    GetObjectState* state = _objectInstStates[id];
    EQASSERT( state );

    if( packet->start == 0 ) // not found
    {
        EQLOG( LOG_OBJECTS ) << "Master for id " << id << " not found" << endl;
        state->instState = INST_ERROR;
        return COMMAND_HANDLED;
    }

    RefPtr<Node> master;
    if( state->nodeConnectRequestID == EQ_ID_INVALID )
    {
         master = _localNode->getNode( packet->masterID );

         if( !master.isValid( ))
         {
             EQLOG( LOG_OBJECTS ) << "Connect node " << packet->masterID 
                                  << endl;
             state->nodeConnectRequestID =
                 _localNode->startNodeConnect( packet->masterID, _server );
    
             return COMMAND_REDISPATCH;
         }
    }
    else
    {
        switch( _localNode->pollNodeConnect( state->nodeConnectRequestID ))
        {
            case Node::CONNECT_PENDING:
                return COMMAND_REDISPATCH;

            case Node::CONNECT_SUCCESS:
                EQLOG( LOG_OBJECTS ) << "Node " << packet->masterID 
                                     << " connected" << endl;
                master = _localNode->getNode( packet->masterID );
                EQASSERT( master.isValid( ));
                state->nodeConnectRequestID = EQ_ID_INVALID;
                break;

            case Node::CONNECT_FAILURE:
                EQWARN 
                    << "Can't connect master node during object instantiation"
                    << endl;
                state->instState            = INST_ERROR;
                state->nodeConnectRequestID = EQ_ID_INVALID;
                return COMMAND_HANDLED;

            default:
                return COMMAND_ERROR;
        }
    }
    
    EQASSERT( master.isValid( ));

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->end, packet->masterID };
    _idMasterInfos.push_back( info );
    
    state->instState = INST_GOTMASTER;
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdRegisterObject( Command& command )
{
    CHECK_THREAD( _receiverThread );

    const SessionRegisterObjectPacket* packet = 
        command.getPacket<SessionRegisterObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd register object " << packet << endl;
    EQASSERT( packet->objectID != EQ_ID_INVALID );

    Object* object = (Object*)_requestHandler.getRequestData(packet->requestID);
    EQASSERT( object );

    addRegisteredObject( packet->objectID, object, packet->policy );
    _requestHandler.serveRequest( packet->requestID, 0 );
    
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnregisterObject( Command& command )
{
    CHECK_THREAD( _receiverThread );

    const SessionUnregisterObjectPacket* packet =
        command.getPacket<SessionUnregisterObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd unregister object " << packet << endl;

    Object* object = (Object*)_requestHandler.getRequestData(packet->requestID);
    EQASSERT( object );

    removeRegisteredObject( object );
    _requestHandler.serveRequest( packet->requestID, 0 );
    
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObject( Command& command )
{
    CHECK_THREAD( _receiverThread );

    const SessionGetObjectPacket* packet = 
        command.getPacket<SessionGetObjectPacket>();

    GetObjectState*         state  = (GetObjectState*)
        _requestHandler.getRequestData( packet->requestID );
    EQASSERT( state );

    const uint32_t id     = state->objectID;
    EQLOG( LOG_OBJECTS ) << "Cmd get object " << packet << " id " << id
                         << " v" << state->version << endl;

    if( state->object ) // successfully instanciated
    {
        EQLOG( LOG_OBJECTS ) << "Object instanciated, id " << id << " @" 
                             << (void*)state->object << " ref " 
                             << state->object->getRefCount() << endl;

        EQASSERTINFO( !state->threadSafe || state->object->isThreadSafe(), 
                      "Can't make existing object thread safe." );
        _objectInstStates.erase( id );
        _requestHandler.serveRequest( packet->requestID, 0 );
        return COMMAND_HANDLED;
    }

    if( state->policy == Object::SHARE_NODE )
    {
        Object* object = pollObject( id, Object::SHARE_NODE );

        if( object ) // per-node object instanciated already
        {
            EQLOG( LOG_OBJECTS ) << "object known, id " << id << " @" 
                                 << (void*)object << " ref " 
                                 << object->getRefCount() << endl;
            state->object = object;
            _requestHandler.serveRequest( packet->requestID, 0 );
            return COMMAND_HANDLED;
        }
    }

    if( !state->pending ) // first iteration of instantiation
    {
        if( _objectInstStates[id] != 0 ) // instantion with same ID pending
            return COMMAND_REDISPATCH;

        EQLOG( LOG_OBJECTS ) << "start object instanciation, id " << id << endl;
        // mark pending instantion 
        state->pending        = true;
        _objectInstStates[id] = state;
    }

    if( _instObject( state ) == COMMAND_ERROR )
    {
        EQLOG( LOG_OBJECTS ) << "error during object instanciation, id " 
                             << id << endl;
        _requestHandler.serveRequest( packet->requestID, 0 );
        return COMMAND_HANDLED;
    }
    return COMMAND_REDISPATCH;
}

CommandResult Session::_cmdInitObject( Command& command )
{
    const SessionInitObjectPacket* packet =
        command.getPacket<SessionInitObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd init object " << packet << endl;

    const uint32_t   id      = packet->objectID;
    EQASSERT( _registeredObjects.find( id ) != _registeredObjects.end( ));

    vector<Object*>& objects = _registeredObjects[id];
    EQASSERT( !objects.empty( ));

    for( vector<Object*>::const_iterator i = objects.begin();
         i != objects.end(); ++i )
    {
        Object* object = *i;
        EQASSERT( object->getID() == id );
        if( !object->isMaster( ))
            continue;

        RefPtr<Node>   node   = command.getNode();
        object->instanciateOnNode( node, packet->policy, packet->version, 
                                   packet->threadSafe );
        return COMMAND_HANDLED;
    }

    // (hopefully) not yet instanciated -> reschedule
    return COMMAND_REDISPATCH;
}

CommandResult Session::_cmdInstanciateObject( Command& command )
{
    CHECK_THREAD( _receiverThread );

    const SessionInstanciateObjectPacket* packet = 
        command.getPacket<SessionInstanciateObjectPacket>();
    EQLOG( LOG_OBJECTS ) << "Cmd instanciate object " << packet << " from "
                         << command.getNode() << endl;

    const uint32_t id = packet->objectID;
    if( packet->error )
    {
        EQWARN << "Object master encountered error during instanciation request"
               << endl;
        GetObjectState* state = _objectInstStates[id];
        if( state )
            state->instState = INST_ERROR;
        return COMMAND_HANDLED;
    }

    Object* object = instanciateObject( packet->objectType, packet->objectData, 
                                        packet->objectDataSize );
    if( !object )
    {
        EQWARN << "Session::instanciateObject of type " << packet->objectType
               << " failed from " << typeid(*this).name() << endl;
        return COMMAND_ERROR;
    }

    if( packet->isMaster )
    {
        RefPtr<Node> node = command.getNode();
        object->addSlave( node ); // Assumes that sender is a subscribed slave
        object->_setInitialVersion( packet->objectData, packet->objectDataSize);
        EQASSERT( object->getVersion() == packet->version );
    }

    object->_master  = packet->isMaster;
    object->_version = packet->version;

    GetObjectState* state = _objectInstStates[id];
    if( state )
    {
        EQLOG( LOG_OBJECTS ) << "Object instanciation caused by request" <<endl;
        if( state->threadSafe ) 
            object->makeThreadSafe();
        state->object = object;
    }

    addRegisteredObject( id, object, 
                         state ? state->policy : packet->policy );

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
