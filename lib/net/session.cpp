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
          _requestHandler( Thread::PTHREAD, threadSafe ),
          _id(EQ_INVALID_ID),
          _server(NULL),
          _isMaster(false),
          _masterPool( IDPool::getMaxCapacity( )),
          _localPool( 0 )
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
    registerCommand( CMD_SESSION_GET_OBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetObject ));
    registerCommand( CMD_SESSION_INIT_OBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdInitObject ));
    registerCommand( CMD_SESSION_INSTANCIATE_OBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdInstanciateObject ));

    EQINFO << "New " << this << endl;

#ifdef CHECK_THREADSAFETY
    _recvThreadID = 0;
#endif
}

//---------------------------------------------------------------------------
// identifier generation
//---------------------------------------------------------------------------
uint32_t Session::genIDs( const uint32_t range )
{
    if( _isMaster && _server->inReceiverThread( ))
        return _masterPool.genIDs( range );

    uint32_t id = _localPool.genIDs( range );
    if( id )
        return id;

    SessionGenIDsPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    packet.range     = MAX(range, MIN_ID_RANGE);

    _server->send( packet );
    id = (uint32_t)(long long)_requestHandler.waitRequest( packet.requestID );

    if( !id || range >= MIN_ID_RANGE )
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
                           Node* master )
{
    EQASSERT( master->checkConnection( ));

    IDMasterInfo info = { start, start+range, master };
    _idMasterInfos.push_back( info );

    if( _isMaster )
        return;

    SessionSetIDMasterPacket packet( _id );
    packet.start    = start;
    packet.range    = range;
    packet.masterID = master->getNodeID();

    _server->send( packet );
}

RefPtr<Node> Session::_pollIDMaster( const uint32_t id )
{
    const uint32_t nInfos = _idMasterInfos.size();
    for( uint32_t i=0; i<nInfos; ++i )
    {
        const IDMasterInfo& info = _idMasterInfos[i];
        if( id >= info.start && id < info.end )
            return info.master;
    }
    return NULL;
}

RefPtr<Node> Session::getIDMaster( const uint32_t id )
{
    RefPtr<Node> master = _pollIDMaster( id );
        
    if( master.isValid() || _isMaster )
        return master;

    // ask session master instance
    SessionGetIDMasterPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    packet.id        = id;

    send( packet );
    return (Node*)_requestHandler.waitRequest( packet.requestID );
}

//---------------------------------------------------------------------------
// object mapping
//---------------------------------------------------------------------------
void Session::_addRegisteredObject( const uint32_t id, Object* object )
{
    vector<Object*>& objects = _registeredObjects[id];
    EQASSERT( object->_id == EQ_INVALID_ID );

    object->_id      = id;
    object->_session = this;
    objects.push_back( object );

    EQVERB << "registered object " << (void*)object << " id " << id
           << " session id " << _id << endl;
}

// XXX use RefPtr for object
void Session::registerObject( Object* object, Node* master )
{
    EQASSERT( object->_id == EQ_INVALID_ID );
    const uint32_t id = genIDs( 1 );

    if( object->getTypeID() != Object::TYPE_UNMANAGED )
    {
        EQASSERT( master );
        setIDMaster( id, 1, master );
    }
    _addRegisteredObject( id, object );

    if( _localNode == master )
    {
        object->_master = true;
        return;
    }
    
    if( object->getTypeID() == Object::TYPE_UNMANAGED )
        return;

    object->_master = false;
    object->instanciateOnNode( master );
}

void Session::deregisterObject( Object* object )
{
    const uint32_t            id      = object->getID();
    vector<Object*>&          objects = _registeredObjects[id];
    vector<Object*>::iterator iter    = find( objects.begin(), objects.end(),
                                              object );
    
    if( iter == objects.end( ))
        return;

    objects.erase( iter );

    // Remove from node/thread shared hash
    _nodeObjects.erase( id );
    if( _threadObjects.get( ))
        _threadObjects->erase( id );

    // TODO: unsetIDMaster( object->_id );
    object->_id      = EQ_INVALID_ID;
    object->_session = NULL;
    freeIDs( id, 1 );
}

