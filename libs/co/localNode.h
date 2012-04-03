
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef CO_LOCALNODE_H
#define CO_LOCALNODE_H

#include <co/defines.h>
#include <co/node.h>            // base class
#include <co/objectVersion.h>   // VERSION_FOO used inline
#include <lunchbox/requestHandler.h> // base class

namespace co
{
namespace detail { class LocalNode; class ReceiverThread; class CommandThread; }

    /** 
     * Specialization of a local node.
     *
     * Local nodes listen on network connections, manage connections to other
     * nodes and provide session registration, mapping and command dispatch.
     */
    class LocalNode : public lunchbox::RequestHandler, public Node
    {
    public:
        CO_API LocalNode( );
        CO_API virtual ~LocalNode( );

        typedef NodePtr SendToken; //!< An acquired send token

        /**
         * @name State Changes
         *
         * The following methods affect the state of the node by changing its
         * connectivity to the network.
         */
        //@{
        /** 
         * Initialize the node.
         *
         * Parses the following command line options and calls listen()
         * afterwards:
         *
         * The '--eq-listen &lt;connection description&gt;' command line option
         * is parsed by this method to add listening connections to this
         * node. This parameter might be used multiple times.
         * ConnectionDescription::fromString() is used to parse the provided
         * description.
         *
         * The '--co-globals &lt;string&gt;' option is used to initialize the
         * Globals. The string is parsed used Globals::fromString().
         *
         * Please note that further command line parameters are recognized by
         * co::init().
         *
         * @param argc the command line argument count.
         * @param argv the command line argument values.
         * @return <code>true</code> if the client was successfully initialized,
         *         <code>false</code> otherwise.
         */
        CO_API virtual bool initLocal( const int argc, char** argv );
        
        /**
         * Open all connections and put this node into the listening state.
         *
         * The node will spawn a receiver and command thread, and listen on all
         * connections for incoming commands. The node will be in the listening
         * state if the method completed successfully. A listening node can
         * connect other nodes.
         * 
         * @return <code>true</code> if the node could be initialized,
         *         <code>false</code> if not.
         * @sa connect
         */
        CO_API virtual bool listen();
        CO_API virtual bool listen( ConnectionPtr connection ); //!< @internal

        /**
         * Close a listening node.
         * 
         * Disconnects all connected node proxies, closes the listening
         * connections and terminates all threads created in listen().
         * 
         * @return <code>true</code> if the node was stopped, <code>false</code>
         *         if it was not stopped.
         */
        CO_API virtual bool close();

        /** Close a listening node. */
        virtual bool exitLocal() { return close(); }

        /**
         * Connect a proxy node to this listening node.
         *
         * The connection descriptions of the node are used to connect the
         * node. On success, the node is in the connected state, otherwise its
         * state is unchanged.
         *
         * This method is one-sided, that is, the node to be connected should
         * not initiate a connection to this node at the same time. For
         * concurrent connects use the other connect() method using node
         * identifiers.
         *
         * @param node the remote node.
         * @return true if this node was connected, false otherwise.
         */
        CO_API bool connect( NodePtr node );

        /** 
         * Create and connect a node given by an identifier.
         *
         * This method is two-sided and thread-safe, that is, it can be called
         * by multiple threads on the same node with the same nodeID, or
         * concurrently on two nodes with each others' nodeID.
         * 
         * @param nodeID the identifier of the node to connect.
         * @return the connected node, or an invalid RefPtr if the node could
         *         not be connected.
         */
        CO_API NodePtr connect( const NodeID& nodeID );

        /** 
         * Disconnects a connected node.
         *
         * @param node the remote node.
         * @return <code>true</code> if the node was disconnected correctly,
         *         <code>false</code> otherwise.
         */
        CO_API virtual bool disconnect( NodePtr node );
        //@}

        /** @name Object Registry */
        //@{
        /** Disable the instance cache of a stopped local node. */
        CO_API void disableInstanceCache();

        /** @internal */
        CO_API void expireInstanceData( const int64_t age );

        /**
         * Enable sending instance data after registration.
         *
         * Send-on-register starts transmitting instance data of registered
         * objects directly after they have been registered. The data is cached
         * on remote nodes and accelerates object mapping. Send-on-register
         * should not be active when remote nodes are joining a multicast group
         * of this node, since they will potentially read out-of-order data
         * streams on the multicast connection.
         *
         * Enable and disable are counted, that is, the last enable on a
         * matched series of disable/enable will be effective. The disable is
         * completely synchronous, that is, no more instance data will be sent
         * after the first disable.
         */
        CO_API void enableSendOnRegister();
        
