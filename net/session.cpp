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

Session::Session( const uint32_t nCommands )
        : Base( nCommands ),
          _id(INVALID_ID),
          _server(NULL),
          _isMaster(false)
{
    ASSERT( nCommands >= CMD_SESSION_CUSTOM );
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
    registerCommand( CMD_SESSION_GET_MOBJECT_MASTER, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetMobjectMaster ));
    registerCommand( CMD_SESSION_GET_MOBJECT_MASTER_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetMobjectMasterReply ));
    registerCommand( CMD_SESSION_GET_MOBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdGetMobject ));
    registerCommand( CMD_SESSION_INIT_MOBJECT, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdInitMobject ));
    registerCommand( CMD_SESSION_INIT_MOBJECT_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Session::_cmdInitMobjectReply ));

    _localPool.genIDs( _localPool.getCapacity( )); // reserve all IDs
    INFO << "New " << this << endl;
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
    ASSERT( master->checkConnection( ));

    IDMasterInfo info = { start, start+range, master };
    _idMasterInfos.push_back( info );

    if( _isMaster )
        return;

    SessionSetIDMasterPacket packet( _id );
    packet.start = start;
    packet.range = range;

    RefPtr<Connection> connection = ( master->isConnected() ?
                                      master->getConnection() : 
                                      master->getListenerConnection( ));
    string  connectionDescription = connection->getDescription()->toString();
    
    _server->send( packet, connectionDescription );
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

    // ask master
    SessionGetIDMasterPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    packet.id        = id;

    _localNode->send( packet );
    return (Node*)_requestHandler.waitRequest( packet.requestID );
}

//---------------------------------------------------------------------------
// object mapping
//---------------------------------------------------------------------------
void Session::registerObject( Object* object )
{
    const uint32_t id = genIDs( 1 );
    addRegisteredObject( id, object );
}

void Session::addRegisteredObject( const uint32_t id, Object* object )
{
    ASSERT( !_registeredObjects[id] );
    _registeredObjects[id] = object;
    object->_id            = id;
    object->_sessionID     = _id;
    VERB << "registered object " << (void*)object << " id " << id
         << " session id " << _id << endl;
}

void Session::deregisterObject( Object* object )
{
    const uint32_t id = object->getID();
    if( _registeredObjects.erase(id) == 0 )
        return;
    
    object->_id        = INVALID_ID;
    object->_sessionID = INVALID_ID;
    freeIDs( id, 1 );
}


void Session::registerMobject( Mobject* object, Node* master )
{
    const uint32_t id = genIDs( 1 );
    setIDMaster( id, 1, master );
    addRegisteredObject( id, object );
}

void Session::deregisterMobject( Mobject* object )
{
    const uint32_t id = object->getID();
    if( _registeredObjects.erase(id) == 0 )
        return;
    
    // TODO: unsetIDMaster( object->_id );
    object->_id        = INVALID_ID;
    object->_sessionID = INVALID_ID;
    freeIDs( id, 1 );
}

Mobject* Session::getMobject( const uint32_t id )
{
    Object* object = _registeredObjects[id];
    if( object )
    {
        ASSERT( dynamic_cast<Mobject*>(object) );
        return static_cast<Mobject*>(object);
    }

    SessionGetMobjectPacket packet( _id );

    packet.requestID = _requestHandler.registerRequest();
    packet.mobjectID = id;

    _server->send( packet );
    return (Mobject*)_requestHandler.waitRequest( packet.requestID );
}

Mobject* Session::instanciateMobject( const uint32_t type, const char* data )
{
    switch( type )
    {
        case MOBJECT_EQNET_BARRIER:
        {
            const uint32_t height = atoi( data );
            return new Barrier( height );
        }

        default:
            return NULL;
    }
}

//===========================================================================
// Packet handling
//===========================================================================

CommandResult Session::dispatchPacket( Node* node, const Packet* packet )
{
    VERB << "dispatch " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            return handleCommand( node, packet );

        case DATATYPE_EQNET_OBJECT:
            return _handleObjectCommand( node, packet );

        case DATATYPE_EQNET_MOBJECT:
            return _handleMobjectCommand( node, packet );

        default:
            WARN << "Undispatched packet " << packet << endl;
            return COMMAND_ERROR;
    }
}

