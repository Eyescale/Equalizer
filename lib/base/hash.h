
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_HASH_H
#define EQBASE_HASH_H

#include <eq/base/base.h>
#include <eq/base/refPtr.h>
#include <eq/base/stdExt.h>

namespace eqBase
{
    /** A hash for pointer keys. */
    template<class K, class T> class PtrHash 
#ifdef WIN32_VC
        : public stde::hash_map< K, T, stde::hash_compare< const void* > >
#else
        : public stde::hash_map< K, T, stde::hash< const void* > >
#endif
    {};

    /** A hash function for RefPtr keys. */
    template< typename T >
    struct hashRefPtr
    {
        size_t operator()( RefPtr< T > key ) const
        {
            return stde::hash< const void* >()( key.get() );
        }
    };

    /** A hash for RefPtr keys. */
    template<class K, class T> class RefPtrHash 
        : public stde::hash_map< RefPtr< K >, T, hashRefPtr< K > >
    {};
}

#endif // EQBASE_HASH_H
