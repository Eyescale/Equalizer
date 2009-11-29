
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file stdExt.h
 *
 * Include extensions to the STL and define a uniform interface to them.
 */

#ifndef EQBASE_STDEXT_H
#define EQBASE_STDEXT_H

#include <algorithm>
#include <string>

//----- Common extensions of the STL
#ifdef __GNUC__              // GCC 3.1 and later
#  ifdef EQ_GCC_4_2_OR_LATER
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = std::tr1;
#  else
#    include <ext/hash_map>
#    include <ext/hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = __gnu_cxx; 
#  endif
#else                        //  other compilers
#  include <hash_map>
#  include <hash_set>
#  ifdef WIN32
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = stdext;
#  else
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = std;
#  endif
#endif


//----- Our extensions of the STL 
#ifdef __GNUC__              // GCC 3.1 and later
#  ifdef EQ_GCC_4_2_OR_LATER
namespace std { namespace tr1
#  else
namespace __gnu_cxx
#  endif
#elif defined (WIN32)
namespace stdext
#else //  other compilers
namespace std
#endif
{
#  ifdef __GNUC__
#    ifdef EQ_GCC_4_2_OR_LATER
    template<class K, class T, class H = hash< K >, 
             class P = std::equal_to< K >, class A = std::allocator< K > >
    class hash_map : public unordered_map< K, T, H, P, A >
    {
    };
#    else
#      ifndef EQ_HAVE_STRING_HASH
    /** std::string hash function. @version 1.0 */
    template<> struct hash< std::string >
    {
        size_t operator()( const std::string& str ) const
        {
            return hash< const char* >()( str.c_str() );
        }
    };
#      endif

    /** uint64_t hash function. @version 1.0 */
    template<> struct hash< uint64_t >
    {
        size_t operator()( const uint64_t& val ) const
        {
            // OPT: tr1 does the same, however it seems suboptimal on 32 bits
            // if the lower 32 bits never change, e.g., for ObjectVersion
            return static_cast< size_t >( val );
        }
    };

#    endif

#    ifndef EQ_HAVE_VOID_PTR_HASH
    /** void* hash functions. @version 1.0 */
    template<> struct hash< void* >
    {
        template< typename P >
        size_t operator()( const P& key ) const
        {
            return reinterpret_cast<size_t>(key);
        }
    };

    template<> struct hash< const void* >
    {
        template< typename P >
        size_t operator()( const P& key ) const
        {
            return reinterpret_cast<size_t>(key);
        }
    };
#    endif // EQ_HAVE_VOID_PTR_HASH

#  else // WIN32
#    ifndef EQ_HAVE_STRING_HASH

    /** std::string hash function. @version 1.0 */
    template<> inline size_t hash_compare< std::string >::operator()
        ( const std::string& key ) const
    {
        return hash_value( key.c_str( ));
    }

#    endif
#  endif

    /**
     * Uniquely sorts and eliminates duplicates in a STL container.
     * @version 1.0
     */
    template< typename C > void usort( C& c )
    {
        std::sort( c.begin(), c.end( ));
        c.erase( std::unique( c.begin(), c.end( )), c.end( ));
    }
#ifdef EQ_GCC_4_2_OR_LATER
}
#endif
}

#endif // EQBASE_STDEXT_H
