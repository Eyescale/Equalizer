
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDRESULT_H
#define EQNET_COMMANDRESULT_H

namespace eqNet
{
    enum CommandResult
    {
        COMMAND_HANDLED,     //*< The command was handled
        COMMAND_DISCARD,     //*< Discard command, used by Objects
        COMMAND_ERROR        //*< An unrecoverable error occured
    };
}

#endif // EQNET_COMMANDRESULT_H