CommandResult Session::_handleObjectCommand( Node* node, const Packet* packet )
{
    const ObjectPacket* objPacket = (ObjectPacket*)packet;
    const uint32_t      id        = objPacket->objectID;
    Object*             object    = _registeredObjects[id];
    
    if( !object )
    {
        ASSERTINFO( object, "Received request for unregistered object");
        return COMMAND_ERROR;
    }

    return object->handleCommand( node, objPacket );
}

CommandResult Session::_handleMobjectCommand( Node* node, const Packet* packet )
{
    const MobjectPacket* objPacket = (MobjectPacket*)packet;
    const uint32_t       id        = objPacket->objectID;
    Object*              object    = _registeredObjects[id];
    
    if( object )
        return object->handleCommand( node, objPacket );

    return _instMobject( id );
}

CommandResult Session::_instMobject( const uint32_t id )
{
    const Mobject::InstState state = _mobjectStates[id];

    switch( state )
    {
        case Mobject::INST_UNKNOWN:
        {
            RefPtr<Node> master = _pollIDMaster( id );
            if( master.isValid( ))
            {
                SessionInitMobjectPacket packet( _id );
                packet.mobjectID = id;
                master->send( packet );
                _mobjectStates[id] = Mobject::INST_INIT;
                return COMMAND_RESCHEDULE;
            }

            if( _isMaster )
                return COMMAND_ERROR;

            SessionGetMobjectMasterPacket packet( _id );
            packet.mobjectID = id;
            _server->send( packet );
            _mobjectStates[id] = Mobject::INST_GETMASTER;
            return COMMAND_RESCHEDULE;
        }
        
        case Mobject::INST_GOTMASTER:
        {
            RefPtr<Node> master = _pollIDMaster( id );
            if( !master )
            {
                _mobjectStates[id] = Mobject::INST_UNKNOWN;
                return COMMAND_ERROR;
            }

            SessionInitMobjectPacket packet( _id );
            packet.mobjectID = id;
            master->send( packet );
            _mobjectStates[id] = Mobject::INST_INIT;
            return COMMAND_RESCHEDULE;
        }
            
        case Mobject::INST_ERROR:
            _mobjectStates[id] = Mobject::INST_UNKNOWN;
            return COMMAND_ERROR;

        default:
            return COMMAND_RESCHEDULE;
    }
}

