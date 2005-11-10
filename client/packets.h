
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/net/packets.h>

#include "commands.h"

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

        uint   requestID;
        uint64 appNameString;
        uint64 renderClientString;
        uint   compoundModes;
    };

    struct ServerChooseConfigReplyPacket : public eqNet::NodePacket
    {
        ServerChooseConfigReplyPacket( const ServerChooseConfigPacket*
                                       requestPacket )
            {
                command   = CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size      = sizeof( ServerChooseConfigReplyPacket );
                requestID = requestPacket->requestID;
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
        ConfigPacket( const uint configID )
            {
                datatype = DATATYPE_EQ_CONFIG; 
                this->configID = configID;
            }
        
        uint configID;
    };

    struct ConfigInitPacket : public ConfigPacket
    {
        ConfigInitPacket( const uint configID ) : ConfigPacket( configID )
            {
                command   = CMD_CONFIG_INIT;
                size      = sizeof( ConfigInitPacket );
            }
        uint requestID;
    };

    struct ConfigInitReplyPacket : public ConfigPacket
    {
        ConfigInitReplyPacket( const ConfigInitPacket* requestPacket )
                : ConfigPacket( requestPacket->configID )
            {
                command   = CMD_CONFIG_INIT_REPLY;
                size      = sizeof( ConfigInitReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint requestID;
        bool result;
    };


    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (eqNet::NodePacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes << " appName " 
           << (void*)packet->appNameString << " renderClient "
           << (void*)packet->renderClientString;
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

