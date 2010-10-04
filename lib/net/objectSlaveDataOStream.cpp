
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

#include "objectSlaveDataOStream.h"

#include "log.h"
#include "masterCM.h"
#include "object.h"
#include "packets.h"
#include "session.h"

#include <eq/base/idPool.h>

namespace eq
{
namespace net
{
ObjectSlaveDataOStream::ObjectSlaveDataOStream( const ObjectCM* cm )
        : ObjectDataOStream( cm )
        , _commit( true )
{}

ObjectSlaveDataOStream::~ObjectSlaveDataOStream()
{}

void ObjectSlaveDataOStream::_sendPacket( ObjectSlaveDeltaPacket& packet,
                                          const void* const* chunks,
                                          const uint64_t* chunkSizes,
                                          const uint64_t sizeUncompressed )
{
    const Object* object = _cm->getObject();   
     
    packet.version    = object->getVersion();
    packet.sequence   = _sequence++;
    packet.dataSize   = sizeUncompressed;
    packet.sessionID  = object->getSession()->getID();
    packet.objectID   = object->getID();
    packet.instanceID = object->getMasterInstanceID();

#if 0
    EQLOG( LOG_OBJECTS ) << "send " << &packet << " to " << _connections.size()
                         << " receivers " << std::endl;
#endif

    Connection::send( _connections, packet, chunks, chunkSizes, packet.nChunks);
}

void ObjectSlaveDataOStream::sendData( const uint32_t name,
                                       const uint32_t nChunks,
                                       const void* const* buffers,
                                       const uint64_t* sizes,
                                       const uint64_t sizeUncompressed )
{
    ObjectSlaveDeltaPacket packet;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    packet.commit         = _commit;
    _sendPacket( packet, buffers, sizes, sizeUncompressed );
}

void ObjectSlaveDataOStream::sendFooter( const uint32_t name, 
                                         const uint32_t nChunks,
                                         const void* const* buffers, 
                                         const uint64_t* sizes,
                                         const uint64_t sizeUncompressed )
{
    ObjectSlaveDeltaPacket packet;
    packet.last           = true;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    packet.commit         = _commit;
    _sendPacket( packet, buffers, sizes, sizeUncompressed );

    _sequence = 0;
    _commit = base::UUID( true /* generate */ );
}

}
}
