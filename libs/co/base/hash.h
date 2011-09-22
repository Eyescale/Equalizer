
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef COBASE_HASH_H
#define COBASE_HASH_H

#include <co/base/refPtr.h>
#include <co/base/stdExt.h>
#include <co/base/uuid.h>

namespace co
{
namespace base
{
    /** A hash for pointer keys. @version 1.0 */
    template<class K, class T> class PtrHash 
#ifdef _MSC_VER
        : public stde::hash_map< K, T, stde::hash_compare< const void* > >
#else
        : public stde::hash_map< K, T, stde::hash< const void* > >
#endif
    {};

    /** A hash function for RefPtr keys. @version 1.0 */
#ifdef _MSC_VER
    template< typename T >
    class hashRefPtr : public stde::hash_compare< RefPtr< T > >
    {
    public:
        size_t operator() ( const RefPtr< T >& key ) const
        {
            stde::hash_compare< const void* > comp;
            return comp( key.get( ));
        }

        bool operator() ( const RefPtr< T >& k1, const RefPtr< T >& k2 ) const
        {
            return k1 < k2;
        }
    };

    template< class K, class T > class RefPtrHash 
        : public stde::hash_map< RefPtr< K >, T, hashRefPtr< K > >
    {};

#else
    template< typename T > struct hashRefPtr
    {
        size_t operator()( RefPtr< T > key ) const
        {
            return stde::hash< const void* >()( key.get() );
        }
    };

    /** A hash for RefPtr keys. @version 1.0 */
    template< class K, class T > class RefPtrHash 
        : public stde::hash_map< RefPtr< K >, T, hashRefPtr< K > >
    {};
#endif

/** A hash for UUID keys. @version 1.0 */
template<class T> class UUIDHash : public stde::hash_map<co::base::UUID, T> {};

}

}
#endif // COBASE_HASH_H
