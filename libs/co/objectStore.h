
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <co/dispatcher.h>    // base class
#include <co/version.h>       // enum

#include <co/base/lockable.h>  // member
#include <co/base/spinLock.h>  // member
#include <co/base/stdExt.h>    // member

namespace co
{
    class InstanceCache;
    struct ObjectVersion;

    /** An object store manages Object mapping for a LocalNode. */
    class ObjectStore : public Dispatcher
    {
    public:
        /** Construct a new ObjectStore. */
        ObjectStore( LocalNode* localNode );

        /** Destruct this ObjectStore. */
        virtual ~ObjectStore();

        /** Remove all objects and clear all caches. */
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

        /** @name Object Registration */
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
        uint32_t mapObjectNB( Object* object, const base::UUID& id, 
                              const uint128_t& version );

        /** Start mapping a distributed object. */
        uint32_t mapObjectNB( Object* object, const base::UUID& id, 
                              const uint128_t& version, NodePtr master );

        /** Finalize the mapping of a distributed object. */
        bool mapObjectSync( const uint32_t requestID );

        /** Convenience wrapper for mapObjectNB(). */
        uint32_t mapObjectNB( Object* object, const ObjectVersion& v );

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
         * commands though the local node. It does not establish any data
         * mapping to other object instances with the same identifier.
         * 
         * @param object the object.
         * @param id the object identifier.
         * @param instanceID the node-local instance identifier, or
         *                   EQ_ID_INVALID if this method should generate one.
         */
        void attachObject( Object* object, const base::UUID& id, 
                           const uint32_t instanceID );

        /** 
         * Detach an object.
         * 
         * @param object the attached object.
         */
        void detachObject( Object* object );

        /** @internal swap the existing object by a new object and keep
                      the cm, id and instanceID. */
        void swapObject( Object* oldObject, Object* newObject );
        //@}

        /** @name Instance Cache. */
        //@{
        /** Expire all data older than age from the cache. */
        void expireInstanceData( const int64_t age );

        /** Remove all entries of the node from the cache. */
        void removeInstanceData( const NodeID& nodeID );

        /** Disable the instance cache of an stopped local node. */
        void disableInstanceCache();

        /** Enable sending data of newly registered objects when idle. */
        void enableSendOnRegister();
        
        /**
         * Disable sending data of newly registered objects when idle.
         *
         * Enable and disable are counted, that is, the last disable on a
         * matched series of enable/disable will be effective. When
         * send-on-register gets deactivated, the associated queue is cleared
         * and all data send on multicast connections is finished.
         */
        void disableSendOnRegister(); 

        /**
         * @internal
         * Notification - no pending commands for the command thread.
         * @return true if more work is pending.
         */
        virtual bool notifyCommandThreadIdle();

        /**
         * @internal
         * Remove a slave node in all objects
         */
        void removeNode( NodePtr node );
        //@}

    private:
        /** The local node managing the object store. */
        LocalNode* const _localNode;

        /** The identifiers for node-local instance identifiers. */
        base::a_int32_t _instanceIDs;

        /** startSendOnRegister() invocations. */
        base::a_int32_t _sendOnRegister;

        typedef stde::hash_map< base::uint128_t, Objects > ObjectsHash;
        typedef ObjectsHash::const_iterator ObjectsHashCIter;

        /** All registered and mapped objects. 
         *   - write locked only in receiver thread
         *   - read unlocked in receiver thread 
         *   - read locked in all other threads
         */
        base::Lockable< ObjectsHash, base::SpinLock > _objects;

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
        NodeID _findMasterNodeID( const base::UUID& id );
 
        NodePtr _connectMaster( const base::UUID& id );

        void _attachObject( Object* object, const base::UUID& id, 
                            const uint32_t instanceID );
        void _detachObject( Object* object );

        /** The command handler functions. */
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
        bool _cmdDisableSendOnRegister( Command& command );
        bool _cmdRemoveNode( Command& command );

        EQ_TS_VAR( _receiverThread );
        EQ_TS_VAR( _commandThread );
    };

    std::ostream& operator << ( std::ostream& os, ObjectStore* objectStore );
}
#endif // CO_OBJECTSTORE_H
