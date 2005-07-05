
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_HASH_H
#define EQBASE_HASH_H

#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/hash_map>
namespace Sgi = ::__gnu_cxx; 
#else                        //  other compilers
#  include <hash_map>
namespace Sgi = std;
#endif

namespace eqBase
{
    /** 
     * Provides a hashing function for pointers.
     */
    template< class T > struct hashFuncPtr
    {

        /** 
         * A hashing function for pointers.
         * 
         * @return the hash value of the pointer.
         */
        size_t operator()(const T & N) const
            {  
                return ((size_t)N);
            }
    };

    /** A hash for pointer keys. */
    template<class K, class T> class PtrHash 
        : public Sgi::hash_map<K, T, hashFuncPtr<K> >
    {};
}

#endif // EQBASE_HASH_H
