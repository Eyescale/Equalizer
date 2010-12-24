
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>.
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

#include "objectSlaveDataOStream.h"

#include "log.h"
#include "masterCM.h"
#include "object.h"
#include "objectDataIStream.h"
#include "objectPackets.h"

namespace co
{
ObjectSlaveDataOStream::ObjectSlaveDataOStream( const ObjectCM* cm )
        : ObjectDataOStream( cm )
        , _commit( true )
{}

ObjectSlaveDataOStream::~ObjectSlaveDataOStream()
{}

void ObjectSlaveDataOStream::sendData( const uint32_t compressor,
                                       const uint32_t nChunks,
                                       const void* const* chunks,
                                       const uint64_t* chunkSizes,
                                       const uint64_t size )
{
    ObjectSlaveDeltaPacket packet;
    packet.commit = _commit;
    packet.instanceID = _cm->getObject()->getMasterInstanceID();
    sendPacket( packet, compressor, nChunks, chunks, chunkSizes, size );
}

void ObjectSlaveDataOStream::sendFooter( const uint32_t compressor,
                                         const uint32_t nChunks,
                                         const void* const* chunks, 
                                         const uint64_t* chunkSizes,
                                         const uint64_t size )
{
    ObjectSlaveDeltaPacket packet;
    packet.last = true;
    packet.commit = _commit;
    packet.instanceID = _cm->getObject()->getMasterInstanceID();

    sendPacket( packet, compressor, nChunks, chunks, chunkSizes, size );

    _sequence = 0;
    _commit = co::base::UUID( true /* generate */ );
}

}
