 
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
template<> int32_t Atomic< int32_t >::getAndAdd( int32_t& value, const int32_t increment )
{
    return InterlockedExchangeAdd( (long*)( &value ), increment );
}

template<> int32_t Atomic< int32_t >::getAndSub( int32_t& value, const int32_t increment )
{
    return InterlockedExchangeAdd( (long*)( &value ), -increment );
}

template<> int32_t Atomic< int32_t >::incAndGet( int32_t& value )
{
    return InterlockedIncrement( (long*)( &value ));
}

template<> int32_t Atomic< int32_t >::decAndGet( int32_t& value )
{
    return InterlockedDecrement( (long*)( &value ));
}

template<> 
bool Atomic< int32_t >::compareAndSwap( int32_t* value, const int32_t expected,
                                        const int32_t newValue )
{
    return InterlockedCompareExchange( (long*)( value ), newValue, expected ) ==
           expected;
}

template<> 
bool Atomic< void* >::compareAndSwap( void** value, void* const expected,
                                      void* const newValue )
{
    return InterlockedCompareExchangePointer( value, newValue, expected ) ==
        expected;
}

#  ifdef _WIN64

template<> ssize_t
Atomic< ssize_t >::getAndAdd( ssize_t& value, const ssize_t increment )
{
    return InterlockedExchangeAdd64( &value, increment );
}

template<> ssize_t 
Atomic< ssize_t >::getAndSub( ssize_t& value, const ssize_t increment )
{
    return InterlockedExchangeAdd64( &value, -increment );
}

template<> ssize_t Atomic< ssize_t >::incAndGet( ssize_t& value )
{
    return InterlockedIncrement64( &value );
}

template<> ssize_t Atomic< ssize_t >::decAndGet( ssize_t& value )
{
    return InterlockedDecrement64( &value );
}

template<> 
bool Atomic< ssize_t >::compareAndSwap( ssize_t* value, const ssize_t expected,
                                        const ssize_t newValue )
{
    return InterlockedCompareExchange64( value, newValue, expected ) ==
           expected;
}

#  else // _WIN64

template<> ssize_t
Atomic< ssize_t >::getAndAdd( ssize_t& value, const ssize_t increment )
{
    return InterlockedExchangeAdd( &value, increment );
}

template<> ssize_t 
Atomic< ssize_t >::getAndSub( ssize_t& value, const ssize_t increment )
{
    return InterlockedExchangeAdd( &value, -increment );
}

template<> ssize_t Atomic< ssize_t >::incAndGet( ssize_t& value )
{
    return InterlockedIncrement( &value );
}

template<> ssize_t Atomic< ssize_t >::decAndGet( ssize_t& value )
{
    return InterlockedDecrement( &value );
}

template<> 
bool Atomic< ssize_t >::compareAndSwap( ssize_t* value, const ssize_t expected,
                                        const ssize_t newValue )
{
    return InterlockedCompareExchange( value, newValue, expected ) ==
           expected;
}

#  endif // else _WIN64
#endif // _MSC_VER

}
}
