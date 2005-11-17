
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDS_H
#define EQ_COMMANDS_H

#include <eq/net/commands.h>

namespace eq
{
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG       = eqNet::CMD_NODE_CUSTOM,
        CMD_SERVER_CHOOSE_CONFIG_REPLY,
        CMD_SERVER_RELEASE_CONFIG,
        CMD_SERVER_ALL
    };

    enum NodeCommand
    {
        CMD_NODE_INIT       = CMD_SERVER_ALL,
        CMD_NODE_ALL
    };

    enum ConfigCommand
    {
        CMD_CONFIG_INIT,
        CMD_CONFIG_INIT_REPLY,
        CMD_CONFIG_ALL
    };
};

#endif // EQ_COMMANDS_H

