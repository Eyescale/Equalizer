
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "sema.h"

#include <eq/base/log.h>

#include <errno.h>
#include <string.h>
#include <pthread.h>

using namespace std;

namespace eq
{
namespace base
{
class SemaPrivate
{
public:
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
};

Sema::Sema()
        : _data( new SemaPrivate )
        , _value( 0 )
{
    int error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " << strerror( error )
                << endl;
        return;
    } 
            
    error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    } 
}

Sema::~Sema()
{
    pthread_cond_destroy( &_data->cond );
    pthread_mutex_destroy( &_data->mutex );
    delete _data;
    _data = 0;
}

void Sema::post()
{
    pthread_mutex_lock( &_data->mutex );
    ++_value;
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

void Sema::wait()
{
    pthread_mutex_lock( &_data->mutex );
    while( _value == 0 )
        pthread_cond_wait( &_data->cond, &_data->mutex );

    --_value;
    pthread_mutex_unlock( &_data->mutex );
}

void Sema::adjust( const int delta )
{
    if( delta == 0 )
        return;

    pthread_mutex_lock( &_data->mutex );
    
    if( delta > 0 )
    {
        _value += delta;
        pthread_cond_broadcast( &_data->cond );
        pthread_mutex_unlock( &_data->mutex );
        return;
    };

    uint32_t amount = (uint32_t)-delta;
    while( amount )
    {
        while( _value == 0 )
            pthread_cond_wait( &_data->cond, &_data->mutex );
                
        if( _value < amount )
        {
            amount -= _value;
            _value = 0;
        }
        else
        {
            _value -= amount;
            amount = 0;
        }
    }
    pthread_mutex_unlock( &_data->mutex );
}
}
}
