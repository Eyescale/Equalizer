
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

        uint requestID;
        uint appNameLength;
        uint compoundModes;
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

#endif // EQS_PACKETS_H
