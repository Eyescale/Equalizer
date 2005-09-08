
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMMANDS_H
#define EQS_COMMANDS_H

namespace eqs
{
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG = eqNet::CMD_NODE_CUSTOM,
        CMD_SERVER_ALL
    };
};

#endif // EQS_COMMANDS_H

