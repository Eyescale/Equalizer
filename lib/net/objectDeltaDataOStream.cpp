
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com>.
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>.
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

namespace eq
{
namespace net
{
ObjectDeltaDataOStream::ObjectDeltaDataOStream( const Object* object)
        : ObjectDataOStream( object )
{}

ObjectDeltaDataOStream::~ObjectDeltaDataOStream()
{}

void ObjectDeltaDataOStream::_sendPacket( ObjectDeltaPacket& packet,
                                          const void* const* chunks,
                                          const uint64_t* chunkSizes,
                                          const uint64_t sizeUncompressed )
{
    packet.version   = _version;
    packet.sequence  = _sequence++;
    packet.dataSize  = sizeUncompressed;
    packet.sessionID = _object->getSession()->getID();
    packet.objectID  = _object->getID();

#if 0
    EQLOG( LOG_OBJECTS ) << "send " << &packet << " to " << _connections.size()
                         << " receivers " << std::endl;
#endif

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

void ObjectDeltaDataOStream::sendData( const uint32_t name,
                                       const uint32_t nChunks,
                                       const void* const* buffers,
                                       const uint64_t* sizes,
                                       const uint64_t sizeUncompressed )
{
    ObjectDeltaPacket packet;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    _sendPacket( packet, buffers, sizes, sizeUncompressed );
}

void ObjectDeltaDataOStream::sendFooter( const uint32_t name, 
                                         const uint32_t nChunks,
                                         const void* const* buffers, 
                                         const uint64_t* sizes,
                                         const uint64_t sizeUncompressed )
{
    ObjectDeltaPacket packet;
    packet.last           = true;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    _sendPacket( packet, buffers, sizes, sizeUncompressed );
    _sequence = 0;
}

}
}
