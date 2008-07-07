
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "log.h"

#include <errno.h>
#include <pthread.h>

using namespace std;

namespace eq
{
namespace base
{
class BarrierPrivate
{
public:
    BarrierPrivate() : count( 0 ) {}

    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    size_t          count;
};

Barrier::Barrier()
        : _data( new BarrierPrivate( ))
{
    // mutex init
    int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror( error )
                << endl;
        return;
    }
    // condvar init
    error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << endl;
        return;
    }
}

Barrier::~Barrier()
{
    pthread_mutex_destroy( &_data->mutex );
    pthread_cond_destroy( &_data->cond );
    delete _data;
    _data = 0;
}

size_t Barrier::enter( const size_t size )
{
    pthread_mutex_lock( &_data->mutex );
    const size_t pos = _data->count++;

    if( _data->count >= size ) // barrier reached, release
    {
        _data->count = 0;
        pthread_cond_broadcast( &_data->cond );
        pthread_mutex_unlock( &_data->mutex );
    }
    else // wait
    {
        pthread_cond_wait( &_data->cond, &_data->mutex );
        pthread_mutex_unlock( &_data->mutex );
    }
    return pos;
}
}
}
