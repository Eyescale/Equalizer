
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_HASH_H
#define EQBASE_HASH_H

#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/hash_map>
#  include <ext/hash_set>
namespace Sgi = ::__gnu_cxx; 
#else                        //  other compilers
#  include <hash_map>
#  include <hash_set>
namespace Sgi = std;
#endif

namespace eqBase
{
    /** 
     * Provides a hashing function for pointers.
     */
    template< class T > struct hashPtr
    {

        /** 
         * A hashing function for pointers.
         * 
         * @return the hash value of the pointer.
         */
        size_t operator()(const T& N) const
            {  
                return ((size_t)N);
            }
    };

    struct hashString : public Sgi::hash<const char*>
    {
        bool operator()(const char* s1, const char* s2) const
            {
                return strcmp(s1, s2) == 0;
            }
    };
    
    /** A hash for pointer keys. */
    template<class K, class T> class PtrHash 
        : public Sgi::hash_map<K, T, hashPtr<K> >
    {};

    /** A hash for const char* keys. */
    template<class T> class StringHash
        : public Sgi::hash_map<const char*, T, hashString, hashString >
    {};

}

#endif // EQBASE_HASH_H
