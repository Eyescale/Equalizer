
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
    template< class T > struct hashFuncPtr
    {
        size_t operator()(const T & N) const
            {  
                return ((size_t)N);
            }
    };

    template<class K, class T> class PtrHash 
        : public Sgi::hash_map<K, T, hashFuncPtr<K> >
    {};
}

#endif // EQBASE_HASH_H
