
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

#ifndef EQNET_COMMAND_H
#define EQNET_COMMAND_H

#include <eq/net/node.h> // NodePtr members

#include <eq/base/base.h>
#include <eq/base/refPtr.h>

namespace eq
{
namespace net
{    
    struct Packet;

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
        /** @name Data access. */
        //@{
        Packet*       getPacket()              { return _packet; }
        const Packet* getPacket() const        { return _packet; }

        template< class P > P* getPacket()
            { EQASSERT( _packet ); return static_cast<P*>( _packet ); }
        template< class P > const P* getPacket() const
            { EQASSERT( _packet ); return static_cast<P*>( _packet ); }

        NodePtr getNode()      const { return _node; }
        NodePtr getLocalNode() const { return _localNode; }

        bool     operator ! () const     { return ( _packet==0 ); }

        Packet*       operator->()       { EQASSERT(_packet); return _packet; }
        const Packet* operator->() const { EQASSERT(_packet); return _packet; }

        bool isValid() const { return ( _packet!=0 ); }
        uint64_t getAllocationSize() const { return _dataSize; }

        void setDispatchID( const uint32_t id ) { _dispatchID = id; }
        uint32_t getDispatchID() const { return _dispatchID; }
        //@}

        /** @name Usage tracking. */
        //@{
        bool isFree() const
            { CHECK_THREAD( _writeThread ); return ( _refCount==0 ); }
        void retain();
        EQ_EXPORT void release();
        //@}

    private:
        Command();
        ~Command();

        /** @return the number of newly allocated bytes. */
        size_t _alloc( NodePtr node, NodePtr localNode, const uint64_t size );

        /** 
         * Clone the from command into this command.
         * 
         * The command will share all data but the dispatch id. The command's
         * allocation size will be 0 and it will never delete the shared
         * data. The command will (de)reference the from command on each
         * retain/release.
         */
        void _clone( Command& from );

        friend class CommandCache;

        Command& operator = ( Command& rhs ); // disable assignment
        Command( const Command& from );       // disable copy

        void _free();

        NodePtr  _node; //!< The node sending the packet
        NodePtr  _localNode; //!< The node receiving the packet
        Packet*  _packet; //!< The packet (this or master _data)

        Packet* _data; //!< Our allocated data
        uint64_t _dataSize; //!< The size of the allocation

        base::a_int32_t* _refCountMaster;
        base::a_int32_t _refCount;

        uint32_t _dispatchID;
        CHECK_THREAD_DECLARE( _writeThread );
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Command& );
}
}
#endif // EQNET_COMMAND_H
