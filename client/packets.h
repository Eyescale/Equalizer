
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/net/packets.h>

#include "commands.h"
#include "config.h"

namespace eq
{
    enum 
    {
        DATATYPE_EQ_CONFIG = eqNet::DATATYPE_CUSTOM
    };

    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerChooseConfigPacket : public eqNet::NodePacket
    {
        ServerChooseConfigPacket()
            {
                command = CMD_SERVER_CHOOSE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
            }

        uint requestID;
        uint appNameLength;
        uint renderClientLength;
        uint compoundModes;
    };

    struct ServerChooseConfigReplyPacket : public eqNet::NodePacket
    {
        ServerChooseConfigReplyPacket()
            {
                command = CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size    = sizeof( ServerChooseConfigReplyPacket );
            }
        uint requestID;
        uint configID;
    };

    struct ServerReleaseConfigPacket : public eqNet::NodePacket
    {
        ServerReleaseConfigPacket()
            {
                command = CMD_SERVER_RELEASE_CONFIG;
                size    = sizeof( ServerReleaseConfigPacket );
            }

        uint configID;
    };

    //------------------------------------------------------------
    // Config
    //------------------------------------------------------------
    struct ConfigPacket : public eqNet::Packet
    {
        ConfigPacket( Config* config )
            {
                datatype = DATATYPE_EQ_CONFIG; 
                configID = config->getID();
            }
        
        uint configID;
    };

    struct ConfigInitPacket : public ConfigPacket
    {
        ConfigInitPacket( Config* config ) : ConfigPacket( config )
            {
                command   = CMD_CONFIG_INIT;
                size      = sizeof( ConfigInitPacket );
            }
        uint requestID;
    };

    struct ConfigInitReplyPacket : public ConfigPacket
    {
        ConfigInitReplyPacket( Config* config ) : ConfigPacket( config )
            {
                command   = CMD_CONFIG_INIT_REPLY;
                size      = sizeof( ConfigInitReplyPacket );
            }
        uint requestID;
        bool result;
    };


    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (eqNet::NodePacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes << " appNameLength " 
           << packet->appNameLength;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerReleaseConfigPacket* packet )
    {
        os << (eqNet::NodePacket*)packet << " config " << packet->configID;
        return os;
    }
}

#endif // EQ_PACKETS_H

