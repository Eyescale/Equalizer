
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMAND_PRIV_H
#define EQNET_COMMAND_PRIV_H

namespace eqNet
{
    enum NodeCommand
    {
        CMD_NODE_MESSAGE,
        CMD_SESSION_CREATE,
        CMD_SESSION_CREATED,
        CMD_SESSION_NEW,
        CMD_NODE_CUSTOM // must be last
    };

    enum SessionCommand
    {
        CMD_SESSION_NEW_USER,
        CMD_SESSION_ALL // must be last
    };

    enum UserCommand
    {
        CMD_USER_ALL // must be last
    };
};

#endif // EQNET_COMMAND_PRIV_H

