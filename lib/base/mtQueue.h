
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
     * implementation.
     *
     * OPT: evaluate lock-free implementation if performance is problematic
     */
    template<class T> class MTQueue
    {
    public:
        /** 
         * Constructs a new Queue of the given type.
         *
         * @param type the type of threads accessing the Queue.
         */
        MTQueue( const Thread::Type type = Thread::PTHREAD )
                : _type( type )
            {
                switch( type )
                {
                    case Thread::PTHREAD:
                    {
                        // mutex init
                        int error = pthread_mutex_init( &_sync.pthread.mutex,
                                                        NULL );
                        if( error )
                        {
                            EQERROR << "Error creating pthread mutex: " 
                                  << strerror( error ) << std::endl;
                            return;
                        }
                        // condvar init
                        error = pthread_cond_init( &_sync.pthread.cond, NULL );
                        if( error )
                        {
                            EQERROR << "Error creating pthread condition: " 
                                  << strerror( error ) << std::endl;
                            return;
                        }
                        break;
                    }
                    default:
                        EQASSERT( "not implemented" == NULL );
                }
            }

        /** Destructs the Queue. */
        ~MTQueue()
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_mutex_destroy( &_sync.pthread.mutex );
                        pthread_cond_destroy( &_sync.pthread.cond );
                        break;
                        
                    default:
                        EQERROR << "not implemented" << std::endl;
                }
            }

        bool empty() const { return _queue.empty(); }

        T* pop()
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                    {
                        pthread_mutex_lock( &_sync.pthread.mutex );
                        while( _queue.empty( ))
                            pthread_cond_wait( &_sync.pthread.cond,
                                               &_sync.pthread.mutex );
                        
                        EQASSERT( !_queue.empty( ));
                        T* element = _queue.front();
                        _queue.pop_front();
                        pthread_mutex_unlock( &_sync.pthread.mutex );
                        return element;
                    }   
                    default:
                        EQERROR << "not implemented" << std::endl;
                        abort();
                }
            }

        T* tryPop()
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                    {
                        if( _queue.empty( ))
                            return NULL;

                        pthread_mutex_lock( &_sync.pthread.mutex );
                        if( _queue.empty( ))
                        {
                            pthread_mutex_unlock( &_sync.pthread.mutex );
                            return NULL;
                        }

                        T* element = _queue.front();
                        _queue.pop_front();
                        pthread_mutex_unlock( &_sync.pthread.mutex );
                        return element;
                    }   
                    default:
                        EQERROR << "not implemented" << std::endl;
                        abort();
                }
            }

        T* back() const
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                    {
                        pthread_mutex_lock( &_sync.pthread.mutex );
                        T* element = _queue.empty() ? NULL : _queue.back();
                        pthread_mutex_unlock( &_sync.pthread.mutex );
                        return element;
                    }   
                    default:
                        EQERROR << "not implemented" << std::endl;
                        abort();
                }
            }

        void push( T* element )
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                    {
                        pthread_mutex_lock( &_sync.pthread.mutex );
                        _queue.push_back( element );
                        pthread_cond_signal( &_sync.pthread.cond );
                        pthread_mutex_unlock( &_sync.pthread.mutex );
                        break;
                    }   
                    default:
                        EQERROR << "not implemented" << std::endl;
                }
            }

        void pushFront( T* element )
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                    {
                        pthread_mutex_lock( &_sync.pthread.mutex );
                        _queue.push_front( element );
                        pthread_cond_signal( &_sync.pthread.cond );
                        pthread_mutex_unlock( &_sync.pthread.mutex );
                        break;
                    }   
                    default:
                        EQERROR << "not implemented" << std::endl;
                }
            }

    private:
        Thread::Type   _type;
        std::deque<T*> _queue;

        union
        {
            struct 
            {
                mutable pthread_mutex_t mutex;
                pthread_cond_t  cond;
            } pthread;
        } _sync;
    };
}

#endif //EQBASE_MTQUEUE_H
