
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKET_PRIV_H
#define EQNET_PACKET_PRIV_H

#include "commands.h"
#include "connectionDescription.h"
#include "global.h"

#include "networkPriv.h"

#include <sys/param.h>

namespace eqNet
{
    enum NetworkProtocol;

    namespace priv
    {
        enum 
        {
            DATATYPE_SERVER,
            DATATYPE_SESSION,
            DATATYPE_NETWORK,
            DATATYPE_NODE,
            DATATYPE_USER = 1<<16
        };

        /**
         * Represents a packet.
         */
        struct Packet
        {
            uint64  size;
            uint64  id;
            uint    datatype;
            uint    command;
        };

        struct ReqSessionCreatePacket : public Packet
        {
            ReqSessionCreatePacket()
                { 
                    command  = CMD_SESSION_CREATE;
                    id       = INVALID_ID;
                    datatype = DATATYPE_SERVER;
                    size     = sizeof( ReqSessionCreatePacket ); 
                }

            char requestorAddress[MAXHOSTNAMELEN+1];
        };            

        struct SessionCreatePacket : public Packet
        {
            SessionCreatePacket() 
                {
                    command  = CMD_SESSION_CREATE;
                    datatype = DATATYPE_SERVER;
                    size     = sizeof( SessionCreatePacket ); 
                }
            
            uint localNodeID;
            uint serverID;
            uint networkID;
        };

        struct SessionNewPacket : public Packet
        {
            SessionNewPacket() 
                {
                    command  = CMD_SESSION_NEW;
                    datatype = DATATYPE_SERVER;
                    size     = sizeof( SessionNewPacket ); 
                }

            uint serverID;
        };

        struct NodeNewPacket : public Packet
        {
            NodeNewPacket() 
                {
                    command  = CMD_NODE_NEW;
                    datatype = DATATYPE_SESSION;
                    size     = sizeof( NodeNewPacket ); 
                }

            uint sessionID;
        };

        struct NetworkNewPacket : public Packet
        {
            NetworkNewPacket() 
                {
                    command  = CMD_NETWORK_NEW;
                    datatype = DATATYPE_SESSION;
                    size     = sizeof( NetworkNewPacket ); 
                }

            uint sessionID;
            Network::State state;
            NetworkProtocol protocol;
        };

        struct NetworkAddNodePacket : public Packet
        {
            NetworkAddNodePacket() 
                {
                    command  = CMD_NETWORK_ADD_NODE;
                    datatype = DATATYPE_NETWORK;
                    size     = sizeof( NetworkAddNodePacket ); 
                }

            uint nodeID;
            ConnectionDescription connectionDescription;
            uint nodeState;
        };

        inline std::ostream& operator << ( std::ostream& os, Packet* packet )
        {
            os << "Packet cmd " << packet->command << " id " << packet->id
               << std::endl;
            return os;
        }
    }
}

#endif // EQNET_PACKET_PRIV_H

