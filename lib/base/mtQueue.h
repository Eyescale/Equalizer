
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_MTQUEUE_H
#define EQBASE_MTQUEUE_H

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <pthread.h>
#include <queue>

namespace eqBase
{
    /**
     * A queue with a blocking pop() implementation, typically used between two
     * execution threads.
     *
     * @todo evaluate lock-free implementation if performance is problematic
     */
    template<class T> class MTQueue
    {
    public:
        /** 
         * Constructs a new queue.
         */
        MTQueue()
            {
                // mutex init
                int error = pthread_mutex_init( &_mutex, 0 );
                if( error )
                {
                    EQERROR << "Error creating pthread mutex: " 
                            << strerror( error ) << std::endl;
                    return;
                }
                // condvar init
                error = pthread_cond_init( &_cond, 0 );
                if( error )
                {
                    EQERROR << "Error creating pthread condition: " 
                            << strerror( error ) << std::endl;
                    return;
                }
            }

        /** Destructs the Queue. */
        ~MTQueue()
            {
                pthread_mutex_destroy( &_mutex );
                pthread_cond_destroy( &_cond );
            }

        bool empty() const { return _queue.empty(); }
        size_t size() const { return _queue.size(); }

        T* pop()
            {
                pthread_mutex_lock( &_mutex );
                while( _queue.empty( ))
                    pthread_cond_wait( &_cond, &_mutex );
                
                EQASSERT( !_queue.empty( ));
                T* element = _queue.front();
                _queue.pop_front();
                pthread_mutex_unlock( &_mutex );
                return element;
            }

        T* tryPop()
            {
                if( _queue.empty( ))
                    return 0;
                
                pthread_mutex_lock( &_mutex );
                if( _queue.empty( ))
                {
                    pthread_mutex_unlock( &_mutex );
                    return 0;
                }

                T* element = _queue.front();
                _queue.pop_front();
                pthread_mutex_unlock( &_mutex );
                return element;
            }   

        T* back() const
            {
                pthread_mutex_lock( &_mutex );
                T* element = _queue.empty() ? 0 : _queue.back();
                pthread_mutex_unlock( &_mutex );
                return element;
            }

        void push( T* element )
            {
                pthread_mutex_lock( &_mutex );
                _queue.push_back( element );
                pthread_cond_signal( &_cond );
                pthread_mutex_unlock( &_mutex );
            }

        void pushFront( T* element )
            {
                pthread_mutex_lock( &_mutex );
                _queue.push_front( element );
                pthread_cond_signal( &_cond );
                pthread_mutex_unlock( &_mutex );
            }

    private:
        std::deque<T*> _queue;

        mutable pthread_mutex_t _mutex;
        pthread_cond_t          _cond;
    };
}

#endif //EQBASE_MTQUEUE_H
