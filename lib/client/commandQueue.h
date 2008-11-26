
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
     * Augments an net::CommandQueue to pump system-specific events where
     * required by the underlying window/operating system.
     */
    class CommandQueue : public net::CommandQueue
    {
    public:
        CommandQueue();
        virtual ~CommandQueue();

        /** @sa net::CommandQueue::push(). */
        virtual void push( net::Command& packet );

        /** @sa net::CommandQueue::pushFront(). */
        virtual void pushFront( net::Command& packet );

        /** @sa net::CommandQueue::wakeup(). */
        virtual void wakeup();

        /** @sa net::CommandQueue::pop(). */
        virtual net::Command* pop();

        /** @sa net::CommandQueue::tryPop(). */
        virtual net::Command* tryPop();

        /** @sa net::CommandQueue::flush(). */
        virtual void flush();

        void         setWindowSystem( const WindowSystem windowSystem );
        WindowSystem getWindowSystem() const { return _windowSystem; }
        
    private:
        MessagePump* _messagePump;
        WindowSystem _windowSystem;
    };
}

#endif //EQ_COMMANDQUEUE_H
