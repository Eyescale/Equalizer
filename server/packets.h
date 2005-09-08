
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PACKETS_H
#define EQS_PACKETS_H

#include <eq/net/packet.h>

#include "commands.h"

namespace eqs
{
    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct ServerChooseConfigPacket : public eqNet::NodePacket
    {
        ServerChooseConfigPacket()
            {
                command = CMD_SERVER_CHOOSE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
            }
    };
}

#endif // EQS_PACKETS_H
