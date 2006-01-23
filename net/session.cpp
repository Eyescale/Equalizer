/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"
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
    if( !master->checkConnection( ))
        return;

    if( _isMaster )
    {
        IDMasterInfo info = { start, start+range, master };
        _idMasterInfos.push_back( info );
        return;
    }

    SessionSetIDMasterPacket packet( _id );
    packet.start     = start;
    packet.range     = range;

    RefPtr<Connection> connection = ( master->isConnected() ?
                                      master->getConnection() : 
                                      master->getListenerConnection( ));
    string  connectionDescription = connection->getDescription()->toString();
    
    _server->send( packet, connectionDescription );
}

Node* Session::getIDMaster( const uint32_t id )
{
    // look up locally
    const uint32_t nInfos = _idMasterInfos.size();
    for( uint32_t i=0; i<nInfos; ++i )
    {
        const IDMasterInfo& info = _idMasterInfos[i];
        if( id >= info.start && id < info.end )
                return info.master;
    }
    
    if( _isMaster )
        return NULL;

    // ask master
    SessionGetIDMasterPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    packet.id        = id;

    _server->send( packet );
    IDMasterInfo* info = (IDMasterInfo*)_requestHandler.waitRequest( 
        packet.requestID );

    if( !info )
        return NULL;
    
    _idMasterInfos.push_back( *info );

    Node*  master = info->master;
    delete info;
    return master;
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

//===========================================================================
// Packet handling
//===========================================================================

void Session::dispatchPacket( Node* node, const Packet* packet)
{
    VERB << "dispatch " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            handleCommand( node, packet );
            break;

        case DATATYPE_EQNET_OBJECT:
        {
            const ObjectPacket* objPacket = (ObjectPacket*)packet;
            const uint32_t      id        = objPacket->objectID;
            Object*             object    = _registeredObjects[id];
            
            if( !object )
            {
                ERROR << "Received request for unregistered object of id "
                      << id << endl;
                ASSERT( object );
                return;
            }

            object->handleCommand( node, objPacket );
            break;
        }

        default:
            WARN << "Unhandled packet " << packet << endl;
    }
}

void Session::_cmdGenIDs( Node* node, const Packet* pkg )
{
    SessionGenIDsPacket*     packet = (SessionGenIDsPacket*)pkg;
    INFO << "Cmd gen IDs: " << packet << endl;

    SessionGenIDsReplyPacket reply( packet );

    reply.id = _masterPool.genIDs( packet->range );
    node->send( reply );
}

void Session::_cmdGenIDsReply( Node* node, const Packet* pkg )
{
    SessionGenIDsReplyPacket* packet = (SessionGenIDsReplyPacket*)pkg;
    INFO << "Cmd gen IDs reply: " << packet << endl;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->id );
}

void Session::_cmdSetIDMaster( Node* node, const Packet* pkg )
{
    SessionSetIDMasterPacket* packet = (SessionSetIDMasterPacket*)pkg;

//     Node* localNode = Node::getLocalNode();
//     Node* master    = localNode->findNodeByConnection( connectionDescription );
//     ASSERT( master );

}

// void Session::_reqSetIDMaster( Node* node, const Packet* pkg )
// {
//     SessionSetIDMasterPacket* packet = (SessionSetIDMasterPacket*)pkg;

//     IDMasterInfo info = 

//     info.
// }

void Session::_cmdGetIDMaster( Node* node, const Packet* packet )
{
}

void Session::_cmdGetIDMasterReply( Node* node, const Packet* packet )
{
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
