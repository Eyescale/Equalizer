
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SCOPEDMUTEX_H
#define EQBASE_SCOPEDMUTEX_H

#include <eq/base/lock.h>

namespace eq
{
namespace base
{
    /**
     * A scoped mutex.
     * 
     * The mutex is automatically set upon creation, and released when the
     * scoped mutex is destroyed, e.g., when the scope is left. The scoped mutex
     * does nothing if a NULL pointer for the lock/spin lock is passed.
     */
    template< typename T > // T == Lock or SpinLock
    class ScopedMutex
    {
    public:
        /** 
         * Constructs a new scoped mutex using the given lock.
         * 
         * @param lock the mutex to set and unset.
         */
        explicit ScopedMutex( T* lock ) : _lock( lock )
            { if( lock ) lock->set(); }
        explicit ScopedMutex( T& lock ) : _lock( &lock )
            { lock.set(); }

        /** Destructs the scoped mutex and unsets the mutex. */
        ~ScopedMutex() { if( _lock ) _lock->unset(); }

    private:
        ScopedMutex(){}
        T* _lock;
    };
}
}
#endif //EQBASE_SCOPEDMUTEX_H
