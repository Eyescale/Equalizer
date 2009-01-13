
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/net/dispatcher.h>  // base class
#include <eq/net/object.h>      // Object::VERSION_NONE enum

#include <eq/base/base.h>
#include <eq/base/idPool.h>
#include <eq/base/requestHandler.h>

namespace eq
{
namespace net
{
    /**
     * Provides higher-level functionality to a set of nodes.
     *
     * A session provides unique identifiers and eq::net::Object mapping for a
     * set of nodes. The master node registers the session, which makes this
     * node the session server and assigns a node-unique identifier to the
     * session. All other nodes map the session using this identifier.
     */
    class EQ_EXPORT Session : public Dispatcher
    {
    public:
        /** Constructs a new session. */
        Session();

        virtual ~Session();

        /** 
         * Returns the identifier of this session.
         * 
         * @return the identifier.
         */
        uint32_t getID() const { return _id; }

        /** Set the local node to which this session is mapped */
        virtual void setLocalNode( NodePtr node );

        /** @return the local node holding this session. */
        NodePtr getLocalNode(){ return _localNode; }

        /** @return the command queue to the command thread. */
        CommandQueue* getCommandThreadQueue() 
            { return _localNode->getCommandThreadQueue(); }

        /** @return the server hosting this session. */
        NodePtr getServer(){ return _server; }

        /** 
         * Dispatches a command packet to the appropriate command queue.
         * 
         * @param packet the command packet.
         * @return the result of the operation.
         * @sa Dispatcher::dispatchCommand
         */
        virtual bool dispatchCommand( Command& packet );

        /** 
         * Dispatches a command packet to the appropriate handler method.
         * 
         * @param packet the command packet.
         * @return the result of the operation.
         * @sa Dispatcher::invokeCommand
         */
        virtual CommandResult invokeCommand( Command& packet );

        /**
         * @name Identifier management
         */
        //*{
        /** 
         * Generates a continous block of unique identifiers.
         * 
         * @param range the size of the block.
         * @return the first identifier of the block, or <code>0</code> if no
         *         identifier is available.
         */
        uint32_t genIDs( const uint32_t range );

        /** 
         * Frees a continous block of unique identifiers.
         * 
         * @param start the first identifier in the block.
         * @param range the size of the block.
         */
        void freeIDs( const uint32_t start, const uint32_t range );

        /** 
         * Set the master node for a block of identifiers.
         * 
         * This can be used to identify the node which is responsible for the
         * object, action or information associated with an identifier. The
         * identifiers must be unique, it is therefore advised to allocate them
         * using genIDs().
         *
         * The master node must be reachable from this node and known by the
         * session server node.
         *
         * @param start the first identifier of the block.
         * @param range the size of the block.
         * @param master the master node for the block of identifiers.
         */
        void setIDMaster( const uint32_t start, const uint32_t range, 
                          const NodeID& master );

        /** 
         * Delete the master node for a block of identifiers.
         * 
         * @param start the first identifier of the block.
         * @param range the size of the block.
         */
        void unsetIDMaster( const uint32_t start, const uint32_t range );

        /** 
         * Returns the master node id for an identifier.
         * 
         * @param id the identifier.
         * @return the master node, or Node::ZERO if no master node is
         *         set for the identifier.
         */
        const NodeID& getIDMaster( const uint32_t id );
        //*}

        /**
         * @name Object Registration
         */
        //*{
        /** 
         * Register a distributed object.
         *
         * The assigned identifier is unique across all registered objects in
         * the session.
         *
         * @param object the object instance.
         */
        void registerObject( Object* object );

        /** 
         * Deregister a distributed object.
         *
         * @param object the object instance.
         */
        void deregisterObject( Object* object );

        /** 
         * Map a distributed object.
         *
         * The mapped object becomes a slave instance of the master version
         * registered to the provided identifier. The version can be used to map
         * a specific version. If this version does not exist, mapObject() will
         * fail. If VERSION_NONE is provided, the oldest available version is
         * mapped. If the requested version is newer than the head version,
         * mapObject() will block until the requested version is available.
         * 
         * @param object the object.
         * @param id the master object identifier.
         * @param version the initial version.
         * @return true if the object was mapped, false if the master of the
         *         object is not found or the requested version is no longer
         *         available.
         * @sa registerObject
         */
        bool mapObject( Object* object, const uint32_t id, 
                        const uint32_t version = Object::VERSION_NONE );

