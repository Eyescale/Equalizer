
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_UUID_H
#define EQBASE_UUID_H

#include <eq/base/hash.h>
#include <eq/base/log.h>

#ifdef WIN32
#  include <rpc.h>
#  ifdef __CYGWIN__
#    include <byteswap.h>
#    define _byteswap_ushort bswap_16
#    define _byteswap_ulong  bswap_32
#  endif
#elif defined (NetBSD) || defined (FreeBSD)
#  include <uuid.h>
#else
#  include <uuid/uuid.h>
#endif

namespace eq
{
namespace base
{
    /**
     * Provides a universally unique identifier.
     *
     * Not to be subclassed.
     */
    class EQ_EXPORT UUID
    {
#ifdef WIN32
    public:
        /** Data type, used for network transport. */
        typedef ::UUID Data;

        UUID( const bool generate = false )
            { generate ? UuidCreate( &_id ) : UuidCreateNil( &_id ); }
        UUID( const UUID& from ) { _id = from._id; }
        UUID( const Data& from )   { _id = from; }

        void getData( Data& data ) const { data = _id; }

        UUID& operator = ( const UUID& from )
            {
                _id = from._id;
                return *this;
            }
        
        UUID& operator = ( const std::string& from )
            {
                UuidFromString( (unsigned char*)from.c_str(), &_id );
                return *this;
            }
        
        bool operator == ( const UUID& rhs ) const
            { 
                RPC_STATUS status; 
                return ( UuidEqual( const_cast< ::UUID* >( &_id ), 
                                    const_cast< ::UUID* >( &rhs._id ),
                                    &status ) == TRUE );
            }
        bool operator != ( const UUID& rhs ) const
            { 
                RPC_STATUS status; 
                return ( UuidEqual( const_cast< ::UUID* >( &_id ), 
                                    const_cast< ::UUID* >( &rhs._id ),
                                    &status ) == FALSE );
            }

        bool operator <  ( const UUID& rhs ) const
            { 
                RPC_STATUS status; 
                return UuidCompare( const_cast< ::UUID* >( &_id ), 
                                    const_cast< ::UUID* >( &rhs._id ),
                                    &status ) < 0;
            }
        bool operator >  ( const UUID& rhs ) const
            { 
                RPC_STATUS status; 
                return UuidCompare( const_cast< ::UUID* >( &_id ), 
                                    const_cast< ::UUID* >( &rhs._id ),
                                    &status ) > 0;
            }

        bool operator ! () const 
            {
                RPC_STATUS status; 
                return ( UuidIsNil( const_cast< ::UUID* >( &_id ), &status )==TRUE);
            }

        void convertToNetwork()
            { 
                _id.Data1 = _byteswap_ulong( _id.Data1 ); 
                _id.Data2 = _byteswap_ushort( _id.Data2 );
                _id.Data3 = _byteswap_ushort( _id.Data3 );
            }
        void convertToHost() { convertToNetwork(); }

    private:
        ::UUID _id;
#else // WIN32
    public:
        /** Opaque data type, used for network transport. */
        struct Data
        {
            uuid_t id;
        };

        /** 
         * Construct a new universally unique identifier.
         *
         * If generate is set to true, a new UUID is allocated. Otherwise the
         * UUID is cleared, i.e., it is equal to UUID::ZERO.
         */
        UUID( const bool generate = false )
            { generate ? uuid_generate( _id ) : uuid_clear( _id ); }

        /** Create a copy of a universally unique identifier. */
        UUID( const UUID& from ) { uuid_copy( _id, from._id ); }

        /** Create a copy of a universally unique identifier. */
        UUID( const Data& from )   { uuid_copy( _id, from.id ); }

        /** Get the raw data for network transport. */
        void getData( Data& data ) const { uuid_copy( data.id, _id ); }

        /** Assign another universally unique identifier. */
        UUID& operator = ( const UUID& from )
            {
                uuid_copy( _id, from._id );
                return *this;
            }

        /** Assign another UUID from a string representation. */
        UUID& operator = ( const std::string& from )
            {
                uuid_parse( from.c_str(), _id );
                return *this;
            }
        
        /** @return true if the UUIDs are equal, false otherwise. */
        bool operator == ( const UUID& rhs ) const
            { return uuid_compare( _id, rhs._id ) == 0; }

        /** @return true if the UUIDs are different, false otherwise. */
        bool operator != ( const UUID& rhs ) const
            { return uuid_compare( _id, rhs._id ) != 0; }

        /** @return true if this UUID is smaller than the RHS UUID. */
        bool operator <  ( const UUID& rhs ) const
            { return uuid_compare( _id, rhs._id ) < 0; }

        /** @return true if this UUID is bigger than the RHS UUID. */
        bool operator >  ( const UUID& rhs ) const
            { return uuid_compare( _id, rhs._id ) > 0; }

        /** @return true if this UUID is set, i.e., it is not UUID::ZERO. */
        bool operator ! () const { return uuid_is_null( _id ); }

        /** Convert this UUID for network transport. */
        void convertToNetwork() {}

        /** Convert this UUID from network transport. */
        void convertToHost() {}

    private:
        uuid_t _id;
#endif // WIN32

    public:
        /** The NULL UUID. */
        static const UUID ZERO;

    private:
        friend std::ostream& operator << ( std::ostream& os, const UUID& id );
#ifdef WIN32_VC
        friend size_t stde::hash_compare< eq::base::UUID >::operator() 
            ( const eq::base::UUID& key ) const;
#else
        friend struct stde::hash< const eq::base::UUID >;
#endif
    };

    /** A hash for UUID keys. */
    template<class T> class UUIDHash 
        : public stde::hash_map< const UUID, T >
    {};

    /** UUID& ostream operator. */
    inline std::ostream& operator << ( std::ostream& os, const UUID& id )
    {
#ifdef WIN32
        unsigned char* uuid;
        UuidToString( const_cast< ::UUID* >( &id._id ), &uuid );
        os << uuid;
        RpcStringFree( &uuid );
#else
        char string[40];
        uuid_unparse( id._id, string );
        os << string;
#endif
        return os;
    }
}
}

#ifdef WIN32_VC
template<>
inline size_t stde::hash_compare< eq::base::UUID >::operator() 
    ( const eq::base::UUID& key ) const
{
    return key._id.Data1;
}

template<>
inline size_t stde::hash_value( const eq::base::UUID& key )
{
    stde::hash_compare< eq::base::UUID > hash;
    return hash( key );
}

#elif defined (WIN32)

namespace __gnu_cxx
{
    template<> 
    struct hash< const eq::base::UUID >
    {
        size_t operator()( const eq::base::UUID& key ) const
        {
            return key._id.Data1;
        }
    };
}

#else // POSIX

#  ifdef __GNUC__              // GCC 3.1 and later
namespace __gnu_cxx
#  else //  other compilers
namespace std
#  endif
{
    template<> 
    struct hash< const eq::base::UUID >
    {
        size_t operator()( const eq::base::UUID& key ) const
        {
            return (size_t)(*key._id);
        }
    };
}

#endif // WIN32_VC
#endif // EQBASE_NODE_H
