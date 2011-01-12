
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

#ifndef CO_LOCALNODE_H
#define CO_LOCALNODE_H

#include <co/node.h>            // base class
#include <co/base/requestHandler.h> // base class

#include <co/commandCache.h>    // member
#include <co/commandQueue.h>    // member
#include <co/connectionSet.h>   // member
#include <co/objectVersion.h>   // used in inline method


#include <co/base/lockable.h>       // member
#include <co/base/spinLock.h>       // member
#include <co/base/types.h>          // member

#pragma warning(push)
#pragma warning(disable: 4190)
extern "C" EQSERVER_EXPORT co::ConnectionPtr eqsStartLocalServer( const
                                                                 std::string& );
extern "C" EQSERVER_EXPORT void eqsJoinLocalServer();
#pragma warning(pop)

namespace co
{
    class ObjectStore;

    /** 
     * Specialization of a local node.
     *
     * Local nodes listen on network connections, manage connections to other
     * nodes and provide session registration, mapping and command dispatch.
     */
    class LocalNode : public co::base::RequestHandler
                    , public Node
    {
    public:
        CO_API LocalNode( );
        CO_API virtual ~LocalNode( );

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
         * Before calling listen(), the '--eq-listen &lt;connection
         * description&gt;' command line options is recognized by this method to
         * add listening connections to this node. This parameter might be used
         * multiple times. ConnectionDescription::fromString() is used to parse
         * the provided description.
         *
         * Please note that further command line parameters are recognized by
         * eq::init().
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

        /** 
         * Close a listening node.
         */
        virtual bool exitLocal() { return close(); }

        /**
         * Connect a proxy node to this listening node.
         *
         * The connection descriptions of the node are used to connect the
         * node. On success, the node is in the connected state, otherwise its
         * state is unchanged.
         *
         * This method is one-sided, that is, the node to be connected should
         * not initiate a connection to this node at the same time.
         *
         * @param node the remote node.
         * @return true if this node was connected, false otherwise.
         * @sa initConnect, syncConnect
         */
        CO_API bool connect( NodePtr node );

        /** 
         * Create and connect a node given by an identifier.
         *
         * This method is two-sided and thread-safe, that is, it can be called
         * by mulltiple threads on the same node with the same nodeID, or
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
        /** Disable the instance cache of an stopped local node. */
        CO_API void disableInstanceCache();

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
        CO_API bool mapObject( Object* object, const co::base::UUID& id, 
                                  const uint128_t& version = VERSION_OLDEST );

        /** Convenience wrapper for mapObject(). */
        bool mapObject( Object* object, const ObjectVersion& v )
            { return mapObject( object, v.identifier, v.version ); }

        /** Start mapping a distributed object. */
        CO_API uint32_t mapObjectNB( Object* object, const co::base::UUID& id, 
                                    const uint128_t& version = VERSION_OLDEST );
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
        void getNodes( Nodes& nodes ) const;

        CO_API void acquireSendToken( NodePtr toNode );
        CO_API void releaseSendToken( NodePtr toNode );

        /** Return the command queue to the command thread. */
        virtual CommandQueue* getCommandThreadQueue() 
            { return &_commandThreadQueue; }

        /** 
         * @return true if executed from the command handler thread, false if
         *         not.
         */
        bool inCommandThread() const  { return _commandThread->isCurrent(); }
        //@}

        /** @name Operations */
        //@{
        /** Add a listening connection to this listening node. */
        CO_API void addListener( ConnectionPtr connection );

        /** Remove a listening connection from this listening node. */
        CO_API uint32_t removeListenerNB( ConnectionPtr connection );

        /**
         * Flush all pending commands on this listening node.
         *
         * This causes the receiver thread to redispatch all pending commands,
         * which are normally only redispatched when a new command is received.
         */
        void flushCommands() { _incoming.interrupt(); }

        /** @internal Clone the given command. */
        Command& cloneCommand( Command& command )
            { return _commandCache.clone( command ); }

        /** 
         * Dispatches a packet to the registered command queue.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa Command::invoke
         */
        CO_API bool dispatchCommand( Command& command );
        //@}

        /** @internal ack an operation to the sender. */
        CO_API void ackRequest( NodePtr node, const uint32_t requestID );

        /** @internal */
        CO_API void expireInstanceData( const int64_t age );

    protected:
        /** 
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
         * @internal
         */
        CO_API bool _connect( NodePtr node, ConnectionPtr connection );

    private:
        /** Commands re-scheduled for dispatch. */
        CommandList  _pendingCommands;
    
        /** The command 'allocator' */
        CommandCache _commandCache;

        /** The receiver->command command queue. */
        CommandQueue _commandThreadQueue;        
    
        /** true if the send token can be granted, false otherwise. */
        bool _hasSendToken;
        std::deque< Command* > _sendTokenQueue;

        /** Manager of distributed object */
        ObjectStore* _objectStore;

        /** Needed for thread-safety during nodeID-based connect() */
        co::base::Lock _connectMutex;
    
        /** The node for each connection. */
        typedef co::base::RefPtrHash< Connection, NodePtr > ConnectionNodeHash;
        ConnectionNodeHash _connectionNodes; // read and write: recv only

        /** The connected nodes. */
        typedef stde::hash_map< uint128_t, NodePtr > NodeHash;
        // r: all, w: recv
        co::base::Lockable< NodeHash, co::base::SpinLock > _nodes; 

        /** The connection set of all connections from/to this node. */
        ConnectionSet _incoming;
    
        friend EQSERVER_EXPORT 
        co::ConnectionPtr (::eqsStartLocalServer( const std::string& ));

        /** @name Receiver management */
        //@{
        /** The receiver thread. */
        class ReceiverThread : public co::base::Thread
        {
        public:
            ReceiverThread( LocalNode* localNode ) : _localNode( localNode ){}
            virtual bool init()
                {
                    setDebugName( std::string( "Rcv " ) +
                        co::base::className( _localNode ));
                    return _localNode->_commandThread->start();
                }
            virtual void run(){ _localNode->_runReceiverThread(); }

        private:
            LocalNode* const _localNode;
        };
        ReceiverThread* _receiverThread;

        bool _connectSelf();
        void _connectMulticast( NodePtr node );

        void _cleanup();
        CO_API void _addConnection( ConnectionPtr connection );
        void _removeConnection( ConnectionPtr connection );
        NodePtr _connect( const NodeID& nodeID, NodePtr server );

        /** 
         * @return <code>true</code> if executed from the command handler
         *         thread, <code>false</code> if not.
         */
        bool _inReceiverThread() const { return _receiverThread->isCurrent(); }

        void _receiverThreadStart() { _receiverThread->start(); }

        void _runReceiverThread();
        void   _handleConnect();
        void   _handleDisconnect();
        bool   _handleData();
        //@}

        friend class ObjectStore;
        template< typename T >
        void _registerCommand( const uint32_t command,
                               const CommandFunc< T >& func,
                               CommandQueue* destinationQueue )
        {
            registerCommand( command, func, destinationQueue );
        }

        /**
         * @name Command management
         */
        //@{
        /** The command handler thread. */
        class CommandThread : public co::base::Thread
        {
        public:
            CommandThread( LocalNode* localNode ) : _localNode( localNode ){}
            virtual bool init()
                {
                    setDebugName( std::string( "Cmd " ) +
                                  co::base::className( _localNode ));
                    return true;
                }
            virtual void run(){ _localNode->_runCommandThread(); }
        private:
            LocalNode* const _localNode;
        };
        CommandThread* _commandThread;

        void _dispatchCommand( Command& command );
        void _runCommandThread();
        void   _redispatchCommands();

        /** The command functions. */
        bool _cmdStop( Command& command );
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
        //@}

        EQ_TS_VAR( _cmdThread );
        EQ_TS_VAR( _recvThread );
    };
    inline std::ostream& operator << ( std::ostream& os, const LocalNode& node )
    {
        os << static_cast< const Node& >( node );
        return os;
    }
}
#endif // CO_LOCALNODE_H
