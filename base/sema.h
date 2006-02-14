
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_SEMA_H
#define EQBASE_SEMA_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * A generalized semaphore for different thread types.
     * 
     * Depending on the thread type, a different implementation is used to
     * create the sema.
     */
    class Sema 
    {
    public:
        /** 
         * Constructs a new sema of the given type.
         * 
         * @param type the type of threads accessing the sema.
         */
        Sema( const Thread::Type type = Thread::PTHREAD );


        /** Destructs the sema. */
        ~Sema();

        /** 
         * Post (v) operation.
         */
        void post();

        /** 
         * Wait (p) operation. 
         */
        void wait();

        /** 
         * Bulk post or wait.
         * 
         * @param delta the resource delta to be applied.
         */
        void adjust( const int delta );

    private:
        Thread::Type _type;

        union
        {
            struct
            {
                pthread_cond_t  cond;
                pthread_mutex_t mutex;

            } _pthread;
        };

        uint32_t _value;
    };
}

#endif //EQBASE_SEMA_H
