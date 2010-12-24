
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_BARRIER_H
#define CO_BARRIER_H

#include <co/object.h>   // base class
#include <co/types.h>
#include <co/base/monitor.h> // member

#include <map>

namespace co
{
    /** A networked, versioned barrier. */
    class Barrier : public Object
    {
    public:
        /** 
         * Construct a new barrier.
         *
         * The master node will maintain the barrier state. It has to be
         * reachable from all other nodes participating in the barrier.
         *
         * The instance created using this constructor should be registered as
         * the master version of the barrier with the LocalNode. Note the node
         * of the object master, i.e., this instance, and the barrier's master
         * node might be different.
         */
        CO_API Barrier( NodePtr master, const uint32_t height = 0 );

        /** Construct a new barrier, to be mapped to the master version. */
        CO_API Barrier();

        /** Destruct the barrier. */
        CO_API virtual ~Barrier();

        /** 
         * @name Data Access
         *
         * After a change, the barrier should be committed and synced to the
         * same version on all nodes entering the barrier.
         */
        //@{
        /** Set the number of participants in the barrier. */
        void setHeight( const uint32_t height ) { _height = height; }

        /** Add one participant to the barrier. */
        void increase() { ++_height; }

        /** @return the number of participants. */
        uint32_t getHeight() const { return _height; }
        //@}

        /** @name Operations */
        //@{
        /** 
         * Enter the barrier, blocks until the barrier has been reached.
         *
         * The implementation currently assumes that the master node instance
         * also enters the barrier.
         */
        CO_API void enter();
        //@}

    protected:
        virtual void attach( const co::base::UUID& id, 
                             const uint32_t instanceID );

        virtual ChangeType getChangeType() const { return DELTA; }

        virtual void getInstanceData( DataOStream& os );
        virtual void applyInstanceData( DataIStream& is );
        virtual void pack( DataOStream& os );
        virtual void unpack( DataIStream& is );

    private:
        /** The master barrier node. */
        NodeID   _masterID;

        /** The height of the barrier, only set on the master. */
        uint32_t _height;

        /** The local, connected instantiation of the master node. */
        NodePtr _master;

        /** Slave nodes which have entered the barrier, index per version. */
        std::map< uint128_t, Nodes > _enteredNodes;
        
        /** The monitor used for barrier leave notification. */
        co::base::Monitor<uint32_t> _leaveNotify;

        /* The command handlers. */
        bool _cmdEnter( Command& command );
        bool _cmdEnterReply( Command& command );

        EQ_TS_VAR( _thread );
    };
}

#endif // CO_BARRIER_H

