
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "spinLock.h"

#include "log.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>
#ifndef WIN32
#  include <unistd.h> // for _POSIX_SPIN_LOCKS on some systems
#endif


using namespace std;

#if 0 // temporarily disabled - causes prio inversion, observed on Linux
#if _POSIX_SPIN_LOCKS > 0
#  define pthread_mutex_init     pthread_spin_init     
#  define pthread_mutex_destroy  pthread_spin_destroy  
#  define pthread_mutex_lock     pthread_spin_lock     
#  define pthread_mutex_unlock   pthread_spin_unlock   
#  define pthread_mutex_trylock  pthread_spin_trylock  
#  define pthread_mutex_t        pthread_spinlock_t
#endif
#endif

namespace eq
{
namespace base
{
class SpinLockPrivate
{
public:
#ifdef WIN32
    CRITICAL_SECTION cs; 
#else
    pthread_mutex_t mutex;
#endif
};

SpinLock::SpinLock()
        : _data( new SpinLockPrivate )
{
#ifdef WIN32
    InitializeCriticalSection( &_data->cs );
#else
    const int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
#endif
    EQVERB << " New SpinLock @" << (void*)this << endl;
}

SpinLock::~SpinLock()
{
#ifdef WIN32
    DeleteCriticalSection( &_data->cs ); 
#else
    pthread_mutex_destroy( &_data->mutex );
#endif
    delete _data;
    _data = 0;
}

void SpinLock::set()
{
#ifdef WIN32
    EnterCriticalSection( &_data->cs );
#else
    pthread_mutex_lock( &_data->mutex );
#endif
}


void SpinLock::unset()
{
#ifdef WIN32
    LeaveCriticalSection( &_data->cs );
#else
    pthread_mutex_unlock( &_data->mutex );
#endif
}


bool SpinLock::trySet()
{
#ifdef WIN32
    return TryEnterCriticalSection( &_data->cs );
#else
    return ( pthread_mutex_trylock( &_data->mutex ) == 0 );
#endif
}


bool SpinLock::test()
{
    if( trySet( ))
    {
        unset();
        return false;
    }
    return true;
}
}
}
