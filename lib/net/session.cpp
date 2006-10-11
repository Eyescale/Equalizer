/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"

#include "barrier.h"
#include "connection.h"
#include "connectionDescription.h"
#include "packets.h"
#include "session.h"

#include <eq/base/log.h>

#include <alloca.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

#define MIN_ID_RANGE 1024

Session::Session( const uint32_t nCommands, const bool threadSafe )
        : Base( nCommands ),
          _requestHandler( threadSafe ),
          _id(EQ_ID_INVALID),
          _server(NULL),
          _isMaster(false),
          _masterPool( IDPool::MAX_CAPACITY ),
          _localPool( 0 ),
          _instanceIDs( IDPool::MAX_CAPACITY ) 
{
    EQASSERT( nCommands >= CMD_SESSION_CUSTOM );
    registerCommand( CMD_SESSION_GEN_IDS, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGenIDs ));
    registerCommand( CMD_SESSION_GEN_IDS_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGenIDsReply ));
    registerCommand( CMD_SESSION_SET_ID_MASTER, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdSetIDMaster ));
    registerCommand( CMD_SESSION_GET_ID_MASTER, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetIDMaster ));
    registerCommand( CMD_SESSION_GET_ID_MASTER_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetIDMasterReply ));
    registerCommand( CMD_SESSION_GET_OBJECT_MASTER, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetObjectMaster ));
    registerCommand( CMD_SESSION_GET_OBJECT_MASTER_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetObjectMasterReply ));
    registerCommand( CMD_SESSION_REGISTER_OBJECT, this,
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdRegisterObject ));
    registerCommand( CMD_SESSION_UNREGISTER_OBJECT, this,
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdUnregisterObject ));
    registerCommand( CMD_SESSION_GET_OBJECT, this,
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetObject ));
    registerCommand( CMD_SESSION_INIT_OBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdInitObject ));
    registerCommand( CMD_SESSION_INSTANCIATE_OBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdInstanciateObject ));

    EQINFO << "New Session @" << (void*)this << endl;
}