Object* Session::getObject( const uint32_t id, const Object::SharePolicy policy)
{
    CHECK_NOT_THREAD( _recvThreadID );

    Object* object = pollObject( id, policy );
    if( object )
        return object;

    GetObjectState state;
    state.policy   = policy;
    state.objectID = id;

    SessionGetObjectPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest( &state );

    _localNode->send( packet );
    const void* result = _requestHandler.waitRequest( packet.requestID );
    EQASSERT( result == NULL );

    switch( policy )
    {
        case Object::SHARE_NODE:
        case Object::SHARE_NEVER:
            break;

        case Object::SHARE_THREAD:
        {
            if( _threadObjects.get() == NULL )
                _threadObjects = new IDHash<Object*>;
            (*_threadObjects.get())[id] = state.object;
            break;
        }

        default:
            EQUNREACHABLE;
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
                state->objectID       = id;
                _objectInstStates[id] = state;
            }
            return _instObject( state );
        }
    
        if( state )
        {
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
        const CommandResult result = (*iter)->handleCommand( node, objPacket );
        switch( result )
        {
            case COMMAND_PROPAGATE:
                break;
            default:
                return result;
        }
    }
    return COMMAND_HANDLED;
}

CommandResult Session::_instObject( GetObjectState* state )
{
    CHECK_THREAD( _recvThreadID );
    const uint32_t objectID = state->objectID;

    switch( state->instState )
    {
        case Object::INST_UNKNOWN:
        {
            RefPtr<Node> master = _pollIDMaster( objectID );
            if( master.isValid( ))
            {
                if( master != _localNode )
                    _sendInitObject( state, master );
                // else hope that the object's instanciation is pending

                return COMMAND_RESCHEDULE;
            }

            if( _isMaster )
                return COMMAND_ERROR;

            SessionGetObjectMasterPacket packet( _id );
            packet.objectID = objectID;
            _server->send( packet );
            state->instState = Object::INST_GETMASTERID;
            return COMMAND_RESCHEDULE;
        }
        
        case Object::INST_GOTMASTER:
        {
            RefPtr<Node> master = _pollIDMaster( objectID );
            if( !master )
            {
                state->instState = Object::INST_UNKNOWN;
                return COMMAND_ERROR;
            }

            _sendInitObject( state, master );
            return COMMAND_RESCHEDULE;
        }
            
        case Object::INST_ERROR:
            return COMMAND_ERROR;

        case Object::INST_GETMASTERID:
        case Object::INST_GETMASTER:
        case Object::INST_INIT:
            return COMMAND_RESCHEDULE;

        default:
            return COMMAND_ERROR;
    }
}

void Session::_sendInitObject( GetObjectState* state, RefPtr<Node> master )
{
    SessionInitObjectPacket packet( _id );
    packet.objectID = state->objectID;
    master->send( packet );
    state->instState = Object::INST_INIT;
}
            
CommandResult Session::_cmdGenIDs( Node* node, const Packet* pkg )
{
    SessionGenIDsPacket*     packet = (SessionGenIDsPacket*)pkg;
    EQINFO << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );

    reply.id = _masterPool.genIDs( packet->range );
    node->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Node* node, const Packet* pkg )
{
    SessionGenIDsReplyPacket* packet = (SessionGenIDsReplyPacket*)pkg;
    EQINFO << "Cmd gen IDs reply: " << packet << endl;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->id );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Node* node, const Packet* pkg )
{
    SessionSetIDMasterPacket* packet = (SessionSetIDMasterPacket*)pkg;
    EQINFO << "Cmd set ID master: " << packet << endl;

    RefPtr<Node> master = _localNode->getNode( packet->masterID );
    EQASSERT( master.isValid( ));

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->start + packet->range, master};
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
    const uint32_t nInfos = _idMasterInfos.size();
    for( uint32_t i=0; i<nInfos; ++i )
    {
        IDMasterInfo& info = _idMasterInfos[i];
        if( packet->id >= info.start && packet->id < info.end )
        {
            reply.start    = info.start;
            reply.end      = info.end;
            reply.masterID = info.master->getNodeID();

            info.slaves.push_back( node );
            break;
        }
    }

    node->send( reply );
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

    RefPtr<Node> master = _localNode->getNode( packet->masterID );

    if( !master )
    {
        // TODO: query connection description, create and connect node
        EQUNIMPLEMENTED;
    }

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->end, master};
    _idMasterInfos.push_back( info );

    _requestHandler.serveRequest( packet->requestID, master.get( ));
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObjectMaster( Node* node, const Packet* pkg )
{
    SessionGetObjectMasterPacket* packet = (SessionGetObjectMasterPacket*)pkg;
    SessionGetObjectMasterReplyPacket reply( packet );
    EQINFO << "Cmd get object master " << packet << endl;

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
            reply.masterID = info.master->getNodeID();

            info.slaves.push_back( node );
            break;
        }
    }

    EQINFO << "Get object master reply " << &reply << endl;
    node->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetObjectMasterReply( Node* node, const Packet* pkg)
{
    CHECK_THREAD( _recvThreadID );
    SessionGetObjectMasterReplyPacket* packet = 
        (SessionGetObjectMasterReplyPacket*)pkg;
    EQINFO << "Cmd get object master reply " << packet << endl;
 
    const uint32_t  id    = packet->objectID;
    GetObjectState* state = _objectInstStates[id];
    EQASSERT( state );

    if( packet->start == 0 ) // not found
    {
        state->instState = Object::INST_ERROR;
        return COMMAND_HANDLED;
    }

    RefPtr<Node> master = _localNode->getNode( packet->masterID );

    if( master.isValid( ))
    {
        // TODO thread-safety: _idMasterInfos is also read & written by app
        IDMasterInfo info = { packet->start, packet->end, master};
        _idMasterInfos.push_back( info );
        
        state->instState = Object::INST_GOTMASTER;
        return COMMAND_HANDLED;
    }

    // query connection description, create and connect node 
    if( state->instState == Object::INST_GETMASTER )
        return COMMAND_RESCHEDULE;
    
    state->instState = Object::INST_GETMASTER;
    
    NodeGetConnectionDescriptionPacket cdPacket;
    cdPacket.nodeID = packet->masterID;
    cdPacket.index  = 0;
    
    _server->send( cdPacket );
    
    return COMMAND_RESCHEDULE;
}

