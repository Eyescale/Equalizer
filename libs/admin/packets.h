
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

#ifndef EQADMIN_PACKETS_H
#define EQADMIN_PACKETS_H

#include <eq/fabric/commands.h>      // enum 
#include <eq/fabric/packetType.h>    // member

#include <eq/net/packets.h>

namespace eq
{
namespace admin
{
/** @cond IGNORE */
    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerPacket : public net::Packet
    {
        ServerPacket(){ type = fabric::PACKETTYPE_EQ_SERVER; }
    };

    struct ServerMapPacket : public ServerPacket
    {
        ServerMapPacket()
            {
                command = fabric::CMD_SERVER_MAP;
                size    = sizeof( ServerMapPacket );
            }

        uint32_t requestID;
    };

    struct ServerMapReplyPacket : public ServerPacket
    {
        ServerMapReplyPacket( const ServerMapPacket* request )
                : requestID( request->requestID )
            {
                command = fabric::CMD_SERVER_MAP_REPLY;
                size    = sizeof( ServerMapReplyPacket );
            }

        const uint32_t requestID;
    };

    struct ServerUnmapPacket : public ServerPacket
    {
        ServerUnmapPacket()
            {
                command = fabric::CMD_SERVER_UNMAP;
                size    = sizeof( ServerUnmapPacket );
            }

        uint32_t requestID;
    };

    struct ServerUnmapReplyPacket : public ServerPacket
    {
        ServerUnmapReplyPacket( const ServerUnmapPacket* request )
                : requestID( request->requestID )
            {
                command = fabric::CMD_SERVER_UNMAP_REPLY;
                size    = sizeof( ServerUnmapReplyPacket );
            }

        const uint32_t requestID;
    };

/** @endcond */
}
}

#endif // EQ_PACKETS_H

