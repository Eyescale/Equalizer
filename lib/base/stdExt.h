
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
#  include <ext/hash_map>
#  include <ext/hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = __gnu_cxx; 
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

#    ifndef EQ_HAVE_VOID_PTR_HASH
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
#    endif

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

    /** Uniquely sorts and eliminates duplicates in a STL container. */
    template< typename C >
    void usort( C& c )
    {
        std::sort( c.begin(), c.end( ));
        c.erase( std::unique( c.begin(), c.end( )), c.end( ));
    }
}

#endif // EQBASE_STDEXT_H
