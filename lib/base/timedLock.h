
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_TIMEDLOCK_H
#define EQBASE_TIMEDLOCK_H

#include <eq/base/thread.h>

namespace eq
{
namespace base
{
    class TimedLockPrivate;

    /**
     * A timed lock primitive.
     */
    class TimedLock 
    {
    public:
        /** 
         * Constructs a new timed lock.
         */
        TimedLock();


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
        TimedLockPrivate* _data;
        bool              _locked;
    };
}
}
#endif //EQBASE_TIMEDLOCK_H
