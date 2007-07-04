
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDQUEUE_H
#define EQ_COMMANDQUEUE_H

#include <eq/net/commandQueue.h>

#include <eq/client/windowSystem.h>

namespace eq
{
    /**
     * Augments an eqNet::CommandQueue to pump system-specific events where
     * required by the underlying window/operating system.
     */
class CommandQueue : public eqNet::CommandQueue
    {
    public:
        CommandQueue();

        /** @sa eqNet::CommandQueue::push(). */
        virtual void push( eqNet::Command& packet );

        /** @sa eqNet::CommandQueue::pushFront(). */
        virtual void pushFront( eqNet::Command& packet );
        
        /** @sa eqNet::CommandQueue::pop(). */
        virtual eqNet::Command* pop();

    private:

        union
        {
#ifdef WIN32
            /** Thread ID of the receiver. */
            DWORD _win32ThreadID;
#endif
#ifdef Darwin
            EventQueueRef _receiverQueue;
#endif
            char _fillDummy[32];
        };

        /** Send a system-dependent message to wake up the receiver thread. */
        void _postWakeupEvent();

        /** Get and dispatch all pending system events, non-blocking */
        void _pumpEvents();

        /** Get and dispatch one pending system events, blocking */
        void _pumpEvent();
    };
}

#endif //EQ_COMMANDQUEUE_H
