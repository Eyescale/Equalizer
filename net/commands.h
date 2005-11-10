
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDS_H
#define EQNET_COMMANDS_H

namespace eqNet
{
    enum NodeCommand
    {
        CMD_NODE_STOP,
        CMD_NODE_MESSAGE,
        CMD_NODE_MAP_SESSION,
        CMD_NODE_MAP_SESSION_REPLY,
        CMD_NODE_SESSION,
        CMD_NODE_CONNECT,
        CMD_NODE_CUSTOM // must be last
    };

    enum SessionCommand
    {
        CMD_SESSION_CREATE_USER,
        CMD_SESSION_NEW_USER,
        CMD_SESSION_CUSTOM // must be last
    };

    enum UserCommand
    {
        CMD_USER_CUSTOM // must be last
    };
};

#endif // EQNET_COMMANDS_H

