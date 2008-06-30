
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDQUEUE_H
#define EQ_COMMANDQUEUE_H

#include <eq/net/commandQueue.h>    // base class
#include <eq/client/messagePump.h>  // member
#include <eq/client/windowSystem.h> // enum

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
        virtual ~CommandQueue();

        /** @sa eqNet::CommandQueue::push(). */
        virtual void push( eqNet::Command& packet );

        /** @sa eqNet::CommandQueue::pushFront(). */
        virtual void pushFront( eqNet::Command& packet );

        /** @sa eqNet::CommandQueue::wakeup(). */
        virtual void wakeup();

        /** @sa eqNet::CommandQueue::pop(). */
        virtual eqNet::Command* pop();

        /** @sa eqNet::CommandQueue::tryPop(). */
        virtual eqNet::Command* tryPop();

        void         setWindowSystem( const WindowSystem windowSystem );
        WindowSystem getWindowSystem() const { return _windowSystem; }
        
    private:
        MessagePump* _messagePump;
        WindowSystem _windowSystem;
    };
}

#endif //EQ_COMMANDQUEUE_H
