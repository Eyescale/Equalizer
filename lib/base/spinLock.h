
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SPINLOCK_H
#define EQBASE_SPINLOCK_H

#include <eq/base/base.h>

#include <pthread.h>
#ifndef WIN32
#  include <unistd.h> // for _POSIX_SPIN_LOCKS on some systems
#endif

namespace eqBase
{
    /**
     * A spinlock primitive.
     *
     * May use non-spinning fallback on certain systems.
     */
    class EQ_EXPORT SpinLock 
    {
    public:
        /** 
         * Constructs a new spinlock.
         */
        SpinLock();


        /** Destructs the spinlock. */
        ~SpinLock();

        /** 
         * Sets the spinlock. 
         */
        void set();

        /** 
         * Releases the spinlock.
         */
        void unset();

        /** 
         * Attempts to set the spinlock.
         * 
         * @return <code>true</code> if the spinlock was set, <code>false</code>
         *         if it was not set.
         */
        bool trySet();

        /** 
         * Tests if the spinlock is set.
         * 
         * @return <code>true</code> if the spinlock is set, <code>false</code>
         *         if it is not set.
         */
        bool test(); 

    private:
#if _POSIX_SPIN_LOCKS > 0
        pthread_spinlock_t _mutex;
#else
        pthread_mutex_t _mutex;
#endif
    };
}

#endif //EQBASE_SPINLOCK_H
