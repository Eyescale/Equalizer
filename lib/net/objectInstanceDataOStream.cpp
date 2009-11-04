
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

#include "objectInstanceDataOStream.h"

#include "log.h"
#include "object.h"
#include "packets.h"
#include "session.h"

#include <eq/base/idPool.h>

namespace eq
{
namespace net
{
ObjectInstanceDataOStream::ObjectInstanceDataOStream( const Object* object)
        : ObjectDataOStream( object )
        , _instanceID( EQ_ID_ANY )
{}

ObjectInstanceDataOStream::~ObjectInstanceDataOStream()
{}

void ObjectInstanceDataOStream::sendBuffer( const void* buffer,
                                            const uint64_t size )
{
    EQASSERT( _instanceID = EQ_ID_NONE );
    EQASSERT( _nodeID == NodeID::ZERO );

    ObjectInstanceDataPacket packet;
    packet.version    = _version;
    packet.sequence   = _sequence++;
    packet.dataSize   = size;
    packet.sessionID  = _object->getSession()->getID();
    packet.objectID   = _object->getID();
    packet.instanceID = _instanceID;

    EQLOG( LOG_OBJECTS ) << "send " << &packet << " to " << _connections.size()
                         << " receivers " << std::endl;
    Connection::send( _connections, packet, buffer, size, true );
}

void ObjectInstanceDataOStream::sendFooter( const void* buffer, 
                                            const uint64_t size )
{
    ObjectInstancePacket packet;
    packet.version    = _version;
    packet.sequence   = _sequence++;
    packet.dataSize   = size;
    packet.sessionID  = _object->getSession()->getID();
    packet.objectID   = _object->getID();
    packet.instanceID = _instanceID;

    if( _nodeID == NodeID::ZERO )
    {
        EQASSERT( packet.nodeID == NodeID::ZERO );
    }
    else
    {
        EQASSERT( _instanceID != EQ_ID_NONE );
        EQASSERTINFO( _connections.size() == 1, 
                      "Expected multicast to one group" );

        packet.datatype = DATATYPE_EQNET_SESSION;
        packet.command = CMD_SESSION_INSTANCE;
        packet.nodeID = _nodeID;
    }

    EQLOG( LOG_OBJECTS ) << "send " << &packet << " to " << _connections.size()
                         << " receivers " << std::endl;

    packet.nodeID.convertToNetwork();
    Connection::send( _connections, packet, buffer, size, true );
    _sequence = 0;
}
}
}
