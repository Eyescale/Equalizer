
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectInstanceDataOStream.h"

#include "log.h"
#include "object.h"
#include "packets.h"
#include "session.h"

#include <eq/base/idPool.h>

using namespace std;

namespace eq
{
namespace net
{
ObjectInstanceDataOStream::ObjectInstanceDataOStream( const Object* object)
        : ObjectDataOStream( object )
        , _sequence( 0 )
{}

ObjectInstanceDataOStream::~ObjectInstanceDataOStream()
{}

void ObjectInstanceDataOStream::sendBuffer( const void* buffer,
                                            const uint64_t size )
{
    ObjectInstanceDataPacket dataPacket;
    dataPacket.dataSize   = size;
    dataPacket.sessionID  = _object->getSession()->getID();
    dataPacket.objectID   = _object->getID();
    dataPacket.instanceID = _instanceID;
    dataPacket.sequence   = _sequence++;

    EQLOG( LOG_OBJECTS ) << "send " << &dataPacket << " to " 
                         << _connections.size() << " receivers " << endl;
    Connection::send( _connections, dataPacket, buffer, size, true );
}

void ObjectInstanceDataOStream::sendFooter( const void* buffer, 
                                            const uint64_t size )
{
    ObjectInstancePacket instancePacket;
    instancePacket.version    = _version;
    instancePacket.dataSize   = size;
    instancePacket.sessionID  = _object->getSession()->getID();
    instancePacket.objectID   = _object->getID();
    instancePacket.instanceID = _instanceID;
    instancePacket.sequence   = _sequence++;

    EQLOG( LOG_OBJECTS ) << "send " << &instancePacket << " to " 
                         << _connections.size() << " receivers " << endl;
    Connection::send( _connections, instancePacket, buffer, size, true );
}
}
}
