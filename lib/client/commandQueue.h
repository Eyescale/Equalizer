
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
     * Augments an eq::net::CommandQueue to pump system-specific events where
     * required by the underlying window/operating system.
     */
    class CommandQueue : public eq::net::CommandQueue
    {
    public:
        CommandQueue();
        virtual ~CommandQueue();

        /** @sa eq::net::CommandQueue::push(). */
        virtual void push( eq::net::Command& packet );

        /** @sa eq::net::CommandQueue::pushFront(). */
        virtual void pushFront( eq::net::Command& packet );

        /** @sa eq::net::CommandQueue::wakeup(). */
        virtual void wakeup();

        /** @sa eq::net::CommandQueue::pop(). */
        virtual eq::net::Command* pop();

        /** @sa eq::net::CommandQueue::tryPop(). */
        virtual eq::net::Command* tryPop();

        void         setWindowSystem( const WindowSystem windowSystem );
        WindowSystem getWindowSystem() const { return _windowSystem; }
        
    private:
        MessagePump* _messagePump;
        WindowSystem _windowSystem;
    };
}

#endif //EQ_COMMANDQUEUE_H
