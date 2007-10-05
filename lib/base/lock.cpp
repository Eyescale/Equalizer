
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "lock.h"

#include "log.h"

#include <errno.h>
#include <pthread.h>

using namespace std;

namespace eqBase
{
class LockPrivate
{
public:
    pthread_mutex_t mutex;
};

Lock::Lock()
        : _data ( new LockPrivate )
{
    const int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
    EQINFO << "New Lock @" << (void*)this << endl;
}

Lock::~Lock()
{
    pthread_mutex_destroy( &_data->mutex );
    delete _data;
    _data = 0;
}

void Lock::set()
{
    pthread_mutex_lock( &_data->mutex );
}


void Lock::unset()
{
    pthread_mutex_unlock( &_data->mutex );
}


bool Lock::trySet()
{
    return ( pthread_mutex_trylock( &_data->mutex ) == 0 );
}


bool Lock::test()
{
    if( pthread_mutex_trylock( &_data->mutex ) == 0 )
    {
        pthread_mutex_unlock( &_data->mutex );
        return false;
    }
    return true;
}
}
