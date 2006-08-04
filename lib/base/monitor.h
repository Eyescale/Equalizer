
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_MONITOR_H
#define EQBASE_MONITOR_H

#include "thread.h"

#include <pthread.h>

namespace eqBase
{
    /**
     * A monitor primitive.
     *
     * A monitor has a value, which can be monitored to reach a certain
     * state. The caller is blocked until the condition is fulfilled.
     */
    template< typename T > class Monitor 
    {
    public:
        /** 
         * Constructs a new monitor for the given thread type.
         * 
         * @param type the type of threads accessing the monitor.
         */
        Monitor( const Thread::Type type = Thread::PTHREAD )
                : _type( type ), _var( 0 )
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                    {
                        int error = pthread_cond_init( &_pthread.cond, NULL );
                        if( error )
                        {
                            EQERROR << "Error creating pthread condition: " 
                                    << strerror( error ) << std::endl;
                            return;
                        } 
                        
                        error = pthread_mutex_init( &_pthread.mutex, NULL );
                        if( error )
                        {
                            EQERROR << "Error creating pthread mutex: " 
                                    << strerror( error ) << std::endl;
                            return;
                        } 
                    } break;
                    
                    default: EQUNIMPLEMENTED;
                }
            }
        
        /** Destructs the monitor. */
        ~Monitor()
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_cond_destroy( &_pthread.cond );
                        pthread_mutex_destroy( &_pthread.mutex );
                        return;
            
                    default: EQUNIMPLEMENTED;
                }
            }

        /** @name Changing the monitored value. */
        //*{
        Monitor& operator++ ()    // prefix only
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_mutex_lock( &_pthread.mutex );
                        ++_var;
                        pthread_cond_broadcast( &_pthread.cond );
                        pthread_mutex_unlock( &_pthread.mutex );
                        return *this;

                    default:
                        EQUNIMPLEMENTED;
                }
                return *this;
            }
        Monitor& operator-- ()    // prefix only
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_mutex_lock( &_pthread.mutex );
                        --_var;
                        pthread_cond_broadcast( &_pthread.cond );
                        pthread_mutex_unlock( &_pthread.mutex );
                        return *this;

                    default:
                        EQUNIMPLEMENTED;
                }
                return *this;
            }
        void set( const T& val )
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_mutex_lock( &_pthread.mutex );
                        _var = val;
                        pthread_cond_broadcast( &_pthread.cond );
                        pthread_mutex_unlock( &_pthread.mutex );
                        return;

                    default:
                        EQUNIMPLEMENTED;
                }
                return *this;
            }
        //*}

        /** Monitor the value. */
        //*{
        void waitEQ( const T& val )
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_mutex_lock( &_pthread.mutex );
                        while( _var != val )
                            pthread_cond_wait( &_pthread.cond, &_pthread.mutex);
                        pthread_mutex_unlock( &_pthread.mutex );
                        return;
                    default:
                        EQUNIMPLEMENTED;
                }
            }
        void waitGE( const T& val )
            {
                switch( _type )
                {
                    case Thread::PTHREAD:
                        pthread_mutex_lock( &_pthread.mutex );
                        while( _var < val )
                            pthread_cond_wait( &_pthread.cond, &_pthread.mutex);
                        pthread_mutex_unlock( &_pthread.mutex );
                        return;
                    default:
                        EQUNIMPLEMENTED;
                }
            }
        //*}
    private:
        Thread::Type _type;
        T            _var;

        union
        {
            struct
            {
                pthread_cond_t  cond;
                pthread_mutex_t mutex;

            } _pthread;
        };
    };
}

#endif //EQBASE_MONITOR_H
