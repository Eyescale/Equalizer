
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "lock.h"

#include "log.h"
#include "os.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>

namespace co
{
namespace base
{
class LockPrivate
{
public:
#ifdef _WIN32
    CRITICAL_SECTION cs; 
#else
    pthread_mutex_t mutex;
#endif
};

Lock::Lock()
        : _data ( new LockPrivate )
{
#ifdef _WIN32
    InitializeCriticalSection( &_data->cs );
#else
    const int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: "
                << strerror(error) << std::endl;
        return;
    }
#endif
}

Lock::~Lock()
{
#ifdef _WIN32
    DeleteCriticalSection( &_data->cs ); 
#else
    pthread_mutex_destroy( &_data->mutex );
#endif
    delete _data;
    _data = 0;
}

void Lock::set()
{
#ifdef _WIN32
    EnterCriticalSection( &_data->cs );
#else
    pthread_mutex_lock( &_data->mutex );
#endif
}

void Lock::unset()
{
#ifdef _WIN32
    LeaveCriticalSection( &_data->cs );
#else
    pthread_mutex_unlock( &_data->mutex );
#endif
}

bool Lock::trySet()
{
#ifdef _WIN32
    return TryEnterCriticalSection( &_data->cs );
#else
    return ( pthread_mutex_trylock( &_data->mutex ) == 0 );
#endif
}

bool Lock::isSet()
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