        /** Disable sending data of newly registered objects when idle. */
        CO_API void disableSendOnRegister(); 
        
        /** 
         * Register a distributed object.
         *
         * Registering a distributed object makes this object the master
         * version. The object's identifier is used to map slave instances of
         * the object. Master versions of objects are typically writable and can
         * commit new versions of the distributed object.
         *
         * @param object the object instance.
         * @return true if the object was registered, false otherwise.
         */
        CO_API bool registerObject( Object* object );

        /** 
         * Deregister a distributed object.
         *
         * @param object the object instance.
         */
        CO_API virtual void deregisterObject( Object* object );

        /** 
         * Map a distributed object.
         *
         * The mapped object becomes a slave instance of the master version
         * which was registered with the provided identifier. The given version
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
         * If a concrete requested version no longer exists, mapObject() will
         * map the oldest available version.
         *
         * If the requested version is newer than the head version, mapObject()
         * will block until the requested version is available.
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
        CO_API bool mapObject( Object* object, const UUID& id, 
                               const uint128_t& version = VERSION_OLDEST );

        /** Convenience wrapper for mapObject(). */
        bool mapObject( Object* object, const ObjectVersion& v )
            { return mapObject( object, v.identifier, v.version ); }

        /** Start mapping a distributed object. @sa mapObject() */
        CO_API uint32_t mapObjectNB( Object* object, const UUID& id, 
                                    const uint128_t& version = VERSION_OLDEST );

        /**
         * Start mapping a distributed object from a known master.
         * @sa mapObject()
         */
        CO_API uint32_t mapObjectNB( Object* object, const UUID& id, 
                                     const uint128_t& version, NodePtr master );

        /** Finalize the mapping of a distributed object. */
        CO_API bool mapObjectSync( const uint32_t requestID );

        /** 
         * Unmap a mapped object.
         * 
         * @param object the mapped object.
         */
        CO_API void unmapObject( Object* object );

        /** Convenience method to deregister or unmap an object. */
        CO_API void releaseObject( Object* object );

        /** 
         * Handler for an Object::push operation.
         *
         * Called at least on each node listed in an Object::push upon reception
         * of the pushed data from the command thread. Called on all nodes of a
         * multicast group, even on nodes not listed in the Object::push.
         *
         * The default implementation is empty. Typically used to create an
         * Object on a remote node, using the typeID for instantiation, the
         * istream to initialize it and the objectID to map it using
         * VERSION_NONE. The groupID may be used to differentiate multiple
         * concurrent push operations.
         *
         * @param groupID The group identifier given to Object::push()
         * @param typeID The type identifier given to Object::push()
         * @param objectID The identifier of the pushed object
         * @param istream the input data stream containing the instance data.
         */
        CO_API virtual void objectPush( const uint128_t& groupID,
                                        const uint128_t& typeID,
                                        const uint128_t& objectID,
                                        DataIStream& istream );

        /** @internal swap the existing object by a new object and keep
                      the cm, id and instanceID. */
        CO_API void swapObject( Object* oldObject, Object* newObject );
        //@}

        /** @name Data Access */
        //@{
        /** 
         * Get a node by identifier.
         *
         * The node might not be connected.
         *
         * @param id the node identifier.
         * @return the node.
         */
        CO_API NodePtr getNode( const NodeID& id ) const;

        /** Assemble a vector of the currently connected nodes. */
        void getNodes( Nodes& nodes, const bool addSelf = true ) const;

        /** Return the command queue to the command thread. */
        CO_API CommandQueue* getCommandThreadQueue();

        /** 
         * @return true if executed from the command handler thread, false if
         *         not.
         */
        CO_API bool inCommandThread() const;

        /** @internal */
        CO_API int64_t getTime64() const;
        //@}

        /** @name Operations */
        //@{
        /**
         * Add a listening connection to this listening node.
         * @return the listening connection, or 0 upon error.
         */
        CO_API ConnectionPtr addListener( ConnectionDescriptionPtr desc );

