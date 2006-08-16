
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "timedLock.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

TimedLock::TimedLock()
        : _locked( false )
{
    int error = pthread_mutex_init( &_mutex, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
    error = pthread_cond_init( &_cond, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " << strerror( error )
                << endl;
        pthread_mutex_destroy( &_mutex );
        return;
    }
}

TimedLock::~TimedLock()
{
    pthread_mutex_destroy( &_mutex );
    pthread_cond_destroy( &_cond );
}

bool TimedLock::set( const uint32_t timeout )
{
    pthread_mutex_lock( &_mutex );

    bool acquired = true;
    while( _locked )
    {
        if( timeout )
        {
            timespec ts;
            ts.tv_sec  = (int)(timeout/1000);
            ts.tv_nsec = (timeout - ts.tv_sec*1000) * 1000000;
            
            timeval tv;
            gettimeofday( &tv, 0 );
            ts.tv_sec  += tv.tv_sec;
            ts.tv_nsec += tv.tv_usec * 1000;
            
            int error = pthread_cond_timedwait( &_cond, &_mutex, &ts );
            if( error == ETIMEDOUT )
            {
                acquired = false;
                break;
            }
        }
        else
            pthread_cond_wait( &_cond, &_mutex );
    }

    if( acquired )
    {
        EQASSERT( !_locked );
        _locked = true;
    }

    pthread_mutex_unlock( &_mutex );
    return acquired;
}

void TimedLock::unset()
{
    pthread_mutex_lock( &_mutex );
    _locked = false;
    pthread_cond_signal( &_cond );
    pthread_mutex_unlock( &_mutex );
}


bool TimedLock::trySet()
{
    pthread_mutex_lock( &_mutex );
    
    bool acquired = false;
    if( _locked )
    {
        _locked  = true;
        acquired = true;
    }

    pthread_mutex_unlock( &_mutex );
    return acquired;
}


bool TimedLock::test()
{
    return _locked;
}
