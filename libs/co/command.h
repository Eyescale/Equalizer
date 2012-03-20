
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
#include <lunchbox/refPtr.h> // NodePtr member

namespace co
{    
    /**
     * A class managing received command packets.
     *
     * This class is used by the LocalNode to pass received packets to the
     * Dispatcher and ultimately command handler functions. It is not intended
     * to be instantiated by applications. It is the applications responsible to
     * provide the correct packet type to the templated get methods.
     */
    class Command : public lunchbox::Referenced
    {
    public:
        explicit Command( lunchbox::a_int32_t& freeCounter ); //!< @internal
        ~Command(); //!< @internal

        /** @name Data Access */
        //@{
        /** @return a const pointer to the packet. @version 1.0 */
        template< class P > const P* get() const
            { LBASSERT( _packet ); return static_cast<P*>( _packet ); }

        /** @return a modifiable pointer to the packet. @version 1.0 */
        template< class P > P* getModifiable()
            { LBASSERT( _packet ); return static_cast<P*>( _packet ); }

        /** @return the sending node proxy instance. @version 1.0 */
        NodePtr getNode() const { return _node; }

        /** @return the receiving node. @version 1.0 */
        LocalNodePtr getLocalNode() const { return _localNode; }

        /** Access the packet directly. @version 1.0 */
        Packet* operator->() { LBASSERT(_packet); return _packet; }

        /** Access the packet directly. @version 1.0 */
        const Packet* operator->() const { LBASSERT(_packet); return _packet; }

        /** @internal @return true if the command has a valid packet. */
        bool isValid() const { return ( _packet!=0 ); }

        /** @internal @return the amount of memory currently allocated. */
        uint64_t getAllocationSize() const { return _dataSize; }
        //@}

        /** @internal @name Dispatch command functions.. */
        //@{
        /** @internal Set the function to which the packet is dispatched. */
        void setDispatchFunction( const Dispatcher::Func& func )
            { LBASSERT( !_func.isValid( )); _func = func; }

        /** Invoke and clear the command function of a dispatched command. */
        CO_API bool operator()();
        //@}

        /** @internal @return true if the packet is no longer in use. */
        bool isFree() const { return getRefCount() == 0; }

        /** @internal @return the number of newly allocated bytes. */
        size_t alloc_( NodePtr node, LocalNodePtr localNode,
                       const uint64_t size );

        /** 
         * @internal Clone the from command into this command.
         * 
         * The command will share all data but the dispatch function. The
         * command's allocation size will be 0 and it will never delete the
         * shared data. The command will release its reference to the from
         * command when it is released.
         */
        void clone_( CommandPtr from );

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

        CommandPtr _master;
        lunchbox::a_int32_t& _freeCount;

        Dispatcher::Func _func;
        friend CO_API std::ostream& operator << (std::ostream&, const Command&);

        virtual void deleteReferenced( const Referenced* object ) const;

        LB_TS_VAR( _writeThread );
    };

    CO_API std::ostream& operator << ( std::ostream& os, const Command& );
}
#endif // CO_COMMAND_H
