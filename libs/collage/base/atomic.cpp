 
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include "atomic.h"
#include "os.h"

namespace co
{
namespace base
{

#ifdef _MSC_VER
template<> bool Atomic< long >::compareAndSwap( const long expected,
                                                const long newValue )
{
    return InterlockedCompareExchange( &_value, newValue, expected ) == expected;
}

template<>
bool Atomic< long long >::compareAndSwap( const long long expected,
                                          const long long newValue )
{
    return
        InterlockedCompareExchange64( &_value, newValue, expected ) == expected;
}

template<> long Atomic< long >::getAndAdd( long& value, const long increment )
{
    return InterLockedExchangeAdd( &value, increment );
}

template<> long long 
Atomic< long long >::getAndAdd( long long& value, const long long increment )
{
    return InterLockedExchangeAdd64( &value, increment );
}

template<> long Atomic< long >::getAndSub( long& value, const long increment )
{
    return InterLockedExchangeAdd( &value, -increment );
}

template<> long long 
Atomic< long long >::getAndSub( long long& value, const long long increment )
{
    return InterLockedExchangeAdd64( &value, -increment );
}

template<> T Atomic< long >::incAndGet( long& value )
{
    return InterlockedIncrement( &value );
}
template<> T Atomic< long long >::incAndGet( long long& value )
{
    return InterlockedIncrement64( &value );
}

template<> T Atomic< long >::decAndGet( long& value )
{
    return InterlockedDecrement( &value );
}

template<> T Atomic< long long >::decAndGet( long long& value )
{
    return InterlockedDecrement64( &value );
}
#endif

}
}
