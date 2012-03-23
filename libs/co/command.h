
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_COMMAND_H
#define CO_COMMAND_H

#include <co/api.h>
#include <co/localNode.h> // NodePtr members

#include <lunchbox/atomic.h> // member
#include <lunchbox/refPtr.h> // NodePtr

namespace co
{    
    /**
     * @internal
     * A class managing command packets.
     *
     * A RefPtr<Packet> can't be used, since Packets are plain C structs send
     * over the network.
     */
    class Command 
    {
    public:
        /** @name Data Access */
        //@{
        template< class P > P* getModifiable()
            { EQASSERT( _packet ); return static_cast<P*>( _packet ); }
        template< class P > const P* get() const
            { EQASSERT( _packet ); return static_cast<P*>( _packet ); }

        NodePtr getNode()      const { return _node; }
        LocalNodePtr getLocalNode() const { return _localNode; }

        bool          operator ! () const { return ( _packet==0 ); }
        Packet*       operator->()       { EQASSERT(_packet); return _packet; }
        const Packet* operator->() const { EQASSERT(_packet); return _packet; }

        bool isValid() const { return ( _packet!=0 ); }
        uint64_t getAllocationSize() const { return _dataSize; }

        void setDispatchFunction( const Dispatcher::Func& func )
            { EQASSERT( !_func.isValid( )); _func = func; }
        //@}

        /** @name Usage tracking. */
        //@{
        bool isFree() const { return ( _refCount==0 ); }
        CO_API void retain();
        CO_API void release();
        //@}

        /** Invoke and clear the command function of a dispatched command. */
        CO_API bool operator()();

        explicit Command( lunchbox::a_int32_t& freeCounter ); //!< @internal
        ~Command(); //!< @internal

        /** @internal @return the number of newly allocated bytes. */
        size_t alloc_( NodePtr node, LocalNodePtr localNode,
                       const uint64_t size );

        /** 
         * @internal Clone the from command into this command.
         * 
         * The command will share all data but the dispatch function. The
         * command's allocation size will be 0 and it will never delete the
         * shared data. The command will (de)reference the from command on each
         * retain/release.
         */
        void clone_( Command& from );

    private:
        Command& operator = ( Command& rhs ); // disable assignment
        Command( const Command& from );       // disable copy
        Command();                            // disable default ctor

        void _free();

        NodePtr       _node;      //!< The node sending the packet
        LocalNodePtr  _localNode; //!< The node receiving the packet
        Packet*       _packet;    //!< The packet (this or master _data)

        Packet*  _data;     //!< Our allocated data
        uint64_t _dataSize; //!< The size of the allocation

        lunchbox::a_int32_t* _refCountMaster;
        lunchbox::a_int32_t  _refCount;
        lunchbox::a_int32_t& _freeCount;

        Dispatcher::Func _func;
        friend CO_API std::ostream& operator << (std::ostream&, const Command&);

        EQ_TS_VAR( _writeThread );
    };

    CO_API std::ostream& operator << ( std::ostream& os, const Command& );
}
#endif // CO_COMMAND_H
