
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LOCK_H
#define EQBASE_LOCK_H

#include <eq/base/base.h>

namespace eqBase
{
    class LockPrivate;

    /**
     * A lock (mutex) primitive.
     */
    class EQ_EXPORT Lock 
    {
    public:
        /** 
         * Constructs a new lock.
         */
        Lock();


        /** Destructs the lock. */
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
        LockPrivate* _data;
    };
}

#endif //EQBASE_LOCK_H
