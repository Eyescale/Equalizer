/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTTHREAD_H
#define EQ_EVENTTHREAD_H

#include <eq/base/thread.h>

namespace eq
{
    /**
     * The per-node event processing thread.
     */
    class EventThread : public eqBase::Thread
    {
    public:
        /** 
         * Constructs a new event thread.
         */
        EventThread(){}

        /**
         * Destructs the event thread.
         */
        virtual ~EventThread(){}
        
    private:
    };
}

#endif // EQ_EVENTTHREAD_H

