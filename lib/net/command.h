
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

        /** @name Usage tracking. */
        //@{
        bool isFree() const { return ( _refCount==0 ); }
        void retain()  { ++_refCount; }
        void release() 
            {
                EQASSERT( _refCount != 0 );
#ifdef NDEBUG
                --_refCount;
#else
                if( --_refCount==0 )
                {
                    // Unref nodes in command to keep node ref counts easier for
                    // debugging.  Release builds will unref the nodes at
                    // receiver thread exit.
                    _node = 0;
                    _localNode = 0;
                }
#endif
            }
        //@}

        bool isValid() const { return ( _packet!=0 ); }
        uint64_t getAllocationSize() const { return _packetAllocSize; }

    private:
        Command();
        ~Command();
        void alloc( NodePtr node, NodePtr localNode, const uint64_t size );
        friend class CommandCache;

        Command& operator = ( Command& rhs ); // disable assignment
        Command( const Command& from );       // disable copy

        void _free();

        NodePtr  _node;
        NodePtr  _localNode;
        Packet*  _packet;
        base::mtLong _refCount;
        uint64_t _packetAllocSize;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Command& );
}
}
#endif // EQNET_COMMAND_H
