
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQNET_LOCALNODE_H
#define EQNET_LOCALNODE_H

#include <eq/net/node.h>            // base class
#include <eq/base/requestHandler.h> // base class

#include <eq/net/commandCache.h>    // member
#include <eq/net/commandQueue.h>    // member
#include <eq/net/connectionSet.h>   // member

#include <eq/base/lockable.h>       // member
#include <eq/base/spinLock.h>       // member
#include <eq/base/types.h>          // member

#pragma warning(disable : 4190)
extern "C" EQSERVER_EXPORT eq::net::ConnectionPtr eqsStartLocalServer( const
                                                                 std::string& );
extern "C" EQSERVER_EXPORT void                   eqsJoinLocalServer();
#pragma warning(default : 4190)

namespace eq
{
namespace net
{

    class Session;

    /** 
     * Specialization of a local node.
     *
     * Local nodes listen on network connections, manage connections to other
     * nodes and provide session registration, mapping and command dispatch.
     */
    class LocalNode : public base::RequestHandler
                    , public Node
    {
    public:
        EQ_NET_DECL LocalNode( );
        EQ_NET_DECL virtual ~LocalNode( );

        /**
         * @name State Changes
         *
         * The following methods affect the state of the node by changing its
         * connectivity to the network.
         */
        //@{
        /** 
         * Initialize a local, listening node.
         *
         * The '--eq-listen &lt;connection description&gt;' command line options
         * is recognized by this method to add listening connections to this
         * node. This parameter might be used multiple
         * times. ConnectionDescription::fromString() is used to parse the
         * provided description.
         *
         * Please note that further command line parameters are recognized by
         * eq::init().
         *
         * @param argc the command line argument count.
         * @param argv the command line argument values.
         * @return <code>true</code> if the client was successfully initialized,
         *         <code>false</code> otherwise.
         */
        EQ_NET_DECL virtual bool initLocal( const int argc, char** argv );
        
        /** 
         * Close the connection.
         */
        virtual bool exitLocal() { return close(); }

        /**
         * Open all connections and put this node into the listening state.
         *
         * The node will spawn a receiver and command thread, and listen on all
         * connections described for incoming commands. The node will be in the
         * listening state if the method completed successfully. A listening
         * node can connect other nodes.
         * 
         * @return <code>true</code> if the node could be initialized,
         *         <code>false</code> if not.
         * @sa connect
         */
        EQ_NET_DECL virtual bool listen();

        /**
         * Close a listening node.
         * 
         * Disconnects all connected node proxies, closes the listening
         * connections and terminates all threads created in listen().
         * 
         * @return <code>true</code> if the node was stopped, <code>false</code>
         *         if it was not stopped.
         */
        EQ_NET_DECL virtual bool close();

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
        EQ_NET_DECL bool connect( NodePtr node );

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
        EQ_NET_DECL NodePtr connect( const NodeID& nodeID );

        /** 
         * Disconnects a connected node.
         *
         * @param node the remote node.
         * @return <code>true</code> if the node was disconnected correctly,
         *         <code>false</code> otherwise.
         */
        EQ_NET_DECL virtual bool disconnect( NodePtr node );
        //@}

        /**
         * @name Session management
         */
        //@{
        /**
         * Register a new session using this node as the session server.
         *
         * This method assigns the session identifier. The node has to be local.
         *
         * @param session the session.
         */
        EQ_NET_DECL void registerSession( Session* session );

        /** Deregister a (master) session. */
        bool deregisterSession( Session* session )
            { return unmapSession( session ); }

        /**
         * Maps a local session object to the session of the same identifier on
         * the server.
         *
         * The node has to be a remote node.
         * 
         * @param server the node serving the session.
         * @param session the session.
         * @param id the identifier of the session.
         * @return <code>true</code> if the session was mapped,
         *         <code>false</code> if not.
         */
        EQ_NET_DECL bool mapSession( NodePtr server, Session* session, 
                                     const SessionID& id );

        /** 
         * Unmaps a mapped session.
         * 
         * @param session the session.
         * @return <code>true</code> if the session was unmapped,
         *         <code>false</code> if there was an error.
         */
        EQ_NET_DECL bool unmapSession( Session* session );

        /** @return the mapped session with the given identifier, or 0. */
        EQ_NET_DECL Session* getSession( const SessionID& id );

        bool hasSessions() const { return !_sessions->empty(); }

        //@}

        /** 
         * Get a node by identifier.
         *
         * The node might not be connected.
         *
         * @param id the node identifier.
         * @return the node.
         */
        EQ_NET_DECL NodePtr getNode( const NodeID& id ) const;

        /** Assemble a vector of the currently connected nodes. */
        void getNodes( Nodes& nodes ) const;

        EQ_NET_DECL void acquireSendToken( NodePtr toNode );
        EQ_NET_DECL void releaseSendToken( NodePtr toNode );

        /** Return the command queue to the command thread. */
        virtual CommandQueue* getCommandThreadQueue() 
            { return &_commandThreadQueue; }

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
         * Invokes the command handler method for the packet.
         * 
         * @param command the command.
         * @return true if the result of the operation is handled.
         * @sa Dispatcher::invokeCommand
         */
        EQ_NET_DECL bool invokeCommand( Command& command );

        /** 
         * Dispatches a packet to the registered command queue.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa invokeCommand
         */
        EQ_NET_DECL bool dispatchCommand( Command& command );
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
        EQ_NET_DECL bool _connect( NodePtr node, ConnectionPtr connection );

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

        typedef base::UUIDHash< Session* > SessionHash;
        /** The current mapped sessions of this node. */
        base::Lockable< SessionHash, base::SpinLock > _sessions;

        /** Needed for thread-safety during nodeID-based connect() */
        base::Lock _connectMutex;
    
        /** The node for each connection. */
        typedef base::RefPtrHash< Connection, NodePtr > ConnectionNodeHash;
        ConnectionNodeHash _connectionNodes; // read and write: recv only

        /** The connected nodes. */
        typedef base::UUIDHash< NodePtr > NodeHash;
        base::Lockable< NodeHash, base::SpinLock > _nodes; // r: all, w: recv

        /** The connection set of all connections from/to this node. */
        ConnectionSet _incoming;
    
        friend net::ConnectionPtr (::eqsStartLocalServer( const std::string& ));

        /**
         * @name Receiver management
         */
        //@{
        /** The receiver thread. */
        class ReceiverThread : public base::Thread
        {
        public:
            ReceiverThread( LocalNode* localNode ) : _localNode( localNode ){}
            virtual bool init()
                {
                    setDebugName( std::string("Rcv ") + base::className(_localNode));
                    return _localNode->_commandThread->start();
                }
            virtual void run(){ _localNode->_runReceiverThread(); }
        private:
            LocalNode* _localNode;
        };
        ReceiverThread* _receiverThread;

        bool _connectSelf();
        void _connectMulticast( NodePtr node );

        void _cleanup();
        EQ_NET_DECL void _addConnection( ConnectionPtr connection );
        void _removeConnection( ConnectionPtr connection );
        NodePtr _connect( const NodeID& nodeID, NodePtr server );

        /**
         * Adds an already mapped session to this node.
         * 
         * @param session the session.
         * @param server the node serving the session.
         * @param sessionID the identifier of the session.
         */
        void _addSession( Session* session, NodePtr server,
                          const SessionID& sessionID );

        /** 
         * Removes an unmapped session from this node.
         * 
         * @param session the session.
         */
        void _removeSession( Session* session );

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

        /**
         * @name Command management
         */
        //@{
        /** The command handler thread. */
        class CommandThread : public base::Thread
        {
        public:
            CommandThread( LocalNode* localNode ) : _localNode( localNode ){}
            virtual bool init()
                {
                    setDebugName( std::string("Cmd ") + base::className(_localNode));
                    return true;
                }
            virtual void run(){ _localNode->_runCommandThread(); }
        private:
            LocalNode* _localNode;
        };
        CommandThread* _commandThread;

        /** 
         * @return <code>true</code> if executed from the command handler
         *         thread, <code>false</code> if not.
         */
        bool _inCommandThread() const  { return _commandThread->isCurrent(); }

        void _dispatchCommand( Command& command );
        void _runCommandThread();
        void   _redispatchCommands();

        /** The command functions. */
        bool _cmdStop( Command& command );
        bool _cmdRegisterSession( Command& command );
        bool _cmdRegisterSessionReply( Command& command );
        bool _cmdMapSession( Command& command );
        bool _cmdMapSessionReply( Command& command );
        bool _cmdUnmapSession( Command& command );
        bool _cmdUnmapSessionReply( Command& command );
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
        //@}

        EQ_TS_VAR( _cmdThread );
        EQ_TS_VAR( _recvThread );
    };
}
}
#endif // EQNET_LOCALNODE_H
