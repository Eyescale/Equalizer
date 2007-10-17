/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 All rights reserved. */

#include "commandQueue.h"

using namespace std;

namespace eq
{
CommandQueue::CommandQueue()
{
}

void CommandQueue::push(eqNet::Command& inCommand)
{
    eqNet::CommandQueue::push(inCommand);
#if defined (WIN32) || defined (Darwin)
    _messagePump.postWakeup();
#endif
}

void CommandQueue::pushFront(eqNet::Command& inCommand)
{
    eqNet::CommandQueue::pushFront(inCommand);
#if defined (WIN32) || defined (Darwin)
    _messagePump.postWakeup();
#endif
}

eqNet::Command* CommandQueue::pop()
{
#if defined (WIN32) || defined (Darwin)
    while( true )
    {
        // Poll for a command
        eqNet::Command* command = tryPop();
        if( command )
            return command;

        _messagePump.dispatchOne(); // blocking - push will send 'fake' event
    }

#else
    return eqNet::CommandQueue::pop();
#endif
}

eqNet::Command* CommandQueue::tryPop()
{
#if defined (WIN32) || defined (Darwin)
    _messagePump.dispatchAll(); // non-blocking
#endif
    return eqNet::CommandQueue::tryPop();
}
}
