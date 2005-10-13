
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_TIMEDLOCK_H
#define EQBASE_TIMEDLOCK_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * A generalized lock with time out capabilities for different thread
     * types.
     */
    class TimedLock 
    {
    public:
        /** 
         * Constructs a new timed lock of the given type.
         * 
         * @param type the type of threads accessing the timed lock.
         */
        TimedLock( const Thread::Type type = Thread::PTHREAD );


        /** Destructs the timed lock. */
        ~TimedLock();

        /** 
         * Sets the timed lock. 
         * 
         * @param timeout the timeout in milliseconds to wait for the lock,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         */
        bool set( const uint timeout = EQ_TIMEOUT_INDEFINITE );

        /** 
         * Releases the timed lock.
         */
        void unset();

        /** 
         * Attempts to set the timed lock.
         * 
         * @return <code>true</code> if the timed lock was set,
         *         <code>false</code> if it was not set.
         */
        bool trySet();

        /** 
         * Tests if the timed lock is set.
         * 
         * @return <code>true</code> if the timed lock is set,
         *         <code>false</code> if it is not set.
         */
        bool test(); 

    private:
        Thread::Type _type;

        union
        {
            struct
            {
                pthread_mutex_t mutex;
                pthread_cond_t  cond;
                bool            locked;
            } pthread;
        } _lock;
    };
}

#endif //EQBASE_TIMEDLOCK_H
