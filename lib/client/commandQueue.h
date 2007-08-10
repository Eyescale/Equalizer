
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDQUEUE_H
#define EQ_COMMANDQUEUE_H

#include <eq/net/commandQueue.h>

#include <eq/base/messagePump.h>
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
		virtual ~CommandQueue(){}

        /** @sa eqNet::CommandQueue::push(). */
        virtual void push( eqNet::Command& packet );

        /** @sa eqNet::CommandQueue::pushFront(). */
        virtual void pushFront( eqNet::Command& packet );
        
        /** @sa eqNet::CommandQueue::pop(). */
        virtual eqNet::Command* pop();

    private:
        eqBase::MessagePump _messagePump;
    };
}

#endif //EQ_COMMANDQUEUE_H