CommandResult Session::_cmdGetObject( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _recvThreadID );

    SessionGetObjectPacket* packet = (SessionGetObjectPacket*)pkg;
    EQINFO << "Cmd get object " << packet << endl;

    GetObjectState*         state  = (GetObjectState*)
        _requestHandler.getRequestData( packet->requestID );
    EQASSERT( state );

    const uint32_t id     = state->objectID;

    if( state->object ) // successfully instanciated
    {
        _objectInstStates.erase( id );
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }

    if( state->policy == Object::SHARE_NODE )
    {
        Object* object = _nodeObjects[id];

        if( object ) // per-node object instanciated already
        {
            state->object = object;
            _requestHandler.serveRequest( packet->requestID, NULL );
            return COMMAND_HANDLED;
        }
    }

    if( !packet->pending ) // first iteration of instantiation
    {
        if( _objectInstStates[id] != NULL ) // instantion with same ID pending
            return COMMAND_RESCHEDULE;

        // mark pending instantion 
        packet->pending       = true;
        _objectInstStates[id] = state;
    }

    if( _instObject( state ) == COMMAND_ERROR )
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }
    return COMMAND_RESCHEDULE;
}

CommandResult Session::_cmdInitObject( Node* node, const Packet* pkg )
{
    SessionInitObjectPacket* packet = (SessionInitObjectPacket*)pkg;
    EQINFO << "Cmd init object " << packet << endl;

    const uint32_t   id      = packet->objectID;
    vector<Object*>& objects = _registeredObjects[id];

    for( vector<Object*>::iterator iter = objects.begin();
         iter != objects.end(); ++iter )
    {
        Object* object = *iter;
        if( object->getID() != id || !object->_master )
            continue;

        object->instanciateOnNode( node );
        return COMMAND_HANDLED;
    }

    // (hopefully) not yet instanciated -> reschedule
    return COMMAND_RESCHEDULE;
}

CommandResult Session::_cmdInstanciateObject( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _recvThreadID );

    SessionInstanciateObjectPacket* packet = 
        (SessionInstanciateObjectPacket*)pkg;
    EQINFO << "Cmd instanciate object " << packet << " from " << node << endl;

    const uint32_t id     = packet->objectID;
    Object*        object = instanciateObject( packet->objectType,
                                               packet->objectData, 
                                               packet->objectDataSize );
    if( !object )
    {
        EQWARN << "Session failed to instanciate object of type "
               << packet->objectType << endl;
        return COMMAND_ERROR;
    }

    object->ref();
    object->_master = packet->isMaster;
    if( packet->isMaster ) // Assumes that sender is a subscribed slave
        object->addSlave( node );

    GetObjectState* state = _objectInstStates[id];
    if( state )
    {
        state->object = object;
        if( state->policy == Object::SHARE_NODE )
        {
            EQASSERT( !_nodeObjects[id] );
            _nodeObjects[id] = object;
        }
    }
    else
    {
        EQASSERT( !_nodeObjects[id] );
        _nodeObjects[id] = object;
    }        

    _addRegisteredObject( id, object );
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
