
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "spinLock.h"

#include "log.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

#if _POSIX_SPIN_LOCKS > 0
#  define pthread_mutex_init     pthread_spin_init     
#  define pthread_mutex_destroy  pthread_spin_destroy  
#  define pthread_mutex_lock     pthread_spin_lock     
#  define pthread_mutex_unlock   pthread_spin_unlock   
#  define pthread_mutex_trylock  pthread_spin_trylock  
#endif

SpinLock::SpinLock()
{
    const int error = pthread_mutex_init( &_mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
}

SpinLock::~SpinLock()
{
    pthread_mutex_destroy( &_mutex );
}

void SpinLock::set()
{
    pthread_mutex_lock( &_mutex );
}


void SpinLock::unset()
{
    pthread_mutex_unlock( &_mutex );
}


bool SpinLock::trySet()
{
    return ( pthread_mutex_trylock( &_mutex ) == 0 );
}


bool SpinLock::test()
{
    if( pthread_mutex_trylock( &_mutex ) == 0 )
    {
        pthread_mutex_unlock( &_mutex );
        return false;
    }
    return true;
}


