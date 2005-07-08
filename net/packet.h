
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKET_PRIV_H
#define EQNET_PACKET_PRIV_H

#include "commands.h"
#include <sys/param.h>

namespace eqNet
{
    enum NetworkProtocol;

    namespace priv
    {
        /**
         * Represents a packet.
         */
        struct Packet
        {
            uint64  size;
            uint64  id;
            Command command;
        };

        struct ReqSessionCreatePacket : public Packet
        {
            ReqSessionCreatePacket()
                { 
                    command = CMD_SESSION_CREATE;
                    id = INVALID_ID;
                    size = sizeof( ReqSessionCreatePacket ); 
                }

            char localAddress[MAXHOSTNAMELEN+8];
        };            

        struct SessionCreatePacket : public Packet
        {
            SessionCreatePacket() 
                {
                    command = CMD_SESSION_CREATE;
                    size = sizeof( SessionCreatePacket ); 
                }
            
            uint localNodeID;
            uint serverID;
            uint networkID;
        };

        struct SessionNewPacket : public Packet
        {
            SessionNewPacket() 
                {
                    command = CMD_SESSION_NEW;
                    size = sizeof( SessionNewPacket ); 
                }

            Session::State state;
        };

        struct NodeNewPacket : public Packet
        {
            NodeNewPacket() 
                {
                    command = CMD_NODE_NEW;
                    size = sizeof( NodeNewPacket ); 
                }

            uint sessionID;
            Node::State state;
        };

        struct NetworkNewPacket : public Packet
        {
            NetworkNewPacket() 
                {
                    command = CMD_NETWORK_NEW;
                    size = sizeof( NetworkNewPacket ); 
                }

            uint sessionID;
            Network::State state;
            NetworkProtocol protocol;
        };

        struct NetworkAddNodePacket : public Packet
        {
            NetworkAddNodePacket() 
                {
                    command = CMD_NETWORK_ADD_NODE;
                    size = sizeof( NetworkAddNodePacket ); 
                }

            uint nodeID;
            ConnectionDescription connectionDescription;
        };
    }
}

#endif // EQNET_PACKET_PRIV_H

