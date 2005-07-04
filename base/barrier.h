
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BARRIER_H
#define EQBASE_BARRIER_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * A generalized barrier.
     * 
     * Depending on the thread type, a different implementation is used to
     * create the barrier.
     */
    class Barrier 
    {
    public:
        Barrier( const Thread::Type type );
        ~Barrier();

        /** 
         * Enters the barrier.
         * 
         * @param size the size of the barrier, i.e., the number of
         *             participants.
         * @return the position in which the barrier was entered.
         */
        size_t enter( const size_t size );

    private:
        Thread::Type _type;

        union
        {
            struct 
            {
                pthread_mutex_t mutex;
                pthread_cond_t  cond;
                size_t          count;
            } pthread;
        } _barrier;
    };
}

#endif //EQBASE_BARRIER_H
