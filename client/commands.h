
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDS_H
#define EQ_COMMANDS_H

namespace eq
{
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG       = eqNet::CMD_NODE_CUSTOM,
        CMD_SERVER_CHOOSE_CONFIG_REPLY,
        CMD_SERVER_ALL
    };
};

#endif // EQ_COMMANDS_H

