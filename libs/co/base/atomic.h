
//  Copyright (C) 2007, 2008 Tim Blechmann & Thomas Grill
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com> 
   Modifications to use within co::base namespace and naming conventions.
   Original at http://tim.klingt.org/git?p=boost_lockfree.git;a=tree
*/

#ifndef COBASE_ATOMIC_H
#define COBASE_ATOMIC_H

#include <co/base/api.h>
#include <co/base/compiler.h>       // GCC version
#include <co/base/types.h>

#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4985) // inconsistent decl of ceil
#    include <math.h> // include math.h early to avoid warning later
#    include <intrin.h>
#  pragma warning (pop)
#  pragma intrinsic(_ReadWriteBarrier)
#endif

namespace co
{
namespace base
{

/** Perform a full memory barrier. */
inline void memoryBarrier()
{
#ifdef EQ_GCC_4_1_OR_LATER
    __sync_synchronize();
#elif defined(_MSC_VER)
    _ReadWriteBarrier();
#else
#  error "no memory barrier implemented for this platform"
#endif
}

/**
 * A variable with atomic semantics and standalone atomic operations.
 *
 * Atomic variables can be modified safely from multiple threads
 * concurrently. They are useful to implement lock-free algorithms.
 *
 * For implementation reasons, only signed atomic variables are supported, of
 * which only int32_t is implemented right now.
 */
template< class T > class Atomic
{
public:
    /** @return the old value, then add the given increment. */
    COBASE_API static T getAndAdd( T& value, const T increment );

    /** @return the old value, then substract the increment. */
    COBASE_API static T getAndSub( T& value, const T increment );

    /** @return the new value after adding the given increment. */
    static T addAndGet( T& value, const T increment );

    /** @return the new value after substracting the increment. */
    static T subAndGet( T& value, const T increment );

    /** @return the new value after incrementing the value. */
    COBASE_API static T incAndGet( T& value );

    /** @return the new value after decrementing the value. */
    COBASE_API static T decAndGet( T& value );

    /** Perform a compare-and-swap atomic operation. */
    COBASE_API static bool compareAndSwap( T* value, const T expected,
                                           const T newValue );

    /** Construct a new atomic variable with an initial value. @version 1.0 */
    explicit Atomic( const T v = 0 );

    /** Construct a copy of an atomic variable. Not thread-safe! @version 1.0 */
    Atomic( const Atomic< T >& v );

    /** @return the current value @version 1.0 */
    operator T(void) const;

    /** Assign a new value @version 1.0 */
    void operator = ( const T v );

    /** Assign a new value. Not thread-safe! @version 1.0 */
    void operator = ( const Atomic< T >& v);

    /** Atomically add a value and return the new value. @version 1.0 */
    T operator +=(T v);

    /** Atomically substract a value and return the new value. @version 1.0 */
    T operator -=(T v);

    /** Atomically increment by one and return the new value. @version 1.0 */
    T operator ++(void);

    /** Atomically decrement by one and return the new value. @version 1.0 */
    T operator --(void);

    /** Atomically increment by one and return the old value. @version 1.0 */
    T operator ++(int);

    /** Atomically decrement by one and return the old value. @version 1.0 */
    T operator --(int);

    /** @return true if the variable has the given value. @version 1.1.2 */
    bool operator == ( const Atomic< T >& rhs ) const;

    /** @return true if the variable has not the given value. @version 1.1.2 */
    bool operator != ( const Atomic< T >& rhs ) const;

    /**
     * Perform a compare-and-swap atomic operation.
     *
     * Atomically replaces the value and return true if the value matched the
     * expected.
     * @return true if the new value was set, false otherwise
     * @version 1.1.2
     */
    bool compareAndSwap( const T expected, const T newValue );

private:
    mutable T _value;
};

typedef Atomic< int32_t > a_int32_t; //!< An atomic 32 bit integer variable
typedef Atomic< ssize_t > a_ssize_t; //!< An atomic signed size variable

// Implementation
#ifdef EQ_GCC_4_1_OR_LATER
template< class T > T Atomic< T >::getAndAdd( T& value, const T increment )
{
    return __sync_fetch_and_add( &value, increment );
}

template< class T > T Atomic< T >::getAndSub( T& value, const T increment )
{
    return __sync_fetch_and_sub( &value, increment );
}

template< class T > T Atomic< T >::addAndGet( T& value, const T increment )
{
    return __sync_add_and_fetch( &value, increment );
}

template< class T > T Atomic< T >::subAndGet( T& value, const T increment )
{
    return __sync_sub_and_fetch( &value, increment );
}

template< class T > T Atomic< T >::incAndGet( T& value )
{
    return addAndGet( value, 1 );
}

template< class T > T Atomic< T >::decAndGet( T& value )
{
    return subAndGet( value, 1 );
}

template< class T > 
bool Atomic< T >::compareAndSwap( T* value, const T expected, const T newValue )
{
    return __sync_bool_compare_and_swap( value, expected, newValue );
}

#elif defined (_MSC_VER)

// see also atomic.cpp
template< class T > T Atomic< T >::addAndGet( T& value, const T increment )
{
    return getAndAdd( value, increment ) + increment;
}

template< class T > T Atomic< T >::subAndGet( T& value, const T increment )
{
    return getAndSub( value, increment ) - increment;
}

#else
#  error No Atomic Support - consider compareAndSwap-based implementation?
#endif

template< class T > Atomic< T >::Atomic ( const T v ) : _value(v) {}

template <class T>
Atomic< T >::Atomic( const Atomic< T >& v ) : _value( v._value ) {}

template <class T>
Atomic< T >::operator T(void) const
{
    return getAndAdd( _value, 0 );
}

template< class T > void Atomic< T >::operator = ( const T v )
{
    _value = v;
    memoryBarrier();
}

template< class T > void Atomic< T >::operator = ( const Atomic< T >& v)
{
    _value = v._value;
    memoryBarrier();
}

template< class T >  T Atomic< T >::operator += (T v)
{
    return addAndGet( _value, v );
}

template< class T > T Atomic< T >::operator -=(T v)
{
    return subAndGet( _value, v );
}

template< class T > T Atomic< T >::operator ++(void)
{
    return incAndGet( _value );
}

template< class T > T Atomic< T >::operator --(void)
{
    return decAndGet( _value );
}

template< class T > T Atomic< T >::operator ++(int)
{
    return getAndAdd( _value, 1 );
}

template< class T > T Atomic< T >::operator --(int)
{
    return getAndSub( _value, 1 );
}

template< class T > bool Atomic< T >::operator == ( const Atomic<T>& rhs ) const
{
    memoryBarrier();
    return _value == rhs._value;
}

template< class T > bool Atomic< T >::operator != ( const Atomic<T>& rhs ) const
{
    memoryBarrier();
    return _value != rhs._value;
}

template< class T >
bool Atomic< T >::compareAndSwap( const T expected, const T newValue )
{
    return compareAndSwap( &_value, expected, newValue );
}

}
}
#endif  // COBASE_ATOMIC_H