        /** Start mapping a distributed object. */
        uint32_t mapObjectNB( Object* object, const uint32_t id, 
                              const uint32_t version = Object::VERSION_NONE );
        /** Finalize the mapping of a distributed object. */
        bool mapObjectSync( const uint32_t requestID );

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
         */
        void attachObject( Object* object, const uint32_t id );

        /** 
         * Detach an object.
         * 
         * @param object the attached object.
         */
        void detachObject( Object* object );
        //*}
        
    protected:
        /** 
         * Send a packet to the session's server node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( SessionPacket& packet )
            { 
                packet.sessionID = _id;
                return _server->send( packet );
            }

        template< typename T >
        bool send( SessionPacket& packet, const std::vector<T>& data )
            { 
                packet.sessionID = _id;
                return _server->send( packet, data );
            }

        /** 
         * Send a packet to a node.
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

        void send( NodePtr node, SessionPacket& packet, 
                   const std::string& text )
            {
                packet.sessionID = _id;
                node->send( packet, text );
            }

        void send( NodePtr node, SessionPacket& packet, 
                   const void* data, const uint64_t size )
            {
                packet.sessionID = _id;
                node->send( packet, data, size );
            }

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

    private:
        friend class Node;
        /** The local node managing the session. */
        NodePtr _localNode;

        /** The node hosting the session. */
        NodePtr _server;

        /** The session's identifier. */
        uint32_t _id;

        /** The state (master/client) of this session instance. */
        bool _isMaster;

        /** The distributed master identifier pool. */
        base::IDPool _masterPool;

        /** The local identifier pool. */
        base::IDPool _localPool;

        /** The identifiers for node-local instance identifiers. */
        uint32_t _instanceIDs;

        /** Stores a mapping from a block of identifiers to a master node. */
        struct IDMasterInfo
        {
            uint32_t start;
            uint32_t end;
            NodeID   master;
        };

        /** The id->master mapping table. */
        std::vector<IDMasterInfo> _idMasterInfos;
        base::SpinLock            _idMasterMutex;
        
        /** All registered and mapped objects. */
        IDHash< ObjectVector > _objects;
        base::SpinLock         _objectsMutex;

        const NodeID& _pollIDMaster( const uint32_t id ) const;
        NodePtr _pollIDMasterNode( const uint32_t id ) const;

        void _registerThreadObject( Object* object, const uint32_t id );

        /** Sends a packet to the local receiver thread. */
        void _sendLocal( SessionPacket& packet )
            {
                packet.sessionID = _id;
                _localNode->send( packet );
            }

        CommandResult _invokeObjectCommand( Command& packet );
        void _attachObject( Object* object, const uint32_t id );
        void _detachObject( Object* object );

        uint32_t _setIDMasterNB( const uint32_t start, const uint32_t range, 
                                 const NodeID& master );
        void _setIDMasterSync( const uint32_t requestID );


        /** The command handler functions. */
        CommandResult _cmdAckRequest( Command& packet );
        CommandResult _cmdGenIDs( Command& packet );
        CommandResult _cmdGenIDsReply( Command& packet );
        CommandResult _cmdSetIDMaster( Command& packet );
        CommandResult _cmdGetIDMaster( Command& packet );
        CommandResult _cmdGetIDMasterReply( Command& packet );
        CommandResult _cmdAttachObject( Command& command );
        CommandResult _cmdDetachObject( Command& command );
        CommandResult _cmdMapObject( Command& command );
        CommandResult _cmdSubscribeObject( Command& command );
        CommandResult _cmdSubscribeObjectSuccess( Command& command );
        CommandResult _cmdSubscribeObjectReply( Command& command );
        CommandResult _cmdUnsubscribeObject( Command& command );

        CHECK_THREAD_DECLARE( _receiverThread );
        CHECK_THREAD_DECLARE( _commandThread );
    };

    std::ostream& operator << ( std::ostream& os, Session* session );
}
}
#endif // EQNET_SESSION_H

