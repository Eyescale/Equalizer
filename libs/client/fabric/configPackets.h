
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

#ifndef EQFABRIC_CONFIGPACKETS_H
#define EQFABRIC_CONFIGPACKETS_H

#include <eq/fabric/packets.h> // base structs
#include <eq/fabric/serverPackets.h> // used inline

/** @cond IGNORE */
namespace eq
{
namespace fabric
{
    struct ConfigCreateReplyPacket : public ConfigPacket
    {
        ConfigCreateReplyPacket(const ServerCreateConfigPacket* request)
        {
            command   = CMD_CONFIG_CREATE_REPLY;
            size      = sizeof( ConfigCreateReplyPacket );
            objectID  = request->configVersion.identifier;
            requestID = request->requestID;
        }

        uint32_t requestID;
    };

    struct ConfigNewLayoutPacket : public ConfigPacket
    {
        ConfigNewLayoutPacket()
            {
                command = fabric::CMD_CONFIG_NEW_LAYOUT;
                size    = sizeof( ConfigNewLayoutPacket );
            }

        uint32_t requestID;
    };

    struct ConfigNewCanvasPacket : public ConfigPacket
    {
        ConfigNewCanvasPacket()
            {
                command = fabric::CMD_CONFIG_NEW_CANVAS;
                size    = sizeof( ConfigNewCanvasPacket );
            }

        uint32_t requestID;
    };

    struct ConfigNewObserverPacket : public ConfigPacket
    {
        ConfigNewObserverPacket()
            {
                command = fabric::CMD_CONFIG_NEW_OBSERVER;
                size    = sizeof( ConfigNewObserverPacket );
            }

        uint32_t requestID;
    };

    struct ConfigNewEntityReplyPacket : public ConfigPacket
    {
        ConfigNewEntityReplyPacket( const ConfigNewLayoutPacket* request )
                : requestID( request->requestID ) { init(); }
        ConfigNewEntityReplyPacket( const ConfigNewCanvasPacket* request )
                : requestID( request->requestID ) { init(); }
        ConfigNewEntityReplyPacket( const ConfigNewObserverPacket* request )
                : requestID( request->requestID ) { init(); }
        void init()
            {
                command = fabric::CMD_CONFIG_NEW_ENTITY_REPLY;
                size    = sizeof( ConfigNewEntityReplyPacket );
            }

        const uint32_t requestID;
        co::base::UUID     entityID;
    };
}
}
/** @endcond */
#endif //EQFABRIC_CONFIGPACKETS_H
