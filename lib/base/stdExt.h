
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_STDEXT_H
#define EQBASE_STDEXT_H

#include <algorithm>
#include <string>

//----- Common extensions of the STL
#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/hash_map>
#  include <ext/hash_set>
namespace stde = __gnu_cxx; 
#else                        //  other compilers
#  include <hash_map>
#  include <hash_set>
#  ifdef WIN32
namespace stde = stdext;
#  else
namespace stde = std;
#  endif
#endif


//----- Our extensions of the STL 
/** Gathers some functionality extending the STL. */
#ifdef __GNUC__              // GCC 3.1 and later
namespace __gnu_cxx
#elif defined (WIN32)
namespace stdext
#else //  other compilers
namespace std
#endif
{
#  ifdef __GNUC__
#    ifndef EQ_HAVE_STRING_HASH
    /** std::string hash function. */
    template<> 
    struct hash< std::string >
    {
        size_t operator()( const std::string& str ) const
        {
            return hash< const char* >()( str.c_str() );
        }
    };
#    endif

    /** void* hash functions. */
    template<> 
    struct hash< void* >
    {
        template< typename P >
        size_t operator()( const P& key ) const
        {
            return reinterpret_cast<size_t>(key);	 
        }
    };
    template<> 
    struct hash< const void* >
    {
        template< typename P >
        size_t operator()( const P& key ) const
        {
            return reinterpret_cast<size_t>(key);	 
        }
    };

#  else // WIN32
#    ifndef EQ_HAVE_STRING_HASH

    /** std::string hash function. */
    template<>
    inline size_t hash_compare< std::string >::operator()
        ( const std::string& key ) const
    {
        return hash_value( key.c_str( ));
    }

#    endif
#  endif

    /** Uniquely sorts and truncates a STL container. */
    template< typename C >
    void usort( C& c )
    {
        std::sort( c.begin(), c.end( ));
        c.erase( std::unique( c.begin(), c.end( )), c.end( ));
    }
}

#endif // EQBASE_STDEXT_H
