
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "command.h"
#include "log.h"
#include "nullCM.h"
#include "packets.h"
#include "session.h"

#include <eq/base/scopedMutex.h>
#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Object::_construct()
{
    _cm            = ObjectCM::ZERO;
    _session       = 0;
    _id            = EQ_ID_INVALID;
    _instanceID    = EQ_ID_INVALID;
    _instanceData     = 0;
    _instanceDataSize = 0;
    _deltaData     = 0;
    _deltaDataSize = 0;

    registerCommand( CMD_OBJECT_INIT,
                     CommandFunc<Object>( this, &Object::_cmdForward ));
    registerCommand( CMD_OBJECT_SYNC, 
                     CommandFunc<Object>( this, &Object::_cmdForward ));
    registerCommand( CMD_OBJECT_COMMIT, 
                     CommandFunc<Object>( this, &Object::_cmdForward ));
}

Object::Object()
{
    _construct();
}

Object::Object( const Object& from )
{
    EQASSERT( from._id == EQ_ID_INVALID );

    _construct();
    _instanceData     = from._instanceData;
    _instanceDataSize = from._instanceDataSize;
    _deltaData        = from._deltaData;
    _deltaDataSize    = from._deltaDataSize;
}

Object::~Object()
{
    if( _session ) // Still registered
        EQERROR << "Object is still registered in session " << _session->getID()
                << " in destructor" << endl;

    if( _cm != ObjectCM::ZERO )
        delete _cm;
    _cm = 0;
}

void Object::_setChangeManager( ObjectCM* cm )
{
    if( _cm != ObjectCM::ZERO )
    {
        EQASSERTINFO( cm == ObjectCM::ZERO, 
                      "Overriding existing object change manager" );
        delete _cm;
    }

    if( _threadSafe )
        cm->makeThreadSafe();

    _cm = cm;
}

void Object::makeThreadSafe()
{
    EQASSERT( _id == EQ_ID_INVALID );
    if( _threadSafe ) return;

    _threadSafe = true;
    _cm->makeThreadSafe();
}

RefPtr<Node> Object::getLocalNode()
{ 
    return _session ? _session->getLocalNode() : 0; 
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

bool Object::send( NodeVector& nodes, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet );
}
bool Object::send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet, data, size );
}

uint32_t Object::commit()
{
    const uint32_t requestID = commitNB();
    return commitSync( requestID );
}

void Object::setInstanceData( void* data, const uint64_t size )
{
    _instanceData     = data;
    _instanceDataSize = size;

    if( _deltaData )
        return;
    _deltaData     = data;
    _deltaDataSize = size;
}
