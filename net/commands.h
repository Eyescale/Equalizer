
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMAND_PRIV_H
#define EQNET_COMMAND_PRIV_H

namespace eqNet
{
    namespace priv
    {
        enum Command
        {
            CMD_SESSION_CREATE,
            CMD_SESSION_NEW,
            CMD_NODE_NEW,
            CMD_NETWORK_NEW,
            CMD_NETWORK_ADD_NODE,
            CMD_ALL // must be last
        };
    }
};

#endif // EQNET_COMMAND_PRIV_H

