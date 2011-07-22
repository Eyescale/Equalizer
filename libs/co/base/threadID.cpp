
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

#include "threadID.h"

#include "log.h"

#include <pthread.h>
#include <cstring>   // for memset

namespace co
{
namespace base
{

struct ThreadIDPrivate
{
    pthread_t pthread;
};

const ThreadID ThreadID::ZERO;

ThreadID::ThreadID()
        : _data( new ThreadIDPrivate )
{
    memset( &_data->pthread, 0, sizeof( pthread_t ));
}

ThreadID::ThreadID( const ThreadID& from )
        : _data( new ThreadIDPrivate )
{
    _data->pthread = from._data->pthread;
}

ThreadID::~ThreadID()
{
    delete _data;
}

ThreadID& ThreadID::operator = ( const ThreadID& from )
{
    _data->pthread = from._data->pthread;
    return *this;
}

bool ThreadID::operator == ( const ThreadID& rhs ) const
{
    return pthread_equal( _data->pthread, rhs._data->pthread );
}

bool ThreadID::operator != ( const ThreadID& rhs ) const
{
    return !pthread_equal( _data->pthread, rhs._data->pthread );
}

std::ostream& operator << ( std::ostream& os, const ThreadID& threadID )
{
#ifdef PTW32_VERSION
    os << threadID._data->pthread.p;
#else
    os << threadID._data->pthread;
#endif
    return os;
}

}
}
