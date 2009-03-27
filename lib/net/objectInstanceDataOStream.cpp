
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
