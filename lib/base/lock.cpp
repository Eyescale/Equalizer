
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
    EQVERB << "New Lock @" << (void*)this << endl;
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
