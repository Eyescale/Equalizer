
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDS_H
#define EQ_COMMANDS_H

namespace eqNet
{
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG_REPLY = eqNet::CMD_NODE_CUSTOM,
        CMD_SERVER_ALL
    };
};

#endif // EQ_COMMANDS_H

