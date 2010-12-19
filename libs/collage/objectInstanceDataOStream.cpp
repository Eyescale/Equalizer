
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
#include "objectDataIStream.h"
#include "objectPackets.h"

namespace co
{
ObjectInstanceDataOStream::ObjectInstanceDataOStream( const ObjectCM* cm )
        : ObjectDataOStream( cm )
        , _instanceID( EQ_INSTANCE_ALL )
{}

ObjectInstanceDataOStream::~ObjectInstanceDataOStream()
{}

void ObjectInstanceDataOStream::_sendPacket( ObjectInstancePacket& packet,
                                             const uint32_t compressor,
                                             const uint32_t nChunks,
                                             const void* const* chunks,
                                             const uint64_t* chunkSizes,
                                             const uint64_t size )
{
    const Object* object    = _cm->getObject();
    packet.nodeID           = _nodeID;
    packet.instanceID       = _instanceID;
    packet.masterInstanceID = object->getInstanceID();

    if( _instanceID == EQ_INSTANCE_NONE ) // send-on-register
        packet.command = CMD_NODE_INSTANCE;

#ifndef NDEBUG
    if( _nodeID == NodeID::ZERO )
    {
        EQASSERT( _instanceID == EQ_INSTANCE_NONE ||
                  _instanceID == EQ_INSTANCE_ALL );
    }
    else
    {
        EQASSERT( _instanceID < EQ_INSTANCE_MAX );
        EQASSERTINFO( _connections.size() == 1,
                      "Expected multicast to one group" );
    }
#endif

    sendPacket( packet, compressor, nChunks, chunks, chunkSizes, size );
}

void ObjectInstanceDataOStream::sendData( const uint32_t compressor,
                                          const uint32_t nChunks,
                                          const void* const* chunks,
                                          const uint64_t* chunkSizes,
                                          const uint64_t size )
{
    ObjectInstancePacket packet;
    _sendPacket( packet, compressor, nChunks, chunks, chunkSizes, size );
}

void ObjectInstanceDataOStream::sendFooter( const uint32_t compressor, 
                                            const uint32_t nChunks,
                                            const void* const* chunks,
                                            const uint64_t* chunkSizes,
                                            const uint64_t size )
{
    ObjectInstancePacket packet;
    packet.last = true;
    _sendPacket( packet, compressor, nChunks, chunks, chunkSizes, size );
    _sequence = 0;
}
}
