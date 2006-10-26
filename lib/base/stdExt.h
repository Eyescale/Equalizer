
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_STDEXT_H
#define EQBASE_STDEXT_H

#include <algorithm>

//----- Common extensions of the STL
#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/hash_map>
#  include <ext/hash_set>
namespace stde = ::__gnu_cxx; 
#else                        //  other compilers
#  include <hash_map>
#  include <hash_set>
namespace stde = std;
#endif


//----- Our extensions of the STL 
/** Gathers some functionality extending the STL. */
#ifdef __GNUC__              // GCC 3.1 and later
namespace __gnu_cxx
#else                        //  other compilers
namespace std
#endif
{
    /** Uniquely sorts and truncates a STL container. */
    template< typename C >
    void usort( C& c )
    {
        std::sort( c.begin(), c.end( ));
        c.erase( std::unique( c.begin(), c.end( )), c.end( ));
    }
}

#endif // EQBASE_STDEXT_H
