
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "object.h"
#include "packets.h"
#include "session.h"

namespace eq
{
namespace net
{
ObjectInstanceDataOStream::ObjectInstanceDataOStream( const Object* object )
        : ObjectDataOStream( object )
        , _instanceID( EQ_ID_ANY )
{}

ObjectInstanceDataOStream::~ObjectInstanceDataOStream()
{}

void ObjectInstanceDataOStream::_sendPacket( ObjectInstancePacket& packet,
                                             const void* const* buffers,
                                             const uint64_t* sizes,
                                             const uint64_t sizeUncompressed )
{
    packet.version    = _version;
    packet.sequence   = _sequence++;
    packet.dataSize   = sizeUncompressed;
    packet.sessionID  = _object->getSession()->getID();
    packet.objectID   = _object->getID();
    packet.instanceID = _instanceID;
    packet.masterInstanceID = _object->getInstanceID();

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
    
    if( packet.compressorName != EQ_COMPRESSOR_NONE )
    {
        uint64_t dataSendSize  = 0;
        for( uint32_t i = 0; i < packet.nChunks; i++ )
        {
            dataSendSize  += sizes[i];
        }

        Connection::send( _connections, packet, buffers, 
                           sizes, packet.nChunks, dataSendSize, true );
        return;
    }

    Connection::send( _connections, packet, buffers[0], sizeUncompressed, true );
}

void ObjectInstanceDataOStream::sendBuffer( const uint32_t name, 
                                            const uint32_t nChunks,
                                            const void* const* buffers,
                                            const uint64_t* sizes,
                                            const uint64_t sizeUncompressed )
{
    ObjectInstancePacket packet;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    _sendPacket( packet, buffers, sizes, sizeUncompressed );
}

void ObjectInstanceDataOStream::sendFooter( const uint32_t name, 
                                            const uint32_t nChunks,
                                            const void* const* buffers,
                                            const uint64_t* sizes,
                                            const uint64_t sizeUncompressed )
{
    ObjectInstancePacket packet;
    packet.compressorName = name;
    packet.nChunks        = nChunks;
    packet.last = true;
    _sendPacket( packet, buffers, sizes, sizeUncompressed );
    _sequence = 0;
}
}
}
