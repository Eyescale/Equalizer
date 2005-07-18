
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMAND_PRIV_H
#define EQNET_COMMAND_PRIV_H

namespace eqNet
{
    namespace priv
    {
        enum ServerCommand
        {
            CMD_SESSION_CREATE,
            CMD_SESSION_CREATED,
            CMD_SESSION_NEW,
            CMD_SERVER_ALL // must be last
        };

        enum SessionCommand
        {
            CMD_NODE_NEW,
            CMD_NETWORK_NEW,
            CMD_SESSION_ALL // must be last
        };

        enum NetworkCommand
        {
            CMD_NETWORK_ADD_NODE,
            CMD_NETWORK_ALL // must be last
        };

        enum NodeCommand
        {
            CMD_NODE_ALL // must be last
        };
    }
};

#endif // EQNET_COMMAND_PRIV_H

