
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#include <pthread.h>
#include <cstring>

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
    pthread_mutex_destroy( &_data->mutex );
    pthread_cond_destroy( &_data->cond );
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

}
}
