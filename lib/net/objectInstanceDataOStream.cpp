
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010, Cedric Stalder  <cedric.stalder@gmail.com>
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
#include "masterCM.h"
#include "object.h"
#include "objectPackets.h"
#include "packets.h"
#include "session.h"

namespace eq
{
namespace net
{
ObjectInstanceDataOStream::ObjectInstanceDataOStream( const ObjectCM* cm )
        : ObjectDataOStream( cm )
        , _instanceID( EQ_ID_ANY )
{}

ObjectInstanceDataOStream::~ObjectInstanceDataOStream()
{}

void ObjectInstanceDataOStream::_sendPacket( ObjectInstancePacket& packet,
                                             const void* const* chunks,
                                             const uint64_t* chunkSizes,
                                             const uint64_t sizeUncompressed )
{
    packet.version    = _version;
    packet.sequence   = _sequence++;
    packet.dataSize   = sizeUncompressed;

    const Object* object    = _cm->getObject();
    packet.sessionID        = object->getSession()->getID();
    packet.objectID         = object->getID();
    packet.instanceID       = _instanceID;
    packet.masterInstanceID = object->getInstanceID();

    _cm->tunePacket( packet );

    if( _nodeID == NodeID::ZERO )
    {
        EQASSERT( packet.nodeID == NodeID::ZERO );
    }
    else
    {
        EQASSERT( _instanceID != EQ_ID_NONE );
        EQASSERTINFO( _connections.size() == 1,
                      "Expected multicast to one group" );

        packet.type = PACKETTYPE_EQNET_SESSION;
        packet.command = CMD_SESSION_OBJECT_INSTANCE;
        packet.nodeID = _nodeID;
    }

    if( sizeUncompressed > 0 )
        Connection::send( _connections, packet, chunks, chunkSizes, 
                          packet.nChunks );
    else
    {
        EQASSERT( packet.nChunks == 1 );
        EQASSERT( chunkSizes[ 0 ] == 0 );
        Connection::send( _connections, packet );
    }
}

void ObjectInstanceDataOStream::sendData( const uint32_t name,
                                          const uint32_t nChunks,
                                          const void* const* chunks,
                                          const uint64_t* chunkSizes,
                                          const uint64_t sizeUncompressed )
{
    ObjectInstancePacket packet;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    _sendPacket( packet, chunks, chunkSizes, sizeUncompressed );
}

void ObjectInstanceDataOStream::sendFooter( const uint32_t name, 
                                            const uint32_t nChunks,
                                            const void* const* chunks,
                                            const uint64_t* chunkSizes,
                                            const uint64_t sizeUncompressed )
{
    ObjectInstancePacket packet;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    packet.last = true;
    _sendPacket( packet, chunks, chunkSizes, sizeUncompressed );
    _sequence = 0;
}
}
}
