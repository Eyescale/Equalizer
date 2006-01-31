
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
        CMD_SESSION_GEN_IDS,
        CMD_SESSION_GEN_IDS_REPLY,
        CMD_SESSION_SET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER_REPLY,
        CMD_SESSION_GET_MOBJECT_MASTER,
        CMD_SESSION_GET_MOBJECT_MASTER_REPLY,
        CMD_SESSION_GET_MOBJECT,
        CMD_SESSION_INIT_MOBJECT,
        CMD_SESSION_INIT_MOBJECT_REPLY,
        CMD_SESSION_CUSTOM // must be last
    };

    enum UserCommand
    {
        CMD_USER_CUSTOM // must be last
    };
};

#endif // EQNET_COMMANDS_H

