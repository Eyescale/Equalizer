
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>.
 *               2010, Cedric Stalder  <cedric.stalder@gmail.com>.
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

#include "objectDataOStream.h"

#include "objectCM.h"
#include "objectPackets.h"

#include <co/plugins/compressorTypes.h>

namespace co
{

void ObjectDataOStream::sendPacket( ObjectDataPacket& packet,
                                    const uint32_t compressor,
                                    const uint32_t nChunks,
                                    const void* const* chunks,
                                    const uint64_t* chunkSizes,
                                    const uint64_t size )
{
    packet.compressorName = compressor;
    packet.nChunks = nChunks;
    packet.version = _version;
    packet.sequence = _sequence++;
    packet.dataSize = size;

    const Object* object = _cm->getObject();
    packet.objectID  = object->getID();

#if 0
    EQLOG( LOG_OBJECTS ) << "send " << &packet << " to " << _connections.size()
                         << " receivers " << std::endl;
#endif

    if( size == 0 )
    {
        EQASSERT( nChunks == 1 );
        EQASSERT( chunkSizes[ 0 ] == 0 );
        Connection::send( _connections, packet );
    }
    else if( compressor == EQ_COMPRESSOR_NONE )
    {
        EQASSERT( nChunks == 1 );
        EQASSERT( chunkSizes[ 0 ] == size );
        Connection::send( _connections, packet, chunks[0], chunkSizes[0] );
    }
    else
        Connection::send( _connections, packet, chunks, chunkSizes, nChunks );
}

}
