
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "lock.h"

#include "log.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <string.h>

using namespace std;

namespace eq
{
namespace base
{
class LockPrivate
{
public:
#ifdef WIN32
    CRITICAL_SECTION cs; 
#else
    pthread_mutex_t mutex;
#endif
};

Lock::Lock()
        : _data ( new LockPrivate )
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
    EQINFO << "New Lock @" << (void*)this << endl;
}

Lock::~Lock()
{
#ifdef WIN32
    DeleteCriticalSection( &_data->cs ); 
#else
    pthread_mutex_destroy( &_data->mutex );
#endif
    delete _data;
    _data = 0;
}

void Lock::set()
{
#ifdef WIN32
    EnterCriticalSection( &_data->cs );
#else
    pthread_mutex_lock( &_data->mutex );
#endif
}


void Lock::unset()
{
#ifdef WIN32
    LeaveCriticalSection( &_data->cs );
#else
    pthread_mutex_unlock( &_data->mutex );
#endif
}


bool Lock::trySet()
{
#ifdef WIN32
    return TryEnterCriticalSection( &_data->cs );
#else
    return ( pthread_mutex_trylock( &_data->mutex ) == 0 );
#endif
}


bool Lock::test()
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
