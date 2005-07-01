
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Barrier::Barrier( const Thread::Type type )
        : _type( type )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            _barrier.pthread.count = 0;
            // mutex init
            int nTries = 10;
            while( nTries-- )
            {
                const int error = pthread_mutex_init( &_barrier.pthread.mutex,
                    NULL );
                switch( error )
                {
                    case 0: // ok
                        return;
                    case EAGAIN:
                        break;
                    default:
                        ERROR << "Error creating pthread mutex: " 
                              << strerror( error ) << endl;
                        return;
                }
            } 
            if( nTries == 0 )
            {
                ERROR << "Error creating pthread mutex"  << endl;
                return;
            }
            // condition init
            nTries = 10;
            while( nTries-- )
            {
                const int error = pthread_cond_init( &_barrier.pthread.cond,
                    NULL );
                switch( error )
                {
                    case 0: // ok
                        return;
                    case EAGAIN:
                        break;
                    default:
                        ERROR << "Error creating pthread condition: " 
                              << strerror( error ) << endl;
                        return;
                }
            } 
            if( nTries == 0 )
            {
                ERROR << "Error creating pthread condition"  << endl;
                return;
            }

        } return;

        default:
            ERROR << "not implemented" << endl;
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
            ERROR << "not implemented" << endl;
    }
}

size_t Barrier::enter( const size_t size )
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_barrier.pthread.mutex );
            size_t pos = _barrier.pthread.count++;

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
            }
            return pos;

        default:
            ERROR << "not implemented" << endl;
    }
    return 0;
}
