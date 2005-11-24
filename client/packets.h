
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/net/packets.h>

#include "commands.h"
#include "config.h"

namespace eq
{
    enum DataType
    {
        DATATYPE_EQ_CLIENT = eqNet::DATATYPE_CUSTOM,
        DATATYPE_EQ_SERVER,
        DATATYPE_EQ_CONFIG,
        DATATYPE_EQ_NODE
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

        uint requestID;
        uint appNameLength;
        uint renderClientLength;
        uint compoundModes;
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

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct NodePacket : public ConfigPacket
    {
        NodePacket( const uint configID, const uint nodeID ) 
                : ConfigPacket( configID )
            {
                datatype     = DATATYPE_EQ_NODE; 
                this->nodeID = nodeID;
            }
        
        uint nodeID;
    };


    struct NodeInitPacket : public NodePacket
    {
        NodeInitPacket( const uint configID, const uint nodeID )
                : NodePacket( configID, nodeID )
            {
                command = CMD_NODE_INIT;
                size    = sizeof( NodeInitPacket );
            }

        uint   requestID;
    };

    struct NodeInitReplyPacket : public NodePacket
    {
        NodeInitReplyPacket( NodeInitPacket* requestPacket )
                : NodePacket( requestPacket->configID, requestPacket->nodeID )
            {
                command   = CMD_NODE_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( NodeInitReplyPacket );
            }

        uint   requestID;
        bool   result;
    };


    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes << " appName " 
           << packet->appNameLength << " renderClient "
           << packet->renderClientLength;
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

