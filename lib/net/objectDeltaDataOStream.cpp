
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectDeltaDataOStream.h"

#include "log.h"
#include "object.h"
#include "packets.h"
#include "session.h"

#include <eq/base/idPool.h>

using namespace std;

namespace eqNet
{
ObjectDeltaDataOStream::ObjectDeltaDataOStream( const Object* object)
        : ObjectDataOStream( object )
{}

ObjectDeltaDataOStream::~ObjectDeltaDataOStream()
{}

void ObjectDeltaDataOStream::sendBuffer( const void* buffer,
                                         const uint64_t size )
{
    ObjectDeltaDataPacket deltaPacket;
    deltaPacket.deltaSize = size;
    deltaPacket.sessionID = _object->getSession()->getID();
    deltaPacket.objectID  = _object->getID();

    EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << " to " 
                         << _connections.size() << " receivers " << endl;
    Connection::send( _connections, deltaPacket, buffer, size, true );
}

void ObjectDeltaDataOStream::sendFooter( const void* buffer, 
                                         const uint64_t size )
{
    ObjectDeltaPacket deltaPacket;
    deltaPacket.version   = _version;
    deltaPacket.deltaSize = size;
    deltaPacket.sessionID = _object->getSession()->getID();
    deltaPacket.objectID  = _object->getID();

    EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << " to " 
                         << _connections.size() << " receivers " << endl;
    Connection::send( _connections, deltaPacket, buffer, size, true );
}
}
