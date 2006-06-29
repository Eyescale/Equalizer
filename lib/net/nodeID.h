
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODEID_H
#define EQNET_NODEID_H

#include <eq/base/hash.h>
#include <uuid/uuid.h>

namespace eqNet
{

    /**
     * Wraps the universally unique node identifier.
     *
     * Not to be subclassed.
     */
    class NodeID
    {
    public:
        NodeID( const bool generate = false )
            { generate ? uuid_generate( _id ) : uuid_clear( _id ); }
        NodeID( const NodeID& from ) { uuid_copy( _id, from._id ); }

        void setNull(){ uuid_clear( _id ); }

        NodeID& operator = ( const NodeID& from )
            {
                uuid_copy( _id, from._id );
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
        friend std::ostream& operator << ( std::ostream& os, const NodeID& id );
        friend struct hashNodeID;
    };

    struct hashNodeID
    {
    public:
        size_t operator() (const NodeID& key) const
            {
                return (size_t)(*key._id);
            }
    };

    /** A hash for NodeID keys. */
    template<class T> class NodeIDHash 
        : public Sgi::hash_map<const NodeID, T, hashNodeID >
    {};

    inline std::ostream& operator << ( std::ostream& os, const NodeID& id )
    {
        char string[40];
        uuid_unparse( id._id, string );

        os << string;
        return os;
    }
};
#endif // EQNET_NODE_H
