
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#include "objectDeltaDataOStream.h"

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
ObjectDeltaDataOStream::ObjectDeltaDataOStream( const Object* object)
        : ObjectDataOStream( object )
{}

ObjectDeltaDataOStream::~ObjectDeltaDataOStream()
{}

void ObjectDeltaDataOStream::sendBuffer( const void* buffer,
                                         const uint64_t size )
{
    ObjectDeltaDataPacket deltaPacket;
    deltaPacket.deltaSize  = size;
    deltaPacket.sessionID  = _object->getSession()->getID();
    deltaPacket.objectID   = _object->getID();

    EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << " to " 
                         << _connections.size() << " receivers " << endl;
    Connection::send( _connections, deltaPacket, buffer, size, true );
}

void ObjectDeltaDataOStream::sendFooter( const void* buffer, 
                                         const uint64_t size )
{
    ObjectDeltaPacket deltaPacket;
    deltaPacket.version    = _version;
    deltaPacket.deltaSize  = size;
    deltaPacket.sessionID  = _object->getSession()->getID();
    deltaPacket.objectID   = _object->getID();

    EQLOG( LOG_OBJECTS ) << "send " << &deltaPacket << " to " 
                         << _connections.size() << " receivers " << endl;
    Connection::send( _connections, deltaPacket, buffer, size, true );
}
}
}
