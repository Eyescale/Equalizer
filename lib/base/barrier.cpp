
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "log.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

Barrier::Barrier()
        : _count(0)
{
    // mutex init
    int error = pthread_mutex_init( &_mutex, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror( error )
                << endl;
        return;
    }
    // condvar init
    error = pthread_cond_init( &_cond, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << endl;
        return;
    }
}

Barrier::~Barrier()
{
    pthread_mutex_destroy( &_mutex );
    pthread_cond_destroy( &_cond );
}

size_t Barrier::enter( const size_t size )
{
    pthread_mutex_lock( &_mutex );
    const size_t pos = _count++;
    //INFO << "barrier enter, pos " << pos << " of " << size << endl;

    if( _count >= size ) // barrier reached, release
    {
        _count = 0;
        pthread_cond_broadcast( &_cond );
        pthread_mutex_unlock( &_mutex );
    }
    else // wait
    {
        pthread_cond_wait( &_cond, &_mutex );
        pthread_mutex_unlock( &_mutex );
    }
    return pos;
}
