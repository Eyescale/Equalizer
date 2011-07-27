
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

#include "condition.h"

#include <co/base/global.h>

#include <cstring>
#include <errno.h>
#include <pthread.h>
#include <sys/timeb.h>

#ifdef WIN32_API
#  define timeb _timeb
#  define ftime _ftime
#endif

namespace co
{
namespace base
{

class ConditionPrivate
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

Condition::Condition()
        : _data( new ConditionPrivate )
{
    // mutex init
    int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror( error )
                << std::endl;
        return;
    }
    // condvar init
    error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " << strerror( error )
                << std::endl;
        return;
    }
}

Condition::~Condition()
{
    int error = pthread_mutex_destroy( &_data->mutex );
    if( error )
        EQERROR << "Error destroying pthread mutex: " << strerror( error )
                << std::endl;

    error = pthread_cond_destroy( &_data->cond );
    if( error )
        EQERROR << "Error destroying pthread condition: " << strerror( error )
                << std::endl;

    delete _data;
}

void Condition::lock()
{
    pthread_mutex_lock( &_data->mutex );
}

void Condition::signal()
{
    pthread_cond_signal( &_data->cond );
}

void Condition::broadcast()
{
    pthread_cond_broadcast( &_data->cond );
}

void Condition::unlock()
{
    pthread_mutex_unlock( &_data->mutex );
}

void Condition::wait()
{
    pthread_cond_wait( &_data->cond, &_data->mutex );
}

bool Condition::timedWait( const uint32_t timeout )
{
    if( timeout == EQ_TIMEOUT_INDEFINITE )
    {
        wait();
        return true;
    }

    timespec ts = { 0, 0 };
    const uint32_t time = timeout == EQ_TIMEOUT_DEFAULT ?
        Global::getIAttribute( Global::IATTR_TIMEOUT_DEFAULT ) : timeout;

    if( time > 0 )
    {
        ts.tv_sec  = static_cast<int>( time / 1000 );
        ts.tv_nsec = ( time - ts.tv_sec * 1000 ) * 1000000;
    }

    timeb tb;
    ftime( &tb );
    ts.tv_sec  += tb.time;
    ts.tv_nsec += tb.millitm * 1000000;

    int error = pthread_cond_timedwait( &_data->cond, &_data->mutex, &ts );
    if( error == ETIMEDOUT )
        return false;

    if( error )
        EQERROR << "pthread_cond_timedwait failed: " << strerror( error )
                << std::endl;
    return true;
}

}
}
