
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SHM_H
#define EQBASE_SHM_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * An interface to allocate shared memory across processes.
     */
    class Shm 
    {

    public:
        /** 
         * Constructs a new shm of the given type.
         * 
         * @param type the type of threads accessing the shm.
         */
        Shm( const Thread::Type type );


        /** Destructs the shm. */
        ~Shm();

        /** 
         * Sets the shm. 
         */
        void set();

        /** 
         * Releases the shm.
         */
        void unset();

        /** 
         * Attempts to set the shm.
         * 
         * @return <code>true</code> if the shm was set, <code>false</code> if
         *         it was not set.
         */
        bool trySet();

        /** 
         * Tests if the shm is set.
         * 
         * @return <code>true</code> if the shm is set, <code>false</code> if
         *         it is not set.
         */
        bool test(); 

    private:
        Thread::Type _type;

        union
        {
            pthread_mutex_t pthread;
        } _shm;
    };
}

#endif //EQBASE_SHM_H
