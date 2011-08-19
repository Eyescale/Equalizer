
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// Based on http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
// 3.2. The SetEvent Solution

// TODO: enhance with real error codes to match pthread API

#ifndef ETIMEDOUT
#  define ETIMEDOUT 10060
#endif

#include <co/base/os.h>

namespace
{

struct pthread_cond_t
{
    unsigned int waiters_count_;    
    CRITICAL_SECTION waiters_count_lock_;    
    enum
    {
        SIGNAL,
        BROADCAST,
        MAX_EVENTS
    };
    
    HANDLE events_[MAX_EVENTS];
};

typedef int pthread_condattr_t;
typedef int pthread_mutexattr_t;

typedef CRITICAL_SECTION pthread_mutex_t;

int pthread_mutex_init( pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)
{
    EQASSERT( !attr );
    InitializeCriticalSection( mutex );
    return 0;
}

int pthread_mutex_destroy( pthread_mutex_t* mutex )
{
    DeleteCriticalSection( mutex );
    return 0;
}

int pthread_mutex_lock( pthread_mutex_t* mutex )
{
    EnterCriticalSection( mutex );
    return 0;
}

int pthread_mutex_unlock( pthread_mutex_t* mutex )
{
    LeaveCriticalSection( mutex );
    return 0;
}

int pthread_cond_init( pthread_cond_t* cond, const pthread_condattr_t* )
{
    InitializeCriticalSection( &cond->waiters_count_lock_ );

    // Initialize the count to 0.
    cond->waiters_count_ = 0;

    // Create an auto-reset event.
    cond->events_[pthread_cond_t::SIGNAL] =
        CreateEvent( NULL,  // no security
                     FALSE, // auto-reset event
                     FALSE, // non-signaled initially
                     NULL ); // unnamed

    // Create a manual-reset event.
    cond->events_[pthread_cond_t::BROADCAST] =
        CreateEvent( NULL,  // no security
                     TRUE,  // manual-reset
                     FALSE, // non-signaled initially
                     NULL ); // unnamed

    return 0;
}

int pthread_cond_destroy( pthread_cond_t* cond )
{
    DeleteCriticalSection( &cond->waiters_count_lock_ );
    CloseHandle( cond->events_[pthread_cond_t::SIGNAL] );
    CloseHandle( cond->events_[pthread_cond_t::BROADCAST] );
    return 0;
}

int pthread_cond_signal( pthread_cond_t* cond )
{
    EnterCriticalSection( &cond->waiters_count_lock_ );
    int have_waiters = cond->waiters_count_ > 0;
    LeaveCriticalSection( &cond->waiters_count_lock_ );

    if( have_waiters )
        SetEvent( cond->events_[pthread_cond_t::SIGNAL] );
    return 0;
}

int pthread_cond_broadcast( pthread_cond_t* cond )
{
    EnterCriticalSection( &cond->waiters_count_lock_ );
    int have_waiters = cond->waiters_count_ > 0;
    LeaveCriticalSection( &cond->waiters_count_lock_ );

    if( have_waiters )
        SetEvent (cond->events_[pthread_cond_t::BROADCAST]);
    return 0;
}

int pthread_cond_wait( pthread_cond_t* cond, pthread_mutex_t* mutex )
{
    EnterCriticalSection( &cond->waiters_count_lock_ );
    cond->waiters_count_++;
    LeaveCriticalSection( &cond->waiters_count_lock_ );

    // It's ok to release the <external_mutex> here since Win32
    // manual-reset events maintain state when used with
    // <SetEvent>.  This avoids the "lost wakeup" bug...
    LeaveCriticalSection( mutex );

    // Wait for either event to become signaled due to <pthread_cond_signal>
    // being called or <pthread_cond_broadcast> being called.
    int result = WaitForMultipleObjects( 2, cond->events_, FALSE, INFINITE);

    EnterCriticalSection( &cond->waiters_count_lock_ );
    cond->waiters_count_--;
    bool last_waiter =
        result == WAIT_OBJECT_0 + pthread_cond_t::BROADCAST &&
        cond->waiters_count_ == 0;
    LeaveCriticalSection( &cond->waiters_count_lock_ );

    // Some thread called <pthread_cond_broadcast>.
    if( last_waiter )
        // We're the last waiter to be notified or to stop waiting, so
        // reset the manual event. 
        ResetEvent( cond->events_[pthread_cond_t::BROADCAST] );

    // Reacquire the <external_mutex>.
    EnterCriticalSection( mutex );

    return 0;
}

int pthread_cond_timedwait_w32_np( pthread_cond_t* cond, pthread_mutex_t* mutex,
                                   const unsigned timeout )
{
    EnterCriticalSection( &cond->waiters_count_lock_ );
    cond->waiters_count_++;
    LeaveCriticalSection( &cond->waiters_count_lock_ );

    // It's ok to release the <external_mutex> here since Win32
    // manual-reset events maintain state when used with
    // <SetEvent>.  This avoids the "lost wakeup" bug...
    LeaveCriticalSection( mutex );

    // Wait for either event to become signaled due to <pthread_cond_signal>
    // being called or <pthread_cond_broadcast> being called.
    int result = WaitForMultipleObjects( 2, cond->events_, FALSE, timeout );

    EnterCriticalSection( &cond->waiters_count_lock_ );
    cond->waiters_count_--;
    bool last_waiter =
        result == WAIT_OBJECT_0 + pthread_cond_t::BROADCAST &&
        cond->waiters_count_ == 0;
    LeaveCriticalSection( &cond->waiters_count_lock_ );

    // Some thread called <pthread_cond_broadcast>.
    if( last_waiter )
        // We're the last waiter to be notified or to stop waiting, so
        // reset the manual event. 
        ResetEvent( cond->events_[pthread_cond_t::BROADCAST] );

    // Reacquire the <external_mutex>.
    EnterCriticalSection( mutex );

    if ( result == WAIT_TIMEOUT )
        return ETIMEDOUT;
    return 0;
}

}
