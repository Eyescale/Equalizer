
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include "condition.h"

#include <cstring>
#include <errno.h>

#ifdef _WIN32
#  include "condition_w32.ipp"
#else
#  include <pthread.h>
#  include <sys/timeb.h>
#endif

namespace co
{
namespace base
{
namespace detail
{
class Condition
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};
}

Condition::Condition()
        : _impl( new detail::Condition )
{
    // mutex init
    int error = pthread_mutex_init( &_impl->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror( error )
                << std::endl;
        return;
    }

    // condvar init
    error = pthread_cond_init( &_impl->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " << strerror( error )
                << std::endl;
        return;
    }
}

Condition::~Condition()
{
    int error = pthread_mutex_destroy( &_impl->mutex );
    if( error )
        EQERROR << "Error destroying pthread mutex: " << strerror( error )
                << std::endl;

    error = pthread_cond_destroy( &_impl->cond );
    if( error )
        EQERROR << "Error destroying pthread condition: " << strerror( error )
                << std::endl;

    delete _impl;
}

void Condition::lock()
{
    pthread_mutex_lock( &_impl->mutex );
}

void Condition::signal()
{
    pthread_cond_signal( &_impl->cond );
}

void Condition::broadcast()
{
    pthread_cond_broadcast( &_impl->cond );
}

void Condition::unlock()
{
    pthread_mutex_unlock( &_impl->mutex );
}

void Condition::wait()
{
    pthread_cond_wait( &_impl->cond, &_impl->mutex );
}

bool Condition::timedWait( const uint32_t timeout )
{
    if( timeout == EQ_TIMEOUT_INDEFINITE )
    {
        wait();
        return true;
    }

    const uint32_t time = timeout == EQ_TIMEOUT_DEFAULT ?
        300000 /* 5 min */ : timeout;

#ifdef _WIN32
    int error = pthread_cond_timedwait_w32_np( &_impl->cond, &_impl->mutex,
                                               time );
#else
    timespec ts = { 0, 0 };
    ts.tv_sec  = static_cast<int>( time / 1000 );
    ts.tv_nsec = ( time - ts.tv_sec * 1000 ) * 1000000;

    timeb tb;
    ftime( &tb );
    ts.tv_sec  += tb.time;
    ts.tv_nsec += tb.millitm * 1000000;

    int error = pthread_cond_timedwait( &_impl->cond, &_impl->mutex, &ts );
#endif
    if( error == ETIMEDOUT )
        return false;

    if( error )
        EQERROR << "pthread_cond_timedwait failed: " << strerror( error )
                << std::endl;
    return true;
}

}
}
