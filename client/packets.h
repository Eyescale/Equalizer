
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/net/packet.h>

#include "commands.h"

namespace eq
{
    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerChooseConfigPacketReply : public eqNet::NodePacket
    {
        ServerChooseConfigPacketReply()
            {
                command = CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size    = sizeof( ServerChooseConfigPacketReply );
            }
        uint requestID;
        uint configID;
    }
}

#endif // EQ_PACKETS_H

