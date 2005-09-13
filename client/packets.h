
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
    struct ServerChooseConfigPacket : public eqNet::NodePacket
    {
        ServerChooseConfigPacket()
            {
                command = CMD_SERVER_CHOOSE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
            }

        uint requestID;
        uint appNameLength;
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

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (eqNet::NodePacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes << " appNameLength " 
           << packet->appNameLength;
        return os;
    }
}

#endif // EQ_PACKETS_H

