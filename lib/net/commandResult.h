
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDRESULT_H
#define EQNET_COMMANDRESULT_H

namespace eqNet
{
    enum CommandResult
    {
        COMMAND_HANDLED,     //*< The command was handled
        COMMAND_DISCARD,     //*< Discard command, used by Objects
        COMMAND_REDISPATCH,  //*< Reschedule command to be handled later
        COMMAND_PUSH,        //*< Push to another thread
        COMMAND_PUSH_FRONT,  //*< Push to another thread with high priority
        COMMAND_ERROR        //*< An unrecoverable error occured
    };
}

#endif // EQNET_COMMANDRESULT_H
