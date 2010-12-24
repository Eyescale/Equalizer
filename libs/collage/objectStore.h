
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef CO_OBJECTSTORE_H
#define CO_OBJECTSTORE_H

#include <co/dispatcher.h>    // parent

#include <co/localNode.h>     // member
#include <co/objectVersion.h> // member
#include <co/packets.h>       // used in inline method
#include <co/version.h>       // enum

#include <co/base/os.h>
#include <co/base/lockable.h>

#include <co/base/spinLock.h>

namespace co
{
    class InstanceCache;

    /** An object store manages Object mapping for a LocalNode. */
    class ObjectStore : public Dispatcher
    {
    public:
        /** Construct a new ObjectStore. */
        ObjectStore( LocalNode* localNode );

        /** Destruct this ObjectStore. */
        virtual ~ObjectStore();

        /** Disable the instance cache of an stopped local node. */
        void disableInstanceCache();

        /** Cleanup all instance and objects link. */
        void clear();

        /** @name Command Packet Dispatch */
        //@{
        /** 
         * Dispatches a command object packet to the registered command queue.
         *
         * Object packets are dispatch to the appropriate objects mapped on 
         * this session.
         * 
         * @param packet the command packet.
         * @return true if the command was dispatched, false otherwise.
         */
        bool dispatchObjectCommand( Command& packet );
        //@}

        /**
         * @name Object Registration
         */
        //@{
        /** 
         * Register a distributed object.
         *
         * Registering a distributed object assigns a session-unique identifier
         * to this object, and makes this object the master version. The
         * identifier is used to map slave instances of the object. Master
         * versions of objects are typically writable and can commit new
         * versions of the distributed object.
         *
         * @param object the object instance.
         * @return true if the object was registered, false otherwise.
         */
        bool registerObject( Object* object );

        /** 
         * Deregister a distributed object.
         *
         * @param object the object instance.
         */
        virtual void deregisterObject( Object* object );


        /** Start mapping a distributed object. */
        uint32_t mapObjectNB( Object* object, const co::base::UUID& id, 
                              const uint128_t& version = VERSION_OLDEST );
        /** Finalize the mapping of a distributed object. */
        bool mapObjectSync( const uint32_t requestID );

        /** Convenience wrapper for mapObjectNB(). */
        uint32_t mapObjectNB( Object* object, const ObjectVersion& v )
            { return mapObjectNB( object, v.identifier, v.version ); }

        /** 
         * Unmap a mapped object.
         * 
         * @param object the mapped object.
         */
        void unmapObject( Object* object );

        /** 
         * Attach an object to an identifier.
         *
         * Attaching an object to an identifier enables it to receive object
         * commands through this session. It does not establish any mapping to
         * other object instances with the same identifier.
         * 
         * @param object the object.
         * @param id the object identifier.
         * @param instanceID the node-local instance identifier, or
         *                   EQ_ID_INVALID if this method should generate one.
         */
        void attachObject( Object* object, const co::base::UUID& id, 
                           const uint32_t instanceID );

        /** 
         * Detach an object.
         * 
         * @param object the attached object.
         */
        void detachObject( Object* object );

        /** Convenience method to deregister or unmap an object. */
        void releaseObject( Object* object );
        //@}

        /** @internal */
        void expireInstanceData( const int64_t age );

        /**
         * @internal
         * Notification - no pending commands for the command thread.
         * @return true if more work is pending.
         */
        virtual bool notifyCommandThreadIdle();

        /** @internal ack an operation to the sender. */
        void ackRequest( NodePtr node, const uint32_t requestID );

        /** @internal swap the existing object by a new object and keep
                      the cm, id and instanceID. */
        void swapObject( Object* oldObject, Object* newObject );

    private:
        /** The local node managing the object store. */
        LocalNode* const _localNode;

        /** The identifiers for node-local instance identifiers. */
        co::base::a_int32_t _instanceIDs;

        /** All registered and mapped objects. 
         *   - write locked only in receiver thread
         *   - read unlocked in receiver thread 
         *   - read locked in all other threads
         */
        co::base::Lockable< ObjectsHash, co::base::SpinLock > _objects;

        /** The global clock for send-on-register timeout. */
        co::base::Clock _clock;

        struct SendQueueItem
        {
            int64_t age;
            Object* object;
        };

        typedef std::deque< SendQueueItem > SendQueue;
        SendQueue _sendQueue;      //!< Object data to broadcast when idle

        InstanceCache* _instanceCache; //!< cached object mapping data

         /** 
         * Returns the master node id for an identifier.
         * 
         * @param id the identifier.
         * @return the master node, or UUID::ZERO if no master node is
         *         found for the identifier.
         */
        NodeID _findMasterNodeID( const co::base::UUID& id );
 
        NodePtr _connectMaster( const co::base::UUID& id );

        void _attachObject( Object* object, const co::base::UUID& id, 
                            const uint32_t instanceID );
        void _detachObject( Object* object );

        /** The command handler functions. */
        bool _cmdAckRequest( Command& packet );
        bool _cmdFindMasterNodeID( Command& command );
        bool _cmdFindMasterNodeIDReply( Command& command );
        bool _cmdAttachObject( Command& command );
        bool _cmdDetachObject( Command& command );
        bool _cmdMapObject( Command& command );
        bool _cmdMapObjectSuccess( Command& command );
        bool _cmdMapObjectReply( Command& command );
        bool _cmdUnmapObject( Command& command );
        bool _cmdUnsubscribeObject( Command& command );
        bool _cmdInstance( Command& command );
        bool _cmdRegisterObject( Command& command );
        bool _cmdDeregisterObject( Command& command );

        EQ_TS_VAR( _receiverThread );
        EQ_TS_VAR( _commandThread );
    };

    std::ostream& operator << ( std::ostream& os, ObjectStore* objectStore );
}
#endif // CO_OBJECTSTORE_H

