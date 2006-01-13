
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
         * Constructs a new timed lock for a given thread type.
         * 
         * @param type the type of threads accessing the lock.
         */
        TimedLock( const Thread::Type type = Thread::PTHREAD );


        /** Destructs the lock. */
        ~TimedLock();

        /** 
         * Sets the lock. 
         * 
         * @param timeout the timeout in milliseconds to wait for the lock,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         * @return <code>true</code> if the lock was acquired,
         *         <code>false</code> if not.
         */
        bool set( const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );

        /** 
         * Releases the lock.
         */
        void unset();

        /** 
         * Attempts to set the lock.
         * 
         * @return <code>true</code> if the lock was acquired,
         *         <code>false</code> if not.
         */
        bool trySet();

        /** 
         * Tests if the lock is set.
         * 
         * @return <code>true</code> if the lock is set,
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
