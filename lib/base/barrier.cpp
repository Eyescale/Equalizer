
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Barrier::Barrier( const Thread::Type type )
        : _type( type )
{
    switch( type )
    {
        case Thread::PTHREAD:
        {
            _barrier.pthread.count = 0;
            // mutex init
            int error = pthread_mutex_init( &_barrier.pthread.mutex, NULL );
            if( error )
            {
                EQERROR << "Error creating pthread mutex: " << strerror( error )
                      << endl;
                return;
            }
            // condvar init
            error = pthread_cond_init( &_barrier.pthread.cond, NULL );
            if( error )
            {
                EQERROR << "Error creating pthread condition: " 
                      << strerror( error ) << endl;
                return;
            }
        } return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

Barrier::~Barrier()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_destroy( &_barrier.pthread.mutex );
            pthread_cond_destroy( &_barrier.pthread.cond );
            return;

        default:
            EQERROR << "not implemented" << endl;
    }
}

size_t Barrier::enter( const size_t size )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            pthread_mutex_lock( &_barrier.pthread.mutex );
            const size_t pos = _barrier.pthread.count++;
            //INFO << "barrier enter, pos " << pos << " of " << size << endl;

            if( _barrier.pthread.count >= size ) // barrier reached, release
            {
                _barrier.pthread.count = 0;
                pthread_cond_broadcast( &_barrier.pthread.cond );
                pthread_mutex_unlock( &_barrier.pthread.mutex );
            }
            else // wait
            {
                pthread_cond_wait( &_barrier.pthread.cond,
                                   &_barrier.pthread.mutex );
                pthread_mutex_unlock( &_barrier.pthread.mutex );
            }
            return pos;
        }

        default:
            EQERROR << "not implemented" << endl;
            return 0;
    }
}
