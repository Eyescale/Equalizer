
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_HASH_H
#define EQBASE_HASH_H

#include <eq/base/stdExt.h>

namespace eqBase
{
    /** Provides a hashing function for std::string. */
    struct hashString
    {
        size_t operator()(const std::string& string) const
            {  
                return stde::__stl_hash_string( string.c_str( ));
            }
    };
    /** A hash for std::string keys. */
    template<class T> class StringHash 
        : public stde::hash_map<std::string, T, hashString >
    {};

    /** Provides a hashing function for pointers. */
    template< class T > struct hashPtr
    {
        size_t operator()(const T& N) const
            {  
                return ((size_t)N);
            }
    };
    /** A hash for pointer keys. */
    template<class K, class T> class PtrHash 
        : public stde::hash_map<K, T, hashPtr<K> >
    {};
}

#endif // EQBASE_HASH_H
