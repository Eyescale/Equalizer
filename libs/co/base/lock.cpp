
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
namespace detail
{
class Lock
{
public:
#ifdef _WIN32
    CRITICAL_SECTION cs; 
#else
    pthread_mutex_t mutex;
#endif
};
}

Lock::Lock()
        : _impl ( new detail::Lock )
{
#ifdef _WIN32
    InitializeCriticalSection( &_impl->cs );
#else
    const int error = pthread_mutex_init( &_impl->mutex, 0 );
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
    DeleteCriticalSection( &_impl->cs ); 
#else
    pthread_mutex_destroy( &_impl->mutex );
#endif
    delete _impl;
}

void Lock::set()
{
#ifdef _WIN32
    EnterCriticalSection( &_impl->cs );
#else
    pthread_mutex_lock( &_impl->mutex );
#endif
}

void Lock::unset()
{
#ifdef _WIN32
    LeaveCriticalSection( &_impl->cs );
#else
    pthread_mutex_unlock( &_impl->mutex );
#endif
}

bool Lock::trySet()
{
#ifdef _WIN32
    return TryEnterCriticalSection( &_impl->cs );
#else
    return ( pthread_mutex_trylock( &_impl->mutex ) == 0 );
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
