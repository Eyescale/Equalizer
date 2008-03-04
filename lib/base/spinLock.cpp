
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "spinLock.h"

#include "log.h"

#include <errno.h>
#include <pthread.h>
#ifndef WIN32
#  include <unistd.h> // for _POSIX_SPIN_LOCKS on some systems
#endif


using namespace std;

#if 0
#if _POSIX_SPIN_LOCKS > 0
#  define pthread_mutex_init     pthread_spin_init     
#  define pthread_mutex_destroy  pthread_spin_destroy  
#  define pthread_mutex_lock     pthread_spin_lock     
#  define pthread_mutex_unlock   pthread_spin_unlock   
#  define pthread_mutex_trylock  pthread_spin_trylock  
#  define pthread_mutex_t        pthread_spinlock_t
#endif
#endif

namespace eqBase
{
class SpinLockPrivate
{
public:
    pthread_mutex_t mutex;
};

SpinLock::SpinLock()
        : _data( new SpinLockPrivate )
{
    const int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
    EQINFO << " New SpinLock @" << (void*)this << endl;
}

SpinLock::~SpinLock()
{
    pthread_mutex_destroy( &_data->mutex );
    delete _data;
    _data = 0;
}

void SpinLock::set()
{
    pthread_mutex_lock( &_data->mutex );
}


void SpinLock::unset()
{
    pthread_mutex_unlock( &_data->mutex );
}


bool SpinLock::trySet()
{
    return ( pthread_mutex_trylock( &_data->mutex ) == 0 );
}


bool SpinLock::test()
{
    if( pthread_mutex_trylock( &_data->mutex ) == 0 )
    {
        pthread_mutex_unlock( &_data->mutex );
        return false;
    }
    return true;
}
}
