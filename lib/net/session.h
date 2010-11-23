
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/net/dispatcher.h>    // base class
#include <eq/net/localNode.h>          // used in inline method
#include <eq/net/objectVersion.h> // member
#include <eq/net/packets.h>       // used in inline method
#include <eq/net/version.h>       // enum

#include <eq/base/base.h>
#include <eq/base/idPool.h>
#include <eq/base/requestHandler.h>
#include <eq/base/spinLock.h>

namespace eq
{
namespace net
{
     class InstanceCache;

    /**
     * Provides higher-level functionality to a set of nodes.
     *
     * A session provides unique identifiers and eq::net::Object mapping for a
     * set of nodes. The master node registers the session, which makes this
     * node the session server and assigns a node-unique identifier to the
     * session. All other nodes map the session using this identifier.
     *
     * A received SessionPacket is dispatched to the locally-mapped session of
     * the same identifier.
     */
    class Session : public Dispatcher
    {
    public:
        /** Construct a new session. */
        EQNET_API Session();

        /** Destruct this session. */
        EQNET_API virtual ~Session();

        /** @name Data Access */
        //@{
        /** @return the identifier of this session. */
        const SessionID& getID() const { return _id; }

        /**
         * @return the local node to which this session is mapped, or 0 if the
         *         session is not mapped.
         */
        LocalNodePtr getLocalNode(){ return _localNode; }

        /**
         * @return the queue to the command thread of the local node, or 0 if
         *         the session is not mapped.
         */
        EQNET_API CommandQueue* getCommandThreadQueue();

        /** @return the server hosting this session, or 0 if the session is not
         *          mapped.. */
        NodePtr getServer() { return _server; }

        /** Disable the instance cache of an unattached session. */
        EQNET_API void disableInstanceCache();
        //@}


        /** @name Command Packet Dispatch */
        //@{

        /** 
         * Dispatches a command packet to the registered command queue.
         *
         * Session packets are dispatch on this session, object packets to the
         * appropriate objects mapped on this session.
         * 
         * @param packet the command packet.
         * @return true if the command was dispatched, false otherwise.
         * @sa Dispatcher::dispatchCommand
         */
        EQNET_API virtual bool dispatchCommand( Command& packet );

        /** 
         * Invokes the registered handler method for a command packet.
         * 
         * For object packets, invocation is forwarded to the appropriate
         * object(s).
         *
         * @param packet the command packet.
         * @return the result of the operation.
         * @sa Dispatcher::invokeCommand
         */
        EQNET_API virtual bool invokeCommand( Command& packet );
        //@}

        /**
         * @name Identifier management
         */
        //@{
        /** 
         * Set the master node for an identifier.
         * 
         * This can be used to identify the node which is responsible for the
         * object, action or information associated with an identifier. The
         * identifiers must be unique, it is therefore advised to allocate them
         * using genIDs().
         *
         * The master node must be reachable from this node and known by the
         * session server node.
         *
         * @param id the identifier.
         * @param master the master node for the block of identifiers.
         */
        EQNET_API void setIDMaster( const base::UUID& id, const NodeID& master );

        /** 
         * Delete the master node for an identifiers.
         * 
         * @param id the identifier.
         */
        EQNET_API void unsetIDMaster( const base::UUID& id );

        /** 
         * Returns the master node id for an identifier.
         * 
         * @param id the identifier.
         * @return the master node, or Node::ZERO if no master node is
         *         set for the identifier.
         */
        EQNET_API const NodeID getIDMaster( const base::UUID& id );
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
        EQNET_API bool registerObject( Object* object );

        /** 
         * Deregister a distributed object.
         *
         * @param object the object instance.
         */
        EQNET_API virtual void deregisterObject( Object* object );

        /** 
         * Map a distributed object.
         *
         * The mapped object becomes a slave instance of the master version
         * which was registered to the provided identifier. The given version
         * can be used to map a specific version.
         * 
         * If VERSION_NONE is provided, the slave instance is not initialized
         * with any data from the master. This is useful if the object has been
         * pre-initialized by other means, for example from a shared file
         * system.
         * 
         * If VERSION_OLDEST is provided, the oldest available version is
         * mapped.
         * 
         * If the requested version does no longer exist, mapObject()
         * will fail. If the requested version is newer than the head version,
         * mapObject() will block until the requested version is available.
         *
         * Mapping an object is a potentially time-consuming operation. Using
         * mapObjectNB() and mapObjectSync() to asynchronously map multiple
         * objects in parallel improves performance of this operation.
         *
         * @param object the object.
         * @param id the master object identifier.
         * @param version the initial version.
         * @return true if the object was mapped, false if the master of the
         *         object is not found or the requested version is no longer
         *         available.
         * @sa registerObject
         */
        EQNET_API bool mapObject( Object* object, const base::UUID& id, 
                                    const uint128_t& version = VERSION_OLDEST );

        /** Start mapping a distributed object. */
        EQNET_API uint32_t mapObjectNB( Object* object, const base::UUID& id, 
                                      const uint128_t& version = VERSION_OLDEST );
        /** Finalize the mapping of a distributed object. */
        EQNET_API bool mapObjectSync( const uint32_t requestID );

        /** Convenience wrapper for mapObject(). */
        bool mapObject( Object* object, const ObjectVersion& v )
            { return mapObject( object, v.identifier, v.version ); }

        /** Convenience wrapper for mapObjectNB(). */
        uint32_t mapObjectNB( Object* object, const ObjectVersion& v )
            { return mapObjectNB( object, v.identifier, v.version ); }

