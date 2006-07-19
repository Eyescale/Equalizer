
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

 // good idea?
#ifdef __GNUC__              // GCC 3.1 and later
namespace __gnu_cxx
#else
namespace std
#endif
{
    /** Provides a hashing function for std::string. */
    template<> struct hash<std::string>
    {
        size_t operator()(const std::string& string) const
            {  
                return __stl_hash_string( string.c_str( ));
            }
    };
}

namespace eqBase
{
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
        : public Sgi::hash_map<K, T, hashPtr<K> >
    {};
}

#endif // EQBASE_HASH_H
