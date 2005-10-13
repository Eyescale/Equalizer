
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "timedLock.h"

#include <errno.h>

using namespace eqBase;
using namespace std;

TimedLock::TimedLock( const Thread::Type type )
        : _type( type )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            int error = pthread_mutex_init(&_lock.pthread.mutex,NULL);
            if( error )
            {
                ERROR << "Error creating pthread mutex: " 
                      << strerror( error ) << endl;
                return;
            }
            error = pthread_cond_init( &_lock.pthread.cond, NULL );
            if( error )
            {
                ERROR << "Error creating pthread condition: " 
                      << strerror( error ) << endl;
                pthread_mutex_destroy( &_lock.pthread.mutex );
                return;
            }
        } break;

        default:
            ERROR << "not implemented" << endl;
    }
}

TimedLock::~TimedLock()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_destroy( &_lock.pthread.mutex );
            pthread_cond_destroy( &_lock.pthread.cond );
            break;

        default:
            ERROR << "not implemented" << endl;
    }
}

bool TimedLock::set( const uint timeout )
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            pthread_mutex_lock( &_lock.pthread.mutex );

            bool acquired = true;
            while( _lock.pthread.locked )
            {
                if( timeout )
                {
                    timespec ts;
                    ts.tv_sec  = (int)(timeout/1000);
                    ts.tv_nsec = (timeout - ts.tv_sec*1000) * 1000000;

                    int error = pthread_cond_timedwait( &_lock.pthread.cond,
                                                        &_lock.pthread.mutex,
                                                        &ts );
                    if( error == ETIMEDOUT )
                    {
                        acquired = false;
                        break;
                    }
                }
                else
                    pthread_cond_wait( &_lock.pthread.cond,
                                       &_lock.pthread.mutex );
            }

            if( acquired )
                _lock.pthread.locked = true;
            
            pthread_mutex_unlock( &_lock.pthread.mutex );
            return acquired;
        }
 
        default:
            ERROR << "not implemented" << endl;
    }
    return false;
}


void TimedLock::unset()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            pthread_mutex_lock( &_lock.pthread.mutex );

            _lock.pthread.locked = false;
            pthread_cond_signal( &_lock.pthread.cond );

            pthread_mutex_unlock( &_lock.pthread.mutex );
            return;

        default:
            ERROR << "not implemented" << endl;
    }
}


bool TimedLock::trySet()
{
    switch( _type )
    {
        case Thread::PTHREAD:
        {
            pthread_mutex_lock( &_lock.pthread.mutex );

            bool acquired = false;
            if( _lock.pthread.locked )
            {
                _lock.pthread.locked = true;
                acquired             = true;
            }

            pthread_mutex_unlock( &_lock.pthread.mutex );
            return acquired;
        }

        default:
            ERROR << "not implemented" << endl;
            return false;
    }
}


bool TimedLock::test()
{
    switch( _type )
    {
        case Thread::PTHREAD:
            return _lock.pthread.locked;

        default:
            ERROR << "not implemented" << endl;
            return false;
    }
}


