
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef EQNET_OBJECTPACKETS_H
#define EQNET_OBJECTPACKETS_H

#include <co/packets.h> // base structs

/** @cond IGNORE */
namespace co
{
    struct ObjectCommitPacket : public ObjectPacket
    {
        ObjectCommitPacket()
            {
                command        = CMD_OBJECT_COMMIT;
                size           = sizeof( ObjectCommitPacket ); 
            }
        
        uint32_t requestID;
    };

    struct ObjectDataPacket : public ObjectPacket
    {
        ObjectDataPacket() : dataSize( 0 )
                           , sequence( 0 )
                           , compressorName( 0 )
                           , nChunks( 0 )
                           , last( false ) {}

        uint128_t version;
        uint64_t dataSize;
        uint32_t sequence;
        uint32_t compressorName;
        uint32_t nChunks;
        EQ_ALIGN8( uint64_t last ); // pad and align to multiple-of-eight
    };

    struct ObjectInstancePacket : public ObjectDataPacket
    {
        ObjectInstancePacket()
            {
                // Always goes through session which caches and forwards to obj
                type    = PACKETTYPE_CO_NODE;
                command = CMD_NODE_OBJECT_INSTANCE;
                size    = sizeof( ObjectInstancePacket );
                data[0] = 0;
            }

        NodeID nodeID;
        uint32_t masterInstanceID;
        EQ_ALIGN8( uint8_t data[8] );
    };

    struct ObjectDeltaPacket : public ObjectDataPacket
    {
        ObjectDeltaPacket()
            {
                command    = CMD_OBJECT_DELTA;
                size       = sizeof( ObjectDeltaPacket ); 
                instanceID = EQ_INSTANCE_NONE; // multicasted
            }
        EQ_ALIGN8( uint8_t data[8] );
    };

    struct ObjectSlaveDeltaPacket : public ObjectDataPacket
    {
        ObjectSlaveDeltaPacket()
            {
                command    = CMD_OBJECT_SLAVE_DELTA;
                size       = sizeof( ObjectSlaveDeltaPacket ); 
            }

        co::base::UUID commit;
        EQ_ALIGN8( uint8_t data[8] );
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectDataPacket* packet )
    {
        os << (ObjectPacket*)packet << " v" << packet->version
           << " size " << packet->dataSize << " seq " << packet->sequence;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectInstancePacket* packet )
    {
        os << (ObjectDataPacket*)packet << " master " 
           << packet->masterInstanceID << " node " << packet->nodeID;
        return os;
    }
}
/** @endcond */

#endif // EQNET_OBJECTPACKETS_H

