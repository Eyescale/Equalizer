
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

#ifndef EQFABRIC_SERVERPACKETS_H
#define EQFABRIC_SERVERPACKETS_H

#include <eq/fabric/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
namespace fabric
{
    struct ServerCreateConfigPacket : public ServerPacket
    {
        ServerCreateConfigPacket()
                : requestID( EQ_UNDEFINED_UINT32 )
            {
                command   = CMD_SERVER_CREATE_CONFIG;
                size      = sizeof( ServerCreateConfigPacket );
            }

        co::ObjectVersion configVersion;
        uint32_t requestID;
    };

    struct ServerDestroyConfigPacket : public ServerPacket
    {
        ServerDestroyConfigPacket()
                : requestID ( EQ_UNDEFINED_UINT32 )
            {
                command = CMD_SERVER_DESTROY_CONFIG;
                size    = sizeof( ServerDestroyConfigPacket );
            }

        co::base::UUID configID;
        uint32_t requestID;
    };

    struct ServerDestroyConfigReplyPacket : public ServerPacket
    {
        ServerDestroyConfigReplyPacket(
            const ServerDestroyConfigPacket* request )
            {
                command       = CMD_SERVER_DESTROY_CONFIG_REPLY;
                size          = sizeof( ServerDestroyConfigReplyPacket );
                requestID     = request->requestID;
            }

        uint32_t requestID;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerCreateConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " config version "
           << packet->configVersion << " request " << packet->requestID;
        return os;
    }
}
}
/** @endcond */
#endif //EQFABRIC_SERVERPACKETS_H
