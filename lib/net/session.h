
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
     * Manages a session.
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

        /** @return the local node holding this session. */
        eqBase::RefPtr<Node> getLocalNode(){ return _localNode; }

        /** 
         * @return the server hosting this session. 
         */
        eqBase::RefPtr<Node> getServer(){ return _server; }

        /** 
         * Dispatches a command packet to the appropriate handler.
         * 
         * @param node the node which sent the packet.
         * @param packet the packet.
         * @return the result of the operation.
         * @sa Base::invokeCommand
         */
        CommandResult dispatchCommand( Command& packet );

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
         * @todo getID();
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
         * Registers a new distributed object.
         *
         * The assigned identifier is unique across all registered objects. The
         * object gets referenced.
         *
         * @param object the object instance.
         * @param master the master node for the object, can be
         *               <code>NULL</code> for unmanaged objects.
         */
        void registerObject( Object* object, eqBase::RefPtr<Node> master,
                          const Object::SharePolicy policy = Object::SHARE_NODE,
                             const Object::ThreadSafety ts = Object::CS_AUTO );

        void addRegisteredObject( const uint32_t id, Object* object,
                                  const  Object::SharePolicy policy );
        void removeRegisteredObject( Object* object, Object::SharePolicy
                                     policy = Object::SHARE_UNDEFINED );

        /** 
         * Access a distributed object.
         * 
         * The object will be instanciated locally, if necessary. During
         * instanciation, it gets referenced. Versioned objects need to have at
         * least one committed version. This function can not be called from the
         * receiver thread, that is, from any command handling function. The
         * object can be deregistered when it is no longer needed.
         * If a version other than the head version is specified, the function
         * will block until the version has been committed. It may fail if the
         * version has already been obsoleted.
         *
         * @param id the object's identifier.
         * @param policy the object's instanciation scope.
         * @param version the initial version.
         * @param ts the thread-safety needed for the object.
         * @return the object, or <code>NULL</code> if the object is not known
         *         or could not be instanciated.
         */
        Object* getObject( const uint32_t id,
                           const Object::SharePolicy policy =Object::SHARE_NODE,
                           const uint32_t version = Object::VERSION_HEAD,
                           const Object::ThreadSafety ts = Object::CS_AUTO );

        /** 
         * Access a registered, distributed object.
         * 
         * Note that objects with the share policy SHARE_NEVER are not tracked
         * and can therefore not be accessed using pollObject().
         *
         * @param id the object's identifier.
         * @param scope the object's instanciation scope.
         * @return the object, or <code>NULL</code> if the object is not known.
         */
        Object* pollObject( const uint32_t id,
                        const Object::SharePolicy policy = Object::SHARE_NODE );

        /** 
         * Deregisters a distributed object.
         *
         * The object gets dereferenced.
         * 
         * @param object the object instance.
         */
        void deregisterObject( Object* object );
        //*}
        
    protected:
        /** 
         * Instanciate the slave(proxy) instance of a object on this session.
         * 
         * @param type the type identifier of the object.
         * @param data the instance data of the object.
         * @param dataSize the data size.
         * @return the object, or <code>NULL</code> upon error.
         * @sa Object::getInstanceInfo
         */
        virtual Object* instanciateObject( const uint32_t type,
                                           const void* data, 
                                           const uint64_t dataSize );
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

        /** The session's identifier. */
        uint32_t _id;
        
    private:
        friend class Node;
        /** The local node managing the session. */
        eqBase::RefPtr<Node> _localNode;

        /** The node hosting the session. */
        eqBase::RefPtr<Node> _server;

        /** The session's name. */
        std::string _name;

        /** The state (master/client) of this session instance. */
        bool _isMaster;

        /** The distributed master identifier pool. */
        eqBase::IDPool _masterPool;

        /** The local identifier pool. */
        eqBase::IDPool _localPool;

        /** The identifier pool for node-local instance identifiers. */
        eqBase::IDPool _instanceIDs;

        /** Stores a mapping from a block of identifiers to a master node. */
        struct IDMasterInfo
        {
            uint32_t                            start;
            uint32_t                            end;
            NodeID                              master;
            std::vector< eqBase::RefPtr<Node> > slaves;
        };
        /** The id->master mapping table. */
        std::vector<IDMasterInfo> _idMasterInfos;
        
        /** All SCOPE_THREAD objects, indexed by identifier. */
        eqBase::PerThread< IDHash<Object*>* > _threadObjects;
        /** All SCOPE_NODE objects, indexed by identifier. */
        IDHash<Object*> _nodeObjects;
        /** All registered objects - unshared, thread and node objects. */
        IDHash< std::vector<Object*> > _registeredObjects;

        /** The current state of pending object instanciations. */
        struct GetObjectState
        {
            GetObjectState()
                    : object( NULL ),
                      nodeConnectRequestID( EQ_ID_INVALID ),
                      instState( Object::INST_UNKNOWN ), 
                      pending( false )
                {}

            Object*             object;
            uint32_t            objectID;
            uint32_t            version;
            uint32_t            nodeConnectRequestID;
            Object::SharePolicy policy;
            Object::InstState   instState;
            bool                threadSafe;
            bool                pending;
        };
        IDHash<GetObjectState*> _objectInstStates;

        const NodeID& _pollIDMaster( const uint32_t id ) const;
        eqBase::RefPtr<Node> _pollIDMasterNode( const uint32_t id ) const;

        void _registerThreadObject( Object* object, const uint32_t id );

        /** Sends a packet to the local receiver thread. */
        void _sendLocal( SessionPacket& packet )
            {
                packet.sessionID = _id;
                _localNode->send( packet );
            }

        CommandResult _handleObjectCommand( Command& packet );
        CommandResult   _instObject( GetObjectState* state );
        void              _sendInitObject( GetObjectState* state, 
                                           eqBase::RefPtr<Node> master );

        /** The command handler functions. */
        CommandResult _cmdGenIDs( Command& packet );
        CommandResult _cmdGenIDsReply( Command& packet );
        CommandResult _cmdSetIDMaster( Command& packet );
        CommandResult _cmdGetIDMaster( Command& packet );
        CommandResult _cmdGetIDMasterReply( Command& packet );
        CommandResult _cmdGetObjectMaster( Command& packet );
        CommandResult _cmdGetObjectMasterReply( Command& pkg);
        CommandResult _cmdRegisterObject( Command& packet );
        CommandResult _cmdUnregisterObject( Command& packet );
        CommandResult _cmdGetObject( Command& packet );
        CommandResult _cmdInitObject( Command& packet );
        CommandResult _cmdInstanciateObject( Command& packet);
        CommandResult _cmdInitObjectReply( Command& packet );

        CHECK_THREAD_DECLARE( _receiverThread );
    };
    std::ostream& operator << ( std::ostream& os, Session* session );
}
#endif // EQNET_SESSION_PRIV_H

