
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

#ifndef EQ_SERVERPACKETS_H
#define EQ_SERVERPACKETS_H

#include <eq/fabric/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
    typedef fabric::ServerPacket ServerPacket;
    
    struct ServerChooseConfigPacket : public ServerPacket
    {
        ServerChooseConfigPacket()
                : fill ( 0 )
            {
                command = fabric::CMD_SERVER_CHOOSE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
                rendererInfo[0] = '\0';
            }

        uint32_t requestID;
        uint32_t fill;
        EQ_ALIGN8( char rendererInfo[8] );
    };

    struct ServerChooseConfigReplyPacket : public ServerPacket
    {
        ServerChooseConfigReplyPacket( const ServerChooseConfigPacket*
                                       requestPacket )
            {
                command   = fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size      = sizeof( ServerChooseConfigReplyPacket );
                requestID = requestPacket->requestID;
                connectionData[0] = 0;
            }

        co::base::UUID configID;
        uint32_t requestID;
        EQ_ALIGN8( char connectionData[8] );
    };

    struct ServerReleaseConfigPacket : public ServerPacket
    {
        ServerReleaseConfigPacket()
            {
                command = fabric::CMD_SERVER_RELEASE_CONFIG;
                size    = sizeof( ServerReleaseConfigPacket );
            }

        co::base::UUID configID;
        uint32_t requestID;
    };

    struct ServerReleaseConfigReplyPacket : public ServerPacket
    {
        ServerReleaseConfigReplyPacket( const ServerReleaseConfigPacket*
                                        requestPacket )
            {
                command   = fabric::CMD_SERVER_RELEASE_CONFIG_REPLY;
                size      = sizeof( ServerReleaseConfigReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
    };

    struct ServerShutdownPacket : public ServerPacket
    {
        ServerShutdownPacket()
            {
                command = fabric::CMD_SERVER_SHUTDOWN;
                size    = sizeof( ServerShutdownPacket );
            }

        uint32_t requestID;
    };

    struct ServerShutdownReplyPacket : public ServerPacket
    {
        ServerShutdownReplyPacket( const ServerShutdownPacket* requestPacket )
            {
                command   = fabric::CMD_SERVER_SHUTDOWN_REPLY;
                size      = sizeof( ServerShutdownReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
        bool     result;
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (fabric::ServerPacket*)packet << " req " << packet->requestID
           << " renderer " << packet->rendererInfo;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                  const ServerChooseConfigReplyPacket* packet )
    {
        os << (fabric::ServerPacket*)packet << " req " << packet->requestID
           << " id " << packet->configID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
        const ServerReleaseConfigPacket* packet )
    {
        os << (fabric::ServerPacket*)packet << " config " << packet->configID;
        return os;
    }
}
/** @endcond */
#endif //EQ_SERVERPACKETS_H
