/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"
#include "connection.h"
#include "connectionDescription.h"
#include "packets.h"
#include "session.h"

#include <eq/base/log.h>

#include <alloca.h>

using namespace eqNet;
using namespace std;

#define MIN_ID_RANGE 1024

Session::Session( const uint nCommands )
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

    _idPool.genIDs( _idPool.getCapacity( )); // reserve all IDs
    INFO << "New " << this << endl;
}

void Session::map( Node* server, const uint id, const std::string& name, 
                   const bool isMaster )
{
    _server   = server;
    _id       = id;
    _name     = name;
    _isMaster = isMaster;

    if( isMaster )
        _idPool.freeIDs( 1, _idPool.getCapacity( )); // free IDs for further use

    INFO << (isMaster ? "master" : "client") << " session, id " << id 
         << ", name " << name << ", served by node " << server << endl;
    // _state = mapped;
}

uint Session::genIDs( const uint range )
{
    // try local pool
    uint id = _idPool.genIDs( range );
    if( id || _isMaster ) // got an id or master pool is depleted
        return id;

    SessionGenIDsPacket packet( id );
    packet.requestID = _requestHandler.registerRequest();
    packet.range     = (range > MIN_ID_RANGE) ? range : MIN_ID_RANGE;

    _server->send( packet );
    id = (uint)(long long)_requestHandler.waitRequest( packet.requestID );

    if( !id || range >= MIN_ID_RANGE )
        return id;

    // We allocated more IDs than requested - let the pool handle the details
    _idPool.freeIDs( id, MIN_ID_RANGE );
    return _idPool.genIDs( range );
}

void Session::freeIDs( const uint start, const uint range )
{
    _idPool.freeIDs( start, range );
    // could return IDs to master sometimes ?
}

void Session::registerObject( Object* object )
{
    const uint id = genIDs( 1 );
    addRegisteredObject( id, object );
}

void Session::addRegisteredObject( const uint id, Object* object )
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
    const uint id = object->getID();
    if( _registeredObjects.erase(id) == 0 )
        return;
    
    object->_id        = INVALID_ID;
    object->_sessionID = INVALID_ID;
    freeIDs( id, 1 );
}

//===========================================================================
// Packet handling
//===========================================================================

void Session::dispatchPacket( Node* node, const SessionPacket* packet)
{
    INFO << "dispatch " << packet << endl;

    switch( packet->datatype )
    {
        case DATATYPE_EQNET_SESSION:
            handleCommand( node, packet );
            break;

        case DATATYPE_EQNET_OBJECT:
        {
            const ObjectPacket* objPacket = (ObjectPacket*)packet;
            const uint          id        = objPacket->objectID;
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
    SessionGenIDsReplyPacket reply( packet );

    reply.id = _idPool.genIDs( packet->range );
    node->send( reply );
}

void Session::_cmdGenIDsReply( Node* node, const Packet* pkg )
{
    SessionGenIDsReplyPacket* packet = (SessionGenIDsReplyPacket*)pkg;
    _requestHandler.serveRequest( packet->requestID, (void*)packet->id );
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
