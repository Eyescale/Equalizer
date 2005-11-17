
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
        DATATYPE_EQ_NODE   = eqNet::DATATYPE_EQNET_NODE,
        DATATYPE_EQ_SERVER = eqNet::DATATYPE_CUSTOM,
        DATATYPE_EQ_CONFIG
    };

    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerPacket : public eqNet::Packet
    {
        ServerPacket(){ datatype = DATATYPE_EQ_SERVER; }
    };

    struct ServerChooseConfigPacket : public ServerPacket
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

    struct ServerChooseConfigReplyPacket : public ServerPacket
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

    struct ServerReleaseConfigPacket : public ServerPacket
    {
        ServerReleaseConfigPacket()
            {
                command = CMD_SERVER_RELEASE_CONFIG;
                size    = sizeof( ServerReleaseConfigPacket );
            }

        uint configID;
    };

    struct NodeInitPacket : public eqNet::NodePacket
    {
        NodeInitPacket()
            {
                command = CMD_NODE_INIT;
                size    = sizeof( NodeInitPacket );
            }

        uint64 callbackName;
        uint   requestID;
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
        os << (ServerPacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes << " appName " 
           << (void*)packet->appNameString << " renderClient "
           << (void*)packet->renderClientString;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerReleaseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " config " << packet->configID;
        return os;
    }
}

#endif // EQ_PACKETS_H