        /** Add a listening connection to this listening node. */
        CO_API void addListener( ConnectionPtr connection );

        /** Remove listening connections from this listening node. */
        CO_API void removeListeners( const Connections& connections );

        /**
         * Flush all pending commands on this listening node.
         *
         * This causes the receiver thread to redispatch all pending commands,
         * which are normally only redispatched when a new command is received.
         */
        CO_API void flushCommands();

        /** @internal Clone the given command. */
        CO_API Command& cloneCommand( Command& command );

        /** @internal Allocate a local command from the receiver thread. */
        CO_API Command& allocCommand( const uint64_t size );

        /** 
         * Dispatches a packet to the registered command queue.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa Command::invoke
         */
        CO_API bool dispatchCommand( Command& command );

        /**
         * Acquire a singular send token from the given node.
         * @return The send token to release.
         */
        CO_API SendToken acquireSendToken( NodePtr toNode );
        CO_API void releaseSendToken( SendToken& token );
        //@}

        /** @internal Ack an operation to the sender. */
        CO_API void ackRequest( NodePtr node, const uint32_t requestID );

        /** Request keep-alive update from the remote node. */
        CO_API void ping( NodePtr remoteNode );

        /**
         * Request updates from all nodes above keep-alive timeout.
         *
         * @return true if at least one ping was send.
         */
        CO_API bool pingIdleNodes();

        /** Set the affinity for this, the receiver and the command thread. */
        CO_API void setAffinity( const int32_t affinity );

    protected:
        /**  @internal
         * Connect a node proxy to this node.
         *
         * This node has to be in the listening state. The node proxy will be
         * put in the connected state upon success. The connection has to be
         * connected.
         *
         * @param node the remote node.
         * @param connection the connection to the remote node.
         * @return <code>true</code> if the node was connected correctly,
         *         <code>false</code> otherwise.
         */
        CO_API bool connect( NodePtr node, ConnectionPtr connection );

        /** Notify remote node disconnection */
        virtual void notifyDisconnect( NodePtr node ) { }

    private:
        detail::LocalNode* const _impl;

        bool _connectSelf();
        void _connectMulticast( NodePtr node );

        bool _startCommandThread();
        bool _notifyCommandThreadIdle();
        friend class detail::ReceiverThread;
        friend class detail::CommandThread;

        void _cleanup();
        CO_API void _addConnection( ConnectionPtr connection );
        void _removeConnection( ConnectionPtr connection );

        NodePtr _connect( const NodeID& nodeID, NodePtr peer );
        uint32_t _removeListenerNB( ConnectionPtr connection );
        uint32_t _connect( NodePtr node );
        uint32_t _connect( NodePtr node, ConnectionPtr connection );

        void _runReceiverThread();
        void   _handleConnect();
        void   _handleDisconnect();
        bool   _handleData();

        friend class ObjectStore;
        template< typename T > void
        _registerCommand( const uint32_t command, const CommandFunc< T >& func,
                          CommandQueue* destinationQueue )
        {
            registerCommand( command, func, destinationQueue );
        }

        void _dispatchCommand( Command& command );
        void   _redispatchCommands();

        /** The command functions. */
        bool _cmdAckRequest( Command& packet );
        bool _cmdStopRcv( Command& command );
        bool _cmdStopCmd( Command& command );
        bool _cmdSetAffinity( Command& command );
        bool _cmdConnect( Command& command );
        bool _cmdConnectReply( Command& command );
        bool _cmdConnectAck( Command& command );
        bool _cmdID( Command& command );
        bool _cmdDisconnect( Command& command );
        bool _cmdGetNodeData( Command& command );
        bool _cmdGetNodeDataReply( Command& command );
        bool _cmdAcquireSendToken( Command& command );
        bool _cmdAcquireSendTokenReply( Command& command );
        bool _cmdReleaseSendToken( Command& command );
        bool _cmdAddListener( Command& command );
        bool _cmdRemoveListener( Command& command );
        bool _cmdPing( Command& command );
        bool _cmdDiscard( Command& ) { return true; }
        //@}

        LB_TS_VAR( _cmdThread );
        LB_TS_VAR( _rcvThread );
    };
    inline std::ostream& operator << ( std::ostream& os, const LocalNode& node )
    {
        os << static_cast< const Node& >( node );
        return os;
    }
}
#endif // CO_LOCALNODE_H
