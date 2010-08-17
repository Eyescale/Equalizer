
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTVERSION_H
#define EQNET_OBJECTVERSION_H

#include <eq/base/base.h>
#include <eq/base/stdExt.h>

#include <iostream>

namespace eq
{
namespace net
{
    class Object;

    /**
     * A helper struct bundling an object identifier and version.
     *
     * The struct either contains the object's identifier and version (if it is
     * registered or mapped), EQ_ID_INVALID and VERSION_NONE if it is unmapped
     * or EQ_ID_NONE and VERSION_NONE if no object was given.
     */
    struct ObjectVersion
    {
        EQ_EXPORT ObjectVersion();
        EQ_EXPORT ObjectVersion( const uint32_t identifier,
                                 const uint32_t version );
        EQ_EXPORT ObjectVersion( const Object* object );
        EQ_EXPORT ObjectVersion& operator = ( const Object* object );
        bool operator == ( const ObjectVersion& value ) const
            {
                return ( identifier == value.identifier &&
                         version == value.version );
            }
        
        bool operator != ( const ObjectVersion& value ) const
            {
                return ( identifier != value.identifier ||
                         version != value.version );
            }
        
        bool operator < ( const ObjectVersion& rhs ) const
            { 
                return identifier < rhs.identifier ||
                    ( identifier == rhs.identifier && version < rhs.version );
            }

        bool operator > ( const ObjectVersion& rhs ) const
            {
                return identifier > rhs.identifier || 
                    ( identifier == rhs.identifier && version > rhs.version );
            }

        uint32_t identifier;
        uint32_t version;

        /** An unset object version. */
        static ObjectVersion NONE;
    };

    inline std::ostream& operator << (std::ostream& os, const ObjectVersion& ov)
    {
        os << "id " << ov.identifier << " v" << ov.version;
        return os;
    }

}
}

EQ_STDEXT_NAMESPACE_OPEN
#ifdef EQ_STDEXT_VC8
    /** ObjectVersion hash function. */
    template<>
    inline size_t hash_compare< eq::net::ObjectVersion >::operator()
        ( const eq::net::ObjectVersion& key ) const
    {
        return hash_value(
            (static_cast< uint64_t >( key.identifier ) << 32) + key.version );
    }
#else
    /** ObjectVersion hash function. */
    template<> struct hash< eq::net::ObjectVersion >
    {
        template< typename P > size_t operator()( const P& key ) const
        {
            return hash< uint64_t >()(
                (static_cast<uint64_t>( key.identifier ) << 32) + key.version);
        }
    };
#endif
EQ_STDEXT_NAMESPACE_CLOSE

#endif // EQNET_OBJECT_H
