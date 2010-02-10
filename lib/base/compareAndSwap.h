
//  Copyright (C) 2007, 2008 Tim Blechmann & Thomas Grill
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  Disclaimer: Not a Boost library.

/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   Modifications to use within eq::base namespace and naming conventions.
   Original at http://tim.klingt.org/git?p=boost_lockfree.git;a=tree
*/

#ifndef EQBASE_COMPAREANDSWAP_H
#define EQBASE_COMPAREANDSWAP_H

#include <eq/base/base.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#  include <intrin.h>
#  pragma intrinsic(_ReadWriteBarrier)
#endif

#ifdef Darwin
#  include <libkern/OSAtomic.h>
#endif

namespace eq
{
namespace base
{

#if (defined(__GNUC__) && \
    ( (__GNUC__ > 4) || ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)) )) || \
    defined(_MSC_VER) || defined(_WIN32) || defined(__APPLE__) || \
    defined(AO_HAVE_compare_and_swap_full)
#  define EQ_HAS_COMPARE_AND_SWAP

/** Perform a memory barrier (atomic operations) */
inline void memoryBarrier()
{
#if defined(__GNUC__) && \
    ( (__GNUC__ > 4) || ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)) )
    __sync_synchronize();
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
    _ReadWriteBarrier();
#elif defined(__APPLE__)
    OSMemoryBarrier();
#elif defined(AO_HAVE_nop_full)
    AO_nop_full();
#elif defined( __PPC__ )
    asm volatile("sync":::"memory");
#elif defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || \
      defined( __i686__ ) || defined( __x86_64__ )
    asm volatile("mfence":::"memory");
#else
#   warning "no memory barrier implemented for this platform"
#endif
}

/** 
 * Atomically replace the value at addr and return true if the value at addr
 * matched old.
 * @version 1.0
 */
template <class D>
inline bool compareAndSwap(volatile long * addr, D old, D nw)
{
#if defined(__GNUC__) && \
    ( (__GNUC__ > 4) || ((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)) )
    return __sync_bool_compare_and_swap(addr, old, nw);
#elif defined(_MSC_VER)
    return _InterlockedCompareExchange(addr,nw,old) == old;
#elif defined(_WIN32)
    return InterlockedCompareExchange(addr,nw,old) == old;
#elif defined(__APPLE__)
    return OSAtomicCompareAndSwap32((int32_t) old, (int32_t)nw, (int32_t*)addr);
#elif defined(AO_HAVE_compare_and_swap_full)
    return AO_compare_and_swap_full(reinterpret_cast<volatile AO_t*>(addr),
                                    reinterpret_cast<AO_t>(old),
                                    reinterpret_cast<AO_t>(nw));
#else
#  warning ("CompareAndSwap emulation")
#  include <eq/base/lock.h>           // used in inline function
#  include <eq/base/scopedMutex.h>    // used in inline function

    static Lock guard;
    ScopedMutex lock(guard);

    if (*addr == old)
    {
        *addr = nw;
        return true;
    }
    else
        return false;
#endif
}

#endif
}

}
#endif // EQBASE_COMPAREANDSWAP_H