CommandResult Session::_cmdGenIDs( Node* node, const Packet* pkg )
{
    SessionGenIDsPacket*     packet = (SessionGenIDsPacket*)pkg;
    INFO << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );

    reply.id = _masterPool.genIDs( packet->range );
    node->send( reply );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGenIDsReply( Node* node, const Packet* pkg )
{
    SessionGenIDsReplyPacket* packet = (SessionGenIDsReplyPacket*)pkg;
    INFO << "Cmd gen IDs reply: " << packet << endl;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->id );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdSetIDMaster( Node* node, const Packet* pkg )
{
    SessionSetIDMasterPacket* packet = (SessionSetIDMasterPacket*)pkg;
    INFO << "Cmd set ID master: " << packet << endl;

    RefPtr<Node> master    = _localNode->getNodeWithConnection( 
        packet->connectionDescription );

    ASSERT( master.isValid( ));

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
    string                        connectionDescription;

    reply.start = 0;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    const uint32_t nInfos = _idMasterInfos.size();
    for( uint32_t i=0; i<nInfos; ++i )
    {
        IDMasterInfo& info = _idMasterInfos[i];
        if( packet->id >= info.start && packet->id < info.end )
        {
            reply.start = info.start;
            reply.end   = info.end;

            RefPtr<Connection> connection = ( info.master->isConnected() ?
                                              info.master->getConnection() : 
                                         info.master->getListenerConnection( ));
            connectionDescription = connection->getDescription()->toString();
            
            info.slaves.push_back( node );
            break;
        }
    }

    node->send( reply, connectionDescription );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetIDMasterReply( Node* node, const Packet* pkg )
{
    SessionGetIDMasterReplyPacket* packet = (SessionGetIDMasterReplyPacket*)pkg;

    if( packet->start == 0 ) // not found
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }

    RefPtr<Node> master = _localNode->getNodeWithConnection( packet->
                                                        connectionDescription );
    ASSERT( master );

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->end, master};
    _idMasterInfos.push_back( info );

    _requestHandler.serveRequest( packet->requestID, master.get( ));
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetMobjectMaster( Node* node, const Packet* pkg )
{
    SessionGetMobjectMasterPacket* packet = (SessionGetMobjectMasterPacket*)pkg;
    SessionGetMobjectMasterReplyPacket reply( packet );
    string                        connectionDescription;

    reply.start = 0;

    // TODO thread-safety: _idMasterInfos is also read & written by app
    const uint32_t id     = packet->mobjectID;
    const uint32_t nInfos = _idMasterInfos.size();
    for( uint32_t i=0; i<nInfos; ++i )
    {
        IDMasterInfo& info = _idMasterInfos[i];
        if( id >= info.start && id < info.end )
        {
            reply.start = info.start;
            reply.end   = info.end;

            RefPtr<Connection> connection = ( info.master->isConnected() ?
                                              info.master->getConnection() : 
                                         info.master->getListenerConnection( ));
            connectionDescription = connection->getDescription()->toString();
            
            info.slaves.push_back( node );
            break;
        }
    }

    node->send( reply, connectionDescription );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetMobjectMasterReply( Node* node, const Packet* pkg)
{
    SessionGetMobjectMasterReplyPacket* packet = 
        (SessionGetMobjectMasterReplyPacket*)pkg;

    const uint32_t id = packet->mobjectID;

    if( packet->start == 0 ) // not found
    {
        _mobjectStates[id] = Mobject::INST_ERROR;
        return COMMAND_HANDLED;
    }

    RefPtr<Node> master = _localNode->getNodeWithConnection( packet->
                                                        connectionDescription );
    ASSERT( master );

    // TODO thread-safety: _idMasterInfos is also read & written by app
    IDMasterInfo info = { packet->start, packet->end, master};
    _idMasterInfos.push_back( info );
    _mobjectStates[id] = Mobject::INST_GOTMASTER;
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdGetMobject( Node* node, const Packet* pkg )
{
    ASSERT( node == _localNode.get( ));
    SessionGetMobjectPacket* packet = (SessionGetMobjectPacket*)pkg;
    INFO << "cmd get mobject " << packet << endl;

    const uint32_t id     = packet->mobjectID;
    Object*        object = _registeredObjects[id];

    if( object )
    {
        ASSERT( dynamic_cast<Mobject*>(object) );
        _requestHandler.serveRequest( packet->requestID, object );
        return COMMAND_HANDLED;
    }

    if( _instMobject( id ) == COMMAND_ERROR )
    {
        _requestHandler.serveRequest( packet->requestID, NULL );
        return COMMAND_HANDLED;
    }
    return COMMAND_RESCHEDULE;
}

CommandResult Session::_cmdInitMobject( Node* node, const Packet* pkg )
{
    SessionInitMobjectPacket* packet = (SessionInitMobjectPacket*)pkg;
    INFO << "cmd init mobject " << packet << endl;

    const uint32_t id      = packet->mobjectID;
    Object*        object  = _registeredObjects[id];
    ASSERT( object );
    ASSERT( dynamic_cast<Mobject*>(object) );
    
    Mobject*       mobject = (Mobject*)object;
    SessionInitMobjectReplyPacket reply( packet );

    string mobjectData;
    mobject->getInstanceInfo( &reply.mobjectType, mobjectData );
    
    node->send( reply, mobjectData );
    return COMMAND_HANDLED;
}

CommandResult Session::_cmdInitMobjectReply( Node* node, const Packet* pkg )
{
    SessionInitMobjectReplyPacket* packet = (SessionInitMobjectReplyPacket*)pkg;
    INFO << "cmd init mobject reply " << packet << endl;

    const uint32_t id      = packet->mobjectID;
    ASSERT( !_registeredObjects[id] );

    Mobject* mobject = instanciateMobject( packet->mobjectType, 
                                           packet->mobjectData );

    if( !mobject )
        return COMMAND_ERROR;

    mobject->ref();
    addRegisteredObject( id, mobject );
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
