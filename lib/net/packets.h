
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

#ifndef EQNET_PACKETS_H
#define EQNET_PACKETS_H

#include <eq/net/commands.h> // used for CMD_ enums
#include <eq/net/types.h>
#include <eq/net/version.h>  // enum

#include <eq/base/idPool.h> // for EQ_ID_*

namespace eq
{
namespace net
{
    enum PacketType
    {
        PACKETTYPE_EQNET_NODE,
        PACKETTYPE_EQNET_SESSION,
        PACKETTYPE_EQNET_OBJECT,
        PACKETTYPE_EQNET_CUSTOM = 1<<7
    };

    /** A packet send over the network. */
    struct Packet
    {
        uint64_t size; //!< Total size, set by the most specific sub-struct
        uint32_t type; //!< The packet (receiver) type
        uint32_t command; //!< The specific command name @sa commands.h
        
#if 0
        union
        {
            Foo foo;
            uint64_t paddingPacket; // pad to multiple-of-8
        };
#endif

        static size_t minSize;
        bool exceedsMinSize() const { return (size > minSize); }
    };

    // String transmission: the packets define a 8-char string at the end of the
    // packet. When the packet is sent using Node::send( Packet&, string& ), the
    // whole string is appended to the packet, so that the receiver has to do
    // nothing special to receive and use the full packet.

    /** Packet sent to and handled by an eq::net::Node. */
    struct NodePacket: public Packet
    {
        NodePacket(){ type = PACKETTYPE_EQNET_NODE; }
    };

    /** Packet sent to and handled by an eq::net::Session. */
    struct SessionPacket : public NodePacket
    {
        SessionPacket() { type = PACKETTYPE_EQNET_SESSION; }
        base::UUID sessionID;
    };

    /** Packet sent to and handled by an eq::net::Object. */
    struct ObjectPacket : public SessionPacket
    {
        ObjectPacket()
            {
                type   = PACKETTYPE_EQNET_OBJECT; 
                instanceID = EQ_ID_ANY;
            }
        uint32_t objectID;
        uint32_t instanceID;
        // pad to multiple-of-eight
    };

    //------------------------------------------------------------
    // ostream operators
    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const Packet* packet )
    {
        os << "packet dt " << packet->type << " cmd "
           << packet->command;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const SessionPacket* packet )
    {
        os << (NodePacket*)packet << " session " << packet->sessionID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectPacket* packet )
    {
        os << (SessionPacket*)packet << " object " << packet->objectID
           << "." << packet->instanceID;
        return os;
    }
}
}

#endif // EQNET_PACKETS_H

