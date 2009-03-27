
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include <eq/net/node.h>

#include <eq/base/base.h>
#include <eq/base/refPtr.h>

namespace eq
{
namespace net
{    
    struct Packet;

    /**
     * A class managing commands and the ownership of packets.
     *
     * A RefPtr<Packet> can't be used, since Packets are plain C structs send
     * over the network. The Command manages the ownership of packet by
     * claiming them so that only one can hold a given packet at a time.
     * A packet passed to a holder is owned by it and will be deleted
     * automatically when necessary.
     */
    class Command 
    {
    public:
        Command();
        Command( const Command& from ); // deep copy (of _packet)
        ~Command();
        
        void swap( Command& rhs );

        Packet*       getPacket()              { return _packet; }
        const Packet* getPacket() const        { return _packet; }

        template< class P > P* getPacket()
            { EQASSERT( _packet ); return reinterpret_cast<P*>( _packet ); }
        template< class P > const P* getPacket() const
            { EQASSERT( _packet ); return reinterpret_cast<P*>( _packet ); }

        NodePtr getNode()      const { return _node; }
        NodePtr getLocalNode() const { return _localNode; }

        bool     operator ! () const     { return ( _packet==0 ); }

        Packet*       operator->()       { EQASSERT(_packet); return _packet; }
        const Packet* operator->() const { EQASSERT(_packet); return _packet; }

        void allocate( NodePtr node, NodePtr localNode,
                       const uint64_t packetSize );
        void release();
        bool isValid() const { return ( _packet!=0 ); }

        bool isDispatched() const { return _dispatched; }

    private:
        Command& operator = ( Command& rhs ); // disable assignment

        NodePtr  _node;
        NodePtr  _localNode;
        Packet*  _packet;
        uint64_t _packetAllocSize;

        bool     _dispatched;
        friend class CommandQueue; // sets _dispatched to true
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Command& );
}
}
#endif // EQNET_COMMAND_H