        /** 
         * Unmap a mapped object.
         * 
         * @param object the mapped object.
         */
        EQNET_API void unmapObject( Object* object );

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
        EQNET_API void attachObject( Object* object, const base::UUID& id, 
                                     const uint32_t instanceID );

        /** 
         * Detach an object.
         * 
         * @param object the attached object.
         */
        EQNET_API void detachObject( Object* object );

        /** Convenience method to deregister or unmap an object. */
        EQNET_API void releaseObject( Object* object );
        //@}


        /** @name Notifications */
        //@{
        /** 
         * Notification that this session has been mapped to a node.
         * 
         * Typically used by sub-classes to register command handlers. Always
         * call the parent's notifyMapped() first.
         *
         * @param node the node to which the session has been mapped.
         */
        EQNET_API virtual void notifyMapped( LocalNodePtr node );
        //@}

        /** @name Sending session packets */
        //@{
        /** 
         * Send a session packet to a node.
         * 
         * @param node the target node.
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        void send( NodePtr node, SessionPacket& packet )
            {
                packet.sessionID = _id;
                node->send( packet );
            }

        /** 
         * Send a packet to the session's server node.
         * 
         * @param packet the packet.
         */
        void send( SessionPacket& packet )
            { 
                packet.sessionID = _id;
                _server->send( packet );
            }

        /** 
         * Send a packet containing additional data to the session's server.
         * 
         * @param packet the packet.
         * @param data the data to attach to the packet.
         */
        template< typename T >
        void send( SessionPacket& packet, const std::vector<T>& data )
            { 
                packet.sessionID = _id;
                _server->send( packet, data );
            }

        /** 
         * Send a packet containing a string to a node.
         * 
         * @param node the target node.
         * @param packet the packet.
         * @param text the string to attach to the packet.
         * @return the success status of the transaction.
         */
        void send( NodePtr node, SessionPacket& packet, 
                   const std::string& text )
            {
                packet.sessionID = _id;
                node->send( packet, text );
            }

        /** 
         * Send a packet containing additional data to a node.
         * 
         * @param node the target node.
         * @param packet the packet.
         * @param data the data to attach to the packet.
         * @param size the size of the data.
         */
        void send( NodePtr node, SessionPacket& packet, 
                   const void* data, const uint64_t size )
            {
                packet.sessionID = _id;
                node->send( packet, data, size );
            }
        //@}

    protected:
        /** @internal */
        EQNET_API void expireInstanceData( const int64_t age );

        /**
         * @internal
         * Notification - no pending commands for the command thread.
         * @return true if more work is pending.
         */
        EQNET_API virtual bool notifyCommandThreadIdle();

        /** @internal ack an operation to the sender. */
        EQNET_API void ackRequest( NodePtr node, const uint32_t requestID );

        /** @internal swap the existing object by a new object and keep
                      the cm, id and instanceID. */
        EQNET_API void swapObject( Object* oldObject, Object* newObject );

    private:
        /** Set the local node to which this session is mapped */
        void _setLocalNode( LocalNodePtr node );

        friend class LocalNode;
        /** The local node managing the session. */
        LocalNodePtr _localNode;

        /** The node hosting the session. */
        NodePtr _server;

        /** The session's identifier. */
        SessionID _id;

        /** The state (master/client) of this session instance. */
        bool _isMaster;

        /** The identifiers for node-local instance identifiers. */
        base::a_int32_t _instanceIDs;

        typedef stde::hash_map< base::UUID, NodeID > NodeIDHash;
        /** The id->master mapping table. */
        base::Lockable< NodeIDHash, base::SpinLock > _idMasters;
        
        /** The global clock for send-on-register for objects. */
        base::Clock _clock;

        /** All registered and mapped objects. */
        base::Lockable< ObjectsHash, base::SpinLock > _objects;

        struct SendQueueItem
        {
            int64_t age;
            Object* object;
        };

        typedef std::deque< SendQueueItem > SendQueue;
        SendQueue _sendQueue;      //!< Object data to broadcast when idle

        InstanceCache* _instanceCache; //!< cached object mapping data

        const NodeID _pollIDMaster( const base::UUID& id ) const;
        NodePtr _pollIDMasterNode( const base::UUID& id ) const;
        NodePtr _connectMaster( const base::UUID& id );

        void _registerThreadObject( Object* object, const uint32_t id );

        /** Sends a packet to the local receiver thread. */
        void _sendLocal( SessionPacket& packet )
            {
                packet.sessionID = _id;
                _localNode->send( packet );
            }

        bool _dispatchObjectCommand( Command& command );
        bool _invokeObjectCommand( Command& packet );
        Object* _findObject( Command& command );

        void _attachObject( Object* object, const base::UUID& id, 
                            const uint32_t instanceID );
        void _detachObject( Object* object );

        uint32_t _setIDMasterNB( const base::UUID& id, const NodeID& master );
        void _setIDMasterSync( const uint32_t requestID );
        uint32_t _unsetIDMasterNB( const base::UUID& id );
        void _unsetIDMasterSync( const uint32_t requestID );

        /** The command handler functions. */
        bool _cmdAckRequest( Command& packet );
        bool _cmdSetIDMaster( Command& packet );
        bool _cmdUnsetIDMaster( Command& packet );
        bool _cmdGetIDMaster( Command& packet );
        bool _cmdGetIDMasterReply( Command& packet );
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

    std::ostream& operator << ( std::ostream& os, Session* session );
}
}
#endif // EQNET_SESSION_H

