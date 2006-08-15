
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_THREADLISTENER_H
#define EQBASE_THREADLISTENER_H

#include "base.h"

namespace eqBase
{
    /**
     * A listener interface to monitor thread state changes.
     */
    class ThreadListener
    {
    public:
        virtual ~ThreadListener() {}

        /**
         * Notify that a new thread started.
         * @sa Thread::start
         */
        virtual void notifyThreadStarted(){};

        /**
         * Notify that the thread is about to stop.
         * @sa Thread::exit
         */
        virtual void notifyThreadStopping(){};        
    };
}

#endif //EQBASE_THREADLISTENER_H
