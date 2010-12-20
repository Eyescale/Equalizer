
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

#ifndef EQFABRIC_PACKETS_H
#define EQFABRIC_PACKETS_H

#include <eq/fabric/commands.h>      // enum 
#include <eq/fabric/packetType.h>    // member

#include <co/packets.h> // 'base'
#include <co/objectVersion.h> // member

namespace eq
{
namespace fabric
{
/** @cond IGNORE */
    struct ServerPacket : public co::Packet
    {
        ServerPacket(){ type = PACKETTYPE_EQ_SERVER; }
    };

    typedef co::ObjectPacket ConfigPacket;

    typedef co::ObjectPacket PipePacket;
    typedef co::ObjectPacket WindowPacket;
    typedef co::ObjectPacket CanvasPacket;
    typedef co::ObjectPacket LayoutPacket;
   
/** @endcond */
}
}

#endif // EQFABRIC_PACKETS_H

