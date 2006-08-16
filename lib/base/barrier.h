
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_BARRIER_H
#define EQBASE_BARRIER_H

#include <pthread.h>

namespace eqBase
{
    /**
     * A barrier primitive.
     */
    class Barrier 
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier();

        /** Destructs the barrier. */
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
        pthread_mutex_t _mutex;
        pthread_cond_t  _cond;
        size_t          _count;
    };
}

#endif //EQBASE_BARRIER_H
