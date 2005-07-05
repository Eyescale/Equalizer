
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PACKET_PRIV_H
#define EQNET_PACKET_PRIV_H

#include <sys/param.h>

namespace eqNet
{
    namespace priv
    {
        enum Command;

        /**
         * Represents a packet.
         */
        struct Packet
        {
            uint    size;
            Command command;
        };

        struct ReqSessionCreatePacket : public Packet
        {
            ReqSessionCreatePacket() 
                { size = sizeof( ReqSessionCreatePacket ); }

            char localAddress[MAXHOSTNAMELEN+8];
        };            

        struct SessionCreatePacket : public Packet
        {
            SessionCreatePacket() { size = sizeof( SessionCreatePacket ); }
            
            uint sessionID;
            uint networkID;
            uint serverID;
            uint localID;
            char serverAddress[MAXHOSTNAMELEN+8];
        };
    }
}

#endif // EQNET_PACKET_PRIV_H

