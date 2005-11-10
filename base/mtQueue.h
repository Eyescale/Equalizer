
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_MTQUEUE_H
#define EQBASE_MTQUEUE_H

#include "thread.h"

#include <pthread.h>
#include <queue>

namespace eqBase
{
    /**
     * A queue between two execution threads with a blocking pop()
     * implemenation.
     * 
     * Depending on the thread type, a different implementation is used to
     * create the Queue.
     */
    template<class T> class MTQueue
    {
    public:
        /** 
         * Constructs a new Queue of the given type.
         *
         * Modified the pop() behaviour by letting it return the front-most
         * value. 
         * 
         * @param type the type of threads accessing the Queue.
         */
        MTQueue( const Thread::Type type );

        /** Destructs the Queue. */
        ~MTQueue(){}

        const T& pop()
            {
                pthread_mutex_lock( &_sync.pthread.mutex );
                while( _queue.size() == 0 )
                    pthread_cond_wait( &_sync.pthread.cond,
                                       &_sync.pthread.mutex );

                T& element = _queue.front();
                _queue.pop();
                pthread_mutex_unlock( &_sync.pthread.mutex );
                return element;
            }

        void push( const T& element )
            {
                pthread_mutex_lock( &_sync.pthread.mutex );
                _queue.push( element );
                pthread_cond_signal( &_sync.pthread.cond );
                pthread_mutex_unlock( &_sync.pthread.mutex );
            }

    private:
        Thread::Type _type;

        std::queue<T> _queue;

        union
        {
            struct 
            {
                pthread_mutex_t mutex;
                pthread_cond_t  cond;
            } pthread;
        } _sync;
    };
}

#endif //EQBASE_MTQUEUE_H
