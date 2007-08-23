
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODEID_H
#define EQNET_NODEID_H

#include <eq/base/hash.h>
#include <eq/base/log.h>

#ifdef WIN32
#  include <rpc.h>
#elif defined (NetBSD) || defined (FreeBSD)
#  include <uuid.h>
#else
#  include <uuid/uuid.h>
#endif

namespace eqNet
{

    /**
     * Wraps the universally unique node identifier.
     *
     * Not to be subclassed.
     */
    class EQ_EXPORT NodeID
    {
#ifdef WIN32
    public:
        /** Data type, used for network transport. */
        typedef UUID Data;

        NodeID( const bool generate = false )
            { generate ? UuidCreate( &_id ) : UuidCreateNil( &_id ); }
        NodeID( const NodeID& from ) { _id = from._id; }
        NodeID( const Data& from )   { _id = from; }

        void getData( Data& data ) const { data = _id; }

        NodeID& operator = ( const NodeID& from )
            {
                _id = from._id;
                return *this;
            }
        
        NodeID& operator = ( const std::string& from )
            {
                UuidFromString( (unsigned char*)from.c_str(), &_id );
                return *this;
            }
        
        bool operator == ( const NodeID& rhs ) const
            { 
                RPC_STATUS status; 
                return ( UuidEqual( const_cast<UUID*>( &_id ), 
                                    const_cast<UUID*>( &rhs._id ),
                                    &status ) == TRUE );
            }
        bool operator != ( const NodeID& rhs ) const
            { 
                RPC_STATUS status; 
                return ( UuidEqual( const_cast<UUID*>( &_id ), 
                                    const_cast<UUID*>( &rhs._id ),
                                    &status ) == FALSE );
            }

        bool operator <  ( const NodeID& rhs ) const
            { 
                RPC_STATUS status; 
                return UuidCompare( const_cast<UUID*>( &_id ), 
                                    const_cast<UUID*>( &rhs._id ),
                                    &status ) < 0;
            }
        bool operator >  ( const NodeID& rhs ) const
            { 
                RPC_STATUS status; 
                return UuidCompare( const_cast<UUID*>( &_id ), 
                                    const_cast<UUID*>( &rhs._id ),
                                    &status ) > 0;
            }

        bool operator ! () const 
            {
                RPC_STATUS status; 
                return ( UuidIsNil( const_cast<UUID*>( &_id ), &status )==TRUE);
            }

    private:
        UUID _id;
#else // WIN32
    public:
        /** Data type, used for network transport. */
        struct Data
        {
            uuid_t id;
        };

        NodeID( const bool generate = false )
            { generate ? uuid_generate( _id ) : uuid_clear( _id ); }
        NodeID( const NodeID& from ) { uuid_copy( _id, from._id ); }
        NodeID( const Data& from )   { uuid_copy( _id, from.id ); }

        void getData( Data& data ) const { uuid_copy( data.id, _id ); }

        NodeID& operator = ( const NodeID& from )
            {
                uuid_copy( _id, from._id );
                return *this;
            }
        NodeID& operator = ( const std::string& from )
            {
                uuid_parse( from.c_str(), _id );
                return *this;
            }
        
        bool operator == ( const NodeID& rhs ) const
            { return uuid_compare( _id, rhs._id ) == 0; }

        bool operator != ( const NodeID& rhs ) const
            { return uuid_compare( _id, rhs._id ) != 0; }
        bool operator <  ( const NodeID& rhs ) const
            { return uuid_compare( _id, rhs._id ) < 0; }
        bool operator >  ( const NodeID& rhs ) const
            { return uuid_compare( _id, rhs._id ) > 0; }

        bool operator ! () const { return uuid_is_null( _id ); }

    private:
        uuid_t _id;
#endif // WIN32

    public:
        static const NodeID ZERO;

    private:
        friend std::ostream& operator << ( std::ostream& os, const NodeID& id );
#ifdef WIN32_VC
        friend size_t stde::hash_compare< eqNet::NodeID >::operator() 
            ( const eqNet::NodeID& key ) const;
#else
        friend struct stde::hash< const eqNet::NodeID >;
#endif
    };

    /** A hash for NodeID keys. */
    template<class T> class NodeIDHash 
        : public stde::hash_map< const NodeID, T >
    {};

    /** NodeID& ostream operator. */
    inline std::ostream& operator << ( std::ostream& os, const NodeID& id )
    {
#ifdef WIN32
        unsigned char* uuid;
        UuidToString( const_cast<UUID*>( &id._id ), &uuid );
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

#ifdef WIN32_VC
template<>
inline size_t stde::hash_compare< eqNet::NodeID >::operator() 
    ( const eqNet::NodeID& key ) const
{
    return key._id.Data1;
}

template<>
inline size_t stde::hash_value( const eqNet::NodeID& key )
{
    stde::hash_compare< eqNet::NodeID > hash;
    return hash( key );
}

#elif defined (WIN32)

namespace __gnu_cxx
{
    template<> 
    struct hash< const eqNet::NodeID >
    {
        size_t operator()( const eqNet::NodeID& key ) const
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
    struct hash< const eqNet::NodeID >
    {
        size_t operator()( const eqNet::NodeID& key ) const
        {
            return (size_t)(*key._id);
        }
    };
}

#endif // WIN32_VC
#endif // EQNET_NODE_H
