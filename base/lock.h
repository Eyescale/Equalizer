
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOCK_H
#define EQBASE_LOCK_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * A generalized lock.
     * 
     * Depending on the thread type, a different implementation is used to
     * create the lock.
     */
    class Lock 
    {

    public:
        Lock( const Thread::Type type );
        ~Lock();

        /** 
         * Sets the lock. 
         */
        void set();

        /** 
         * Releases the lock.
         */
        void unset();

        /** 
         * Attempts to set the lock.
         * 
         * @return <code>true</code> if the lock was set, <code>false</code> if
         *         it was not set.
         */
        bool trySet();

        /** 
         * Tests if the lock is set.
         * 
         * @return <code>true</code> if the lock is set, <code>false</code> if
         *         it is not set.
         */
        bool test(); 

    private:
        Thread::Type _type;

        union
        {
            pthread_mutex_t pthread;
        } _lock;
    };
}

#endif //EQBASE_LOCK_H
