
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_PACKETTYPE_H
#define EQFABRIC_PACKETTYPE_H

#include <co/packets.h> // 'base' enum

namespace eq
{
namespace fabric
{
    /** Packet types to identify the target of a packet. */
    enum PacketType
    {
        PACKETTYPE_EQ_CLIENT = co::PACKETTYPE_CO_CUSTOM, // 128
        PACKETTYPE_EQ_SERVER,
        PACKETTYPE_EQ_CUSTOM = 1<<8 // 256
    };
}
}
#endif // EQFABRIC_PACKETTYPE_H
