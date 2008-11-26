
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_MESSAGEPUMP_H
#define EQ_MESSAGEPUMP_H

#include <eq/base/base.h>

namespace eq
{
    /**
     * Defines an interface to process OS messages/events.
     */
    class MessagePump
    {
    public:
        /** Wake up dispatchOne(). */
        virtual void postWakeup() = 0;

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll() = 0;

        /** Get and dispatch at least one pending system event, blocking. */
        virtual void dispatchOne() = 0;
        
        /** Clean up, no more dispatch from thread. */
        virtual void dispatchDone(){}

        virtual ~MessagePump() {}
    };
}

#endif //EQ_MESSAGEPUMP_H
