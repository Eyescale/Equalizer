
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

#ifndef CO_PACKETS_H
#define CO_PACKETS_H

#include <co/commands.h> // used for CMD_ enums
#include <co/types.h>
#include <co/version.h>  // enum

namespace co
{
    enum PacketType
    {
        PACKETTYPE_CO_NODE,
        PACKETTYPE_CO_OBJECT,
        PACKETTYPE_CO_CUSTOM = 1<<7
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

    /** Packet sent to and handled by an co::Node. */
    struct NodePacket: public Packet
    {
        NodePacket(){ type = PACKETTYPE_CO_NODE; }
    };

    /** Packet sent to and handled by an co::Object. */
    struct ObjectPacket : public NodePacket
    {
        ObjectPacket()
            {
                type = PACKETTYPE_CO_OBJECT; 
                instanceID = EQ_INSTANCE_ALL;
            }
        co::base::UUID objectID;
        uint32_t instanceID;
        uint32_t pad; // pad to multiple-of-eight
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
                                       const NodePacket* packet )
    {
        os << (Packet*)packet;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectPacket* packet )
    {
        os << (NodePacket*)packet << " object " << packet->objectID
           << "." << packet->instanceID;
        return os;
    }
}

#endif // CO_PACKETS_H

