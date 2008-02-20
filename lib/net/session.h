
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/net/base.h>
#include <eq/net/commands.h>
#include <eq/net/global.h>
#include <eq/net/idHash.h>
#include <eq/net/node.h>
#include <eq/net/object.h>

#include <eq/base/base.h>
#include <eq/base/idPool.h>
#include <eq/base/requestHandler.h>

namespace eqNet
{
    /**
     * A logical abstraction for multiple Node s.
     *
     * A session provides unique identifiers for a number of nodes.
     */
    class EQ_EXPORT Session : public Base
    {
    public:
        /** 
         * Constructs a new session.
         *
         * @param nCommands the highest command ID to be handled by the session,
         *                  at least <code>CMD_SESSION_CUSTOM</code>.
         * @param threadSafe if <code>true</code>, all public functions are
         *                   thread-safe.
         */
        Session( const bool threadSafe = false );

        virtual ~Session();

        /** 
         * Returns the name of the session.
         * 
         * @return the name of the session.
         */
        const std::string& getName() const { return _name; }

        /** 
         * Returns the identifier of this session.
         * 
         * @return the identifier.
         */
        uint32_t getID() const { return _id; }

        /** Set the local node */
        virtual void setLocalNode( eqBase::RefPtr< Node > node );

        /** @return the local node holding this session. */
        eqBase::RefPtr<Node> getLocalNode(){ return _localNode; }

        /** @return the command queue to the command thread. */
        CommandQueue& getCommandThreadQueue() 
            { return _localNode->getCommandThreadQueue(); }

        /** 
         * @return the server hosting this session. 
         */
        eqBase::RefPtr<Node> getServer(){ return _server; }

        /** 
         * Dispatches a command packet to the appropriate command queue.
         * 
         * @param command the command packet.
         * @return the result of the operation.
         * @sa Base::dispatchCommand
         */
        virtual bool dispatchCommand( Command& packet );

        /** 
         * Dispatches a command packet to the appropriate handler method.
         * 
         * @param command the command packet.
         * @return the result of the operation.
         * @sa Base::invokeCommand
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
         * registered to the provided identifier.
         * 
         * @param object the object.
         * @param id the master object identifier.
         * @return true if the object was mapped, false if the master of the
         *         object is no longer mapped.
         * @sa registerObject
         */
        bool mapObject( Object* object, const uint32_t id );

        /** Start mapping a distributed object. */
        uint32_t mapObjectNB( Object* object, const uint32_t id );
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
         * Detach an object..
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

        /** 
         * Send a packet to a node.
         * 
         * @param node the target node.
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        void send( eqBase::RefPtr<Node> node, SessionPacket& packet )
            {
                packet.sessionID = _id;
                node->send( packet );
            }

        void send( eqBase::RefPtr<Node> node, SessionPacket& packet, 
                   const std::string& text )
            {
                packet.sessionID = _id;
                node->send( packet, text );
            }

        void send( eqBase::RefPtr<Node> node, SessionPacket& packet, 
                   const void* data, const uint64_t size )
            {
                packet.sessionID = _id;
                node->send( packet, data, size );
            }

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

    private:
        friend class Node;
        /** The local node managing the session. */
        eqBase::RefPtr<Node> _localNode;

        /** The node hosting the session. */
        eqBase::RefPtr<Node> _server;

        /** The session's identifier. */
        uint32_t _id;
        
        /** The session's name. */
        std::string _name;

        /** The state (master/client) of this session instance. */
        bool _isMaster;

        /** The distributed master identifier pool. */
        eqBase::IDPool _masterPool;

        /** The local identifier pool. */
        eqBase::IDPool _localPool;

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
        
        /** All registered and mapped objects. */
        IDHash< std::vector<Object*> > _objects;
        eqBase::SpinLock               _objectsMutex;

        const NodeID& _pollIDMaster( const uint32_t id ) const;
        eqBase::RefPtr<Node> _pollIDMasterNode( const uint32_t id ) const;

        void _registerThreadObject( Object* object, const uint32_t id );

        /** Sends a packet to the local receiver thread. */
        void _sendLocal( SessionPacket& packet )
            {
                packet.sessionID = _id;
                _localNode->send( packet );
            }

        CommandResult _invokeObjectCommand( Command& packet );

        /** The command handler functions. */
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
    };
    std::ostream& operator << ( std::ostream& os, Session* session );
}
#endif // EQNET_SESSION_H

