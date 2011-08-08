
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com> 
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

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/fabric/commands.h>      // enum 
#include <eq/fabric/packetType.h>    // member
#include <eq/fabric/renderContext.h> // member

namespace eq
{
/** @cond IGNORE */
    typedef co::ObjectPacket  ConfigPacket;
    typedef co::ObjectPacket  ChannelPacket;
    typedef co::ObjectPacket  WindowPacket;
    typedef co::ObjectPacket  PipePacket;
    typedef co::ObjectPacket  NodePacket;

    struct ClientPacket : public co::Packet
    {
        ClientPacket(){ type = fabric::PACKETTYPE_EQ_CLIENT; }
    };

    struct ChannelTaskPacket : public ChannelPacket
    {
        RenderContext context;
    };
    
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelTaskPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " " << packet->context;
        return os;
    }
/** @endcond */
}

#endif // EQ_PACKETS_H

