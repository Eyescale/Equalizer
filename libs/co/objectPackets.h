
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <co/objectVersion.h> // base structs

#ifdef _WIN32
#  include <malloc.h>
#  define bzero( ptr, size ) memset( ptr, 0, size );
#else
#  include <alloca.h>
#endif

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
        uint32_t incarnation;
    };

    struct ObjectObsoletePacket : public ObjectPacket
    {
        ObjectObsoletePacket( const uint32_t c ) : count( c )
            {
                command        = CMD_OBJECT_OBSOLETE;
                size           = sizeof( ObjectObsoletePacket ); 
            }
        
        const uint32_t count;
    };

    struct ObjectPushPacket : public ObjectPacket
    {
        ObjectPushPacket( const uint32_t instanceID_, const uint32_t requestID_,
                          const uint128_t& groupID_, const uint128_t& typeID_,
                          const Nodes& nodes_ )
                : groupID( groupID_ )
                , typeID( typeID_ )
                , requestID( requestID_ )
                , nodes( &nodes_ )
            {
                command = CMD_OBJECT_PUSH;
                instanceID = instanceID_;
                size = sizeof( ObjectPushPacket ); 
            }
        
        const uint128_t groupID;
        const uint128_t typeID;
        const uint32_t requestID;
        const Nodes* nodes;
    };

    struct ObjectDataPacket : public ObjectPacket
    {
        ObjectDataPacket()
                : version( VERSION_NONE )
                , dataSize( 0 )
                , sequence( 0 )
                , compressorName( 0 )
                , nChunks( 0 )
                , fill( 0 )
                , last( false )
            {}

        uint128_t version;
        uint64_t dataSize;
        uint32_t sequence;
        uint32_t compressorName;
        uint32_t nChunks;
        uint32_t fill;
        EQ_ALIGN8( uint64_t last ); // pad and align to multiple-of-eight
    };

    struct ObjectInstancePacket : public ObjectDataPacket
    {
        ObjectInstancePacket( const NodeID& id, const uint32_t iID )
                : nodeID( id )
                , masterInstanceID( iID )
                , fill( 0 )
            {
                // Always go through session which caches and forwards to object
                type    = PACKETTYPE_CO_NODE;
                command = CMD_NODE_OBJECT_INSTANCE;
                size    = sizeof( ObjectInstancePacket );
                bzero( data, sizeof( data ));
            }

        const NodeID nodeID;
        const uint32_t masterInstanceID;
        const uint32_t fill;
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

        base::UUID commit;
        EQ_ALIGN8( uint8_t data[8] );
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectDataPacket* packet )
    {
        os << (ObjectPacket*)packet << " v" << packet->version
           << " size " << packet->dataSize << " seq " << packet->sequence
           << " last " << static_cast< bool >( packet->last );
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

