
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_STDEXT_H
#define COBASE_STDEXT_H

#include <co/base/compiler.h>
#include <co/base/uint128_t.h>

#include <algorithm>
#include <string>
#include <vector>

//----- Common extensions of the STL
#ifdef __GNUC__
#  if defined EQ_GCC_4_3_OR_LATER && !defined __INTEL_COMPILER
#    define CO_STDEXT_TR1
#  else
#    define CO_STDEXT_EXT
#    ifndef EQ_HAVE_LONG_HASH
#      define EQ_HAVE_LONG_HASH
#    endif
#  endif
#else
#  ifdef _MSC_VER
#    define CO_STDEXT_MSVC
#  else
#    define CO_STDEXT_STD
#  endif
#endif

#ifdef CO_STDEXT_TR1
#  include <tr1/unordered_map>
#  include <tr1/unordered_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = std::tr1;
#  define CO_STDEXT_NAMESPACE_OPEN namespace std { namespace tr1 {
#  define CO_STDEXT_NAMESPACE_CLOSE }}
#endif

#ifdef CO_STDEXT_EXT
#  include <ext/hash_map>
#  include <ext/hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = __gnu_cxx; 
#  define CO_STDEXT_NAMESPACE_OPEN namespace __gnu_cxx {
#  define CO_STDEXT_NAMESPACE_CLOSE }
#endif

#ifdef CO_STDEXT_MSVC
#  include <hash_map>
#  include <hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = stdext;
#  define CO_STDEXT_NAMESPACE_OPEN namespace stdext {
#  define CO_STDEXT_NAMESPACE_CLOSE }
#endif

#ifdef CO_STDEXT_STD
#  include <hash_map>
#  include <hash_set>
/* Alias stde namespace to uniformly access stl extensions. */
namespace stde = std;
#  define CO_STDEXT_NAMESPACE_OPEN namespace std {
#  define CO_STDEXT_NAMESPACE_CLOSE }
#endif


CO_STDEXT_NAMESPACE_OPEN

//----- Our extensions of the STL 
#ifdef CO_STDEXT_TR1
#   ifndef EQ_HAVE_HASH_MAP
    template<class K, class T, class H = hash< K >, 
             class P = std::equal_to< K >, class A = std::allocator< K > >
    class hash_map : public unordered_map< K, T, H, P, A >
    {
    };
#  endif // EQ_HAVE_HASH_MAP
#endif

#ifdef CO_STDEXT_EXT
#  ifndef EQ_HAVE_STRING_HASH
    /** std::string hash function. @version 1.0 */
    template<> struct hash< std::string >
    {
        size_t operator()( const std::string& str ) const
        {
            return hash< const char* >()( str.c_str() );
        }
    };
#  endif // EQ_HAVE_STRING_HASH

#  if !defined __INTEL_COMPILER
#    ifndef EQ_HAVE_LONG_HASH
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
#  endif // !__INTEL_COMPILER

#  ifndef EQ_HAVE_VOID_PTR_HASH
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
#  endif // EQ_HAVE_VOID_PTR_HASH
#endif // CO_STDEXT_EXT

#ifdef CO_STDEXT_MSVC
#  ifndef EQ_HAVE_STRING_HASH

    /** std::string hash function. @version 1.0 */
    template<> inline size_t hash_compare< std::string >::operator()
        ( const std::string& key ) const
    {
        return hash_value( key.c_str( ));
    }

#  endif

    template<> inline size_t hash_compare< co::base::uint128_t >::operator() 
        ( const co::base::uint128_t& key ) const
    {
        return static_cast< size_t >( key.high() ^ key.low() );
    }

    template<> inline size_t hash_value( const co::base::uint128_t& key )
    {
        return static_cast< size_t >( key.high() ^ key.low() );
    }

#else // MSVC

    template<> struct hash< co::base::uint128_t >
    {
        size_t operator()( const co::base::uint128_t& key ) const
            {
                return key.high() ^ key.low();
            }
    };

#endif //! MSVC

    /**
     * Uniquely sorts and eliminates duplicates in a STL container.
     * @version 1.0
     */
    template< typename C > void usort( C& c )
    {
        std::sort( c.begin(), c.end( ));
        c.erase( std::unique( c.begin(), c.end( )), c.end( ));
    }

    /** Find the element in the given vector. @version 1.0 */
    template< typename T > typename std::vector< T >::iterator 
    find( std::vector< T >& container, const T& element )
        { return std::find( container.begin(), container.end(), element ); }

    /** Find the element in the given vector. @version 1.0 */
    template< typename T > typename std::vector< T >::const_iterator 
    find( const std::vector< T >& container, const T& element )
        { return std::find( container.begin(), container.end(), element ); }

    /** Find the element matching the predicate @version 1.0 */
    template< typename T, typename P > typename std::vector< T >::iterator 
    find_if( std::vector< T >& container, const P& predicate )
        { return std::find_if( container.begin(), container.end(), predicate );}

    /** Find the element matching the predicate @version 1.0 */
    template< typename T, typename P > typename std::vector<T>::const_iterator 
    find_if( std::vector< const T >& container, const P& predicate )
        { return std::find_if( container.begin(), container.end(), predicate );}

CO_STDEXT_NAMESPACE_CLOSE

#endif // COBASE_STDEXT_H
