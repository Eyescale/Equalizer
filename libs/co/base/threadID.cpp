
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

#include "threadID.h"

#include "log.h"

#include <pthread.h>
#include <cstring>   // for memset

namespace co
{
namespace base
{
namespace detail
{
class ThreadID
{
public:
    pthread_t pthread;
};
}

const ThreadID ThreadID::ZERO;

ThreadID::ThreadID()
        : _impl( new detail::ThreadID )
{
    memset( &_impl->pthread, 0, sizeof( pthread_t ));
}

ThreadID::ThreadID( const ThreadID& from )
        : _impl( new detail::ThreadID )
{
    _impl->pthread = from._impl->pthread;
}

ThreadID::~ThreadID()
{
    delete _impl;
}

ThreadID& ThreadID::operator = ( const ThreadID& from )
{
    _impl->pthread = from._impl->pthread;
    return *this;
}

bool ThreadID::operator == ( const ThreadID& rhs ) const
{
    return pthread_equal( _impl->pthread, rhs._impl->pthread );
}

bool ThreadID::operator != ( const ThreadID& rhs ) const
{
    return !pthread_equal( _impl->pthread, rhs._impl->pthread );
}

std::ostream& operator << ( std::ostream& os, const ThreadID& threadID )
{
#ifdef PTW32_VERSION
    os << threadID._impl->pthread.p;
#else
    os << threadID._impl->pthread;
#endif
    return os;
}

}
}