Session::~Session()
{
    EQINFO << "Delete Session @" << (void*)this << endl;
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
    for( vector<IDMasterInfo>::const_iterator iter = _idMasterInfos.begin();
         iter != _idMasterInfos.end(); ++iter )
    {
        const IDMasterInfo& info = *iter;
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
        {
            object->_policy  = policy;
            _registerThreadObject( object, id );
        }

        return;
    }

    object->_id         = id;
    object->_instanceID = _instanceIDs.genIDs(1);
    object->_policy     = policy;
    object->_session    = this;
    object->ref();

    EQASSERT( object->_instanceID != EQ_ID_INVALID );

    vector<Object*>& objects = _registeredObjects[id];
    objects.push_back( object );

    switch( policy )
    {
        case Object::SHARE_NODE:
            EQASSERT( !_nodeObjects[id] );
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
                         << (void*)object << endl;
}

void Session::removeRegisteredObject( Object* object, 
                                      Object::SharePolicy policy )
{
    EQASSERT( object->_id != EQ_ID_INVALID );
    if( policy == Object::SHARE_UNDEFINED )
        policy = object->_policy;

    if( !_localNode->inReceiverThread( ))
    {
        SessionUnregisterObjectPacket packet;
        packet.requestID = _requestHandler.registerRequest( object );
        packet.policy    = ( policy == Object::SHARE_THREAD ?
                             Object::SHARE_NEVER : policy );

        _sendLocal( packet );
        _requestHandler.waitRequest( packet.requestID );

        if( policy == Object::SHARE_THREAD )
        {
            const uint32_t id = object->getID();
            EQASSERT( _threadObjects.get( ))
            EQASSERT( (*_threadObjects.get( ))[id] );
            _threadObjects->erase( id );
        }
        return;
    }

    const uint32_t            id      = object->getID();
    vector<Object*>&          objects = _registeredObjects[id];
    vector<Object*>::iterator iter    = find( objects.begin(), objects.end(),
                                              object );
    if( iter == objects.end( ))
        return;

    EQLOG( LOG_OBJECTS ) << "removeRegisteredObject id " << id << " @" 
                         << (void*)object << " ref " << object->getRefCount()
                         << endl;
    objects.erase( iter );

    switch( policy )
    {
        case Object::SHARE_NODE:
            EQASSERT( _nodeObjects[id] );
            _nodeObjects.erase( id );
            break;

        case Object::SHARE_THREAD:
            EQASSERT( _threadObjects.get( ))
            EQASSERT( (*_threadObjects.get( ))[id] );
            _threadObjects->erase( id );
            break;

        case Object::SHARE_NEVER:
            break;
        default:
            EQUNREACHABLE;
    }

    EQASSERT( object->_instanceID != EQ_ID_INVALID );
    _instanceIDs.freeIDs( object->_instanceID, 1 );
    // TODO: unsetIDMaster( object->_id );
    object->_id         = EQ_ID_INVALID;
    object->_instanceID = EQ_ID_INVALID;
    object->_policy     = Object::SHARE_UNDEFINED;
    object->_session    = NULL;
    object->unref();
}

void Session::registerObject( Object* object, RefPtr<Node> master, 
                              const Object::SharePolicy policy,
                              const Object::ThreadSafety ts )
{
    EQASSERT( object->_id == EQ_ID_INVALID );

    const uint32_t id = genIDs( 1 );
    EQASSERT( id != EQ_ID_INVALID );

    if( object->getTypeID() != Object::TYPE_UNMANAGED )
    {
        EQASSERT( master.isValid( ));
        setIDMaster( id, 1, master->getNodeID( ));
    }

    const bool threadSafe = ( ts==Object::CS_SAFE || 
                         ( ts==Object::CS_AUTO && policy==Object::SHARE_NODE ));
    if( threadSafe ) 
        object->makeThreadSafe();

    EQLOG( LOG_OBJECTS ) << "registerObject id " << id 
                         << " @" << (void*)object << " ref " 
                         << object->getRefCount() << endl;

    addRegisteredObject( id, object, policy );

    if( _localNode == master )
    {
        object->_master = true;
        return;
    }
    object->_master = false;

    if( object->getTypeID() == Object::TYPE_UNMANAGED )
        return;

    object->instanciateOnNode( master, policy );
}

void Session::_registerThreadObject( Object* object, const uint32_t id )
{
    EQASSERT( !_localNode->inReceiverThread( ))

    if( _threadObjects.get() == NULL )
        _threadObjects = new IDHash<Object*>;
    else
        EQASSERT( !(*_threadObjects.get())[id] );

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

    const bool threadSafe = ( ts==Object::CS_SAFE || 
                         ( ts==Object::CS_AUTO && policy==Object::SHARE_NODE ));

    Object* object = pollObject( id, policy );
    if( object )
    {
        EQLOG( LOG_OBJECTS ) << "getObject polled ok, id " << id 
                             << " @" << (void*)object << " ref " 
                             << object->getRefCount() << endl;

        EQASSERTINFO( !threadSafe || object->isThreadSafe(), 
                      "Can't make existing object thread safe." );

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

    EQLOG( LOG_OBJECTS ) << "getObject send request for id " << id 
                         << " my node id " << _localNode->getNodeID() << endl;
    _sendLocal( packet );

    const void* result = _requestHandler.waitRequest( packet.requestID );
    EQASSERT( result == NULL );
    EQASSERT( state.nodeConnectRequestID == EQ_ID_INVALID );
    EQLOG( LOG_OBJECTS ) << "getObject request ok, id " << id 
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

    if( state.object )
    {
        EQASSERT( threadSafe == state.object->isThreadSafe( ));
        state.object->sync( version );
    }
    return state.object;
}

Object* Session::pollObject( const uint32_t id, 
                             const Object::SharePolicy policy)
{
    switch( policy )
    {
        case Object::SHARE_NODE:
            return _nodeObjects[id];

        case Object::SHARE_THREAD:
            if( _threadObjects.get() == NULL )
                return NULL;
            
            return (*_threadObjects.get())[id];
            
        case Object::SHARE_NEVER:
            return NULL;

        default:
            EQUNREACHABLE;
    }
    return NULL;
}

Object* Session::instanciateObject( const uint32_t type, const void* data, 
                                    const uint64_t dataSize )
{
    switch( type )
    {
        case Object::TYPE_BARRIER:
            return new Barrier( data );

        default:
            return NULL;
    }
}

//===========================================================================
// Packet handling
//===========================================================================

CommandResult Session::dispatchPacket( Node* node, const Packet* packet )
{
    EQVERB << "dispatch " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            return handleCommand( node, packet );

        case DATATYPE_EQNET_OBJECT:
            return _handleObjectCommand( node, packet );

        default:
            EQWARN << "Undispatched packet " << packet << endl;
            return COMMAND_ERROR;
    }
}

CommandResult Session::_handleObjectCommand( Node* node, const Packet* packet )
{
    const ObjectPacket* objPacket = (ObjectPacket*)packet;
    const uint32_t      id        = objPacket->objectID;
    vector<Object*>&    objects   = _registeredObjects[id];

    if( _localNode->inReceiverThread( ))
    {
        GetObjectState*     state     = _objectInstStates[id];

        if( objects.empty( ))
        {
            if( !state )
            {
                state                 = new GetObjectState;
                state->policy         = Object::SHARE_NODE;
                state->threadSafe     = true;
                state->objectID       = id;
                state->version        = Object::VERSION_HEAD;
                _objectInstStates[id] = state;
            }
            return _instObject( state );
        }
    
        if( state )
        {
            EQASSERT( state->nodeConnectRequestID == EQ_ID_INVALID );
            delete state;
            _objectInstStates.erase( id );
        }
    }
    else if( objects.empty( ))
    {
        Object* object = getObject( id, Object::SHARE_NODE );
        if( !object )
            return COMMAND_ERROR;
        EQASSERT( !objects.empty( ));
    }

    for( vector<Object*>::iterator iter = objects.begin(); 
         iter != objects.end(); ++iter )
    {
        Object* object = *iter;
        if( objPacket->instanceID == EQ_ID_ANY ||
            objPacket->instanceID == object->getInstanceID( ))
        {
            const CommandResult result = object->handleCommand( node, 
                                                                objPacket );
            switch( result )
            {
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
                    if( iter != objects.begin() &&
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
    return ( objPacket->instanceID == EQ_ID_ANY ) ? 
        COMMAND_HANDLED : COMMAND_ERROR;
}

CommandResult Session::_instObject( GetObjectState* state )
{
    CHECK_THREAD( _receiverThread );
    const uint32_t objectID = state->objectID;

    switch( state->instState )
    {
        case Object::INST_UNKNOWN:
        {
            RefPtr<Node> master = _pollIDMasterNode( objectID );
            if( master.isValid( ))
            {
                EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                     << " send init req to master" << endl;
                state->instState = Object::INST_GOTMASTER;
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
            state->instState = Object::INST_GETMASTERID;
            return COMMAND_REDISPATCH;
        }
        
        case Object::INST_GOTMASTER:
        {
            RefPtr<Node> master = _pollIDMasterNode( objectID );
            if( !master )
            {
                EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                     << " failure in get master req" << endl;
                state->instState = Object::INST_UNKNOWN;
                return COMMAND_ERROR;
            }

            EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                 << " got master, send init req" << endl;
            _sendInitObject( state, master );
            return COMMAND_REDISPATCH;
        }
            
        case Object::INST_ERROR:
            EQLOG( LOG_OBJECTS ) << "instObject id " << objectID
                                 << " failure" << endl;
            return COMMAND_ERROR;

        case Object::INST_GETMASTERID:
        case Object::INST_INIT:
            return COMMAND_REDISPATCH;

        default:
            return COMMAND_ERROR;
    }
}

void Session::_sendInitObject( GetObjectState* state, RefPtr<Node> master )
{
    SessionInitObjectPacket packet;
    packet.objectID  = state->objectID;
    packet.version   = state->version;
    packet.policy    = state->policy;

    send( master.get(), packet );
    state->instState = Object::INST_INIT;
}
            
CommandResult Session::_cmdGenIDs( Node* node, const Packet* pkg )
{
    SessionGenIDsPacket*     packet = (SessionGenIDsPacket*)pkg;
    EQINFO << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );

    reply.id = _masterPool.genIDs( packet->range );
    send( node, reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Node* node, const Packet* pkg )
{
    SessionGenIDsReplyPacket* packet = (SessionGenIDsReplyPacket*)pkg;
    EQINFO << "Cmd gen IDs reply: " << packet << endl;
    _requestHandler.serveRequest( packet->requestID, 
                                  (void*)(long long)(packet->id) );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Node* node, const Packet* pkg )
{
    SessionSetIDMasterPacket* packet = (SessionSetIDMasterPacket*)pkg;
    EQINFO << "Cmd set ID master: " << packet << endl;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->start + packet->range, 
                          packet->masterID };
    info.slaves.push_back( node );
    _idMasterInfos.push_back( info );

    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMaster( Node* node, const Packet* pkg )
{
    SessionGetIDMasterPacket*     packet = (SessionGetIDMasterPacket*)pkg;
    SessionGetIDMasterReplyPacket reply( packet );
    EQINFO << "handle get idMaster " << packet << endl;

    reply.start = 0;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    const uint32_t id     = packet->id;
    const uint32_t nInfos = _idMasterInfos.size();
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

CommandResult Session::_cmdGetIDMasterReply( Node* node, const Packet* pkg )
{
    SessionGetIDMasterReplyPacket* packet = (SessionGetIDMasterReplyPacket*)pkg;
    EQINFO << "handle get idMaster reply " << packet << endl;

    if( packet->start == 0 ) // not found
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->end, packet->masterID };
    _idMasterInfos.push_back( info );

    _requestHandler.serveRequest( packet->requestID, NULL );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObjectMaster( Node* node, const Packet* pkg )
{
    SessionGetObjectMasterPacket* packet = (SessionGetObjectMasterPacket*)pkg;
    SessionGetObjectMasterReplyPacket reply( packet );
    EQLOG( LOG_OBJECTS ) << "Cmd get object master " << packet << endl;

    reply.start = 0;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    const uint32_t id     = packet->objectID;
    const uint32_t nInfos = _idMasterInfos.size();
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

CommandResult Session::_cmdGetObjectMasterReply( Node* node, const Packet* pkg)
{
    CHECK_THREAD( _receiverThread );
    SessionGetObjectMasterReplyPacket* packet = 
        (SessionGetObjectMasterReplyPacket*)pkg;
    EQLOG( LOG_OBJECTS ) << "Cmd get object master reply " << packet << endl;
 
    const uint32_t  id    = packet->objectID;
    GetObjectState* state = _objectInstStates[id];
    EQASSERT( state );

    if( packet->start == 0 ) // not found
    {
        EQLOG( LOG_OBJECTS ) << "Master for id " << id << " not found" << endl;
        state->instState = Object::INST_ERROR;
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
                 _localNode->startConnectNode( packet->masterID, _server );
    
             return COMMAND_REDISPATCH;
         }
    }
    else
    {
        switch( _localNode->pollConnectNode( state->nodeConnectRequestID ))
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
                state->instState            = Object::INST_ERROR;
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
    
    state->instState = Object::INST_GOTMASTER;
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdRegisterObject( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _receiverThread );

    SessionRegisterObjectPacket* packet = (SessionRegisterObjectPacket*)pkg;
    EQLOG( LOG_OBJECTS ) << "Cmd register object " << packet << endl;
    EQASSERT( packet->objectID != EQ_ID_INVALID );

    Object* object = (Object*)_requestHandler.getRequestData(packet->requestID);
    EQASSERT( object );

    addRegisteredObject( packet->objectID, object, packet->policy );
    _requestHandler.serveRequest( packet->requestID, NULL );
    
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdUnregisterObject( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _receiverThread );

    SessionUnregisterObjectPacket* packet = (SessionUnregisterObjectPacket*)pkg;
    EQLOG( LOG_OBJECTS ) << "Cmd unregister object " << packet << endl;

    Object* object = (Object*)_requestHandler.getRequestData(packet->requestID);
    EQASSERT( object );

    removeRegisteredObject( object, packet->policy );
    _requestHandler.serveRequest( packet->requestID, NULL );
    
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObject( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _receiverThread );

    SessionGetObjectPacket* packet = (SessionGetObjectPacket*)pkg;

    GetObjectState*         state  = (GetObjectState*)
        _requestHandler.getRequestData( packet->requestID );
    EQASSERT( state );

    const uint32_t id     = state->objectID;
    EQLOG( LOG_OBJECTS ) << "Cmd get object " << packet << " id " << id << endl;

    if( state->object ) // successfully instanciated
    {
        EQLOG( LOG_OBJECTS ) << "Object instanciated, id " << id << " @" 
                             << (void*)state->object << " ref " 
                             << state->object->getRefCount() << endl;

        EQASSERTINFO( !state->threadSafe || state->object->isThreadSafe(), 
                      "Can't make existing object thread safe." );
        _objectInstStates.erase( id );
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }

    if( state->policy == Object::SHARE_NODE )
    {
        Object* object = _nodeObjects[id];

        if( object ) // per-node object instanciated already
        {
            EQLOG( LOG_OBJECTS ) << "object known, id " << id << " @" 
                                 << (void*)state->object << " ref " 
                                 << state->object->getRefCount() << endl;
            state->object = object;
            _requestHandler.serveRequest( packet->requestID, NULL );
            return COMMAND_HANDLED;
        }
    }

    if( !state->pending ) // first iteration of instantiation
    {
        if( _objectInstStates[id] != NULL ) // instantion with same ID pending
            return COMMAND_REDISPATCH;

        EQLOG( LOG_OBJECTS ) << "start object instanciation, id " << id << endl;
        // mark pending instantion 
        state->pending        = true;
        _objectInstStates[id] = state;
    }

    if( _instObject( state ) == COMMAND_ERROR )
    {
        EQLOG( LOG_OBJECTS ) << "error during instanciation, id " << id << endl;
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }
    return COMMAND_REDISPATCH;
}

CommandResult Session::_cmdInitObject( Node* node, const Packet* pkg )
{
    SessionInitObjectPacket* packet = (SessionInitObjectPacket*)pkg;
    EQLOG( LOG_OBJECTS ) << "Cmd init object " << packet << endl;

    const uint32_t   id      = packet->objectID;
    vector<Object*>& objects = _registeredObjects[id];

    for( vector<Object*>::iterator iter = objects.begin();
         iter != objects.end(); ++iter )
    {
        Object* object = *iter;
        if( object->getID() != id || !object->_master )
            continue;

        object->instanciateOnNode( node, packet->policy, packet->version );
        return COMMAND_HANDLED;
    }

    // (hopefully) not yet instanciated -> reschedule
    return COMMAND_REDISPATCH;
}

CommandResult Session::_cmdInstanciateObject( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _receiverThread );

    SessionInstanciateObjectPacket* packet = 
        (SessionInstanciateObjectPacket*)pkg;
    EQLOG( LOG_OBJECTS ) << "Cmd instanciate object " << packet << " from "
                         << node << endl;

    const uint32_t id     = packet->objectID;
    if( packet->error )
    {
        EQWARN << "Object master encountered error during instanciation request"
               << endl;
        GetObjectState* state = _objectInstStates[id];
        if( state )
            state->instState = Object::INST_ERROR;
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

    object->_master  = packet->isMaster;
    object->_version = packet->version;
    if( packet->isMaster )
    {
        object->addSlave( node ); // Assumes that sender is a subscribed slave
        object->_setInitialVersion( packet->objectData, packet->objectDataSize);
    }

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
    object->init( packet->objectData, packet->objectDataSize );

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
