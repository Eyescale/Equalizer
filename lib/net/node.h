
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

#ifndef EQNET_NODE_H
#define EQNET_NODE_H

#include <eq/net/dispatcher.h>               // base class
#include <eq/net/commandCache.h>             // member
#include <eq/net/commandQueue.h>             // member
#include <eq/net/connectionSet.h>            // member
#include <eq/net/nodeType.h>                 // for TYPE_EQNET_NODE enum
#include <eq/net/types.h>

#include <eq/base/base.h>
#include <eq/base/lockable.h>
#include <eq/base/perThread.h>
#include <eq/base/requestHandler.h>
#include <eq/base/spinLock.h>
#include <eq/base/thread.h>

#include <list>

#pragma warning(disable : 4190)
extern "C" EQSERVER_EXPORT eq::net::ConnectionPtr eqsStartLocalServer( const
                                                                 std::string& );
extern "C" EQSERVER_EXPORT void                   eqsJoinLocalServer();
#pragma warning(default : 4190)

namespace eq
{
namespace net
{
    class Command;
    class ConnectionDescription;
    class Session;

    /**
     * Manages a node.
     *
     * A node represents a separate entity in a peer-to-peer network, typically
     * a process on a cluster node or on a shared-memory system. It should have
     * at least one Connection through which is reachable. A Node provides the
     * basic communication facilities through message passing. Nodes manage
     * sessions, that is, one or more Session can be mapped to a Node, in which
     * case the node will dispatch packets to these sessions.
     */
    class Node : public Dispatcher, public base::RequestHandler,
                 public base::Referenced
    {
    public:
        /** The state of the node. */
        enum State 
        {
            STATE_STOPPED,   //!< initial state
            STATE_LAUNCHED,  //!< proxy for a remote node, launched
            STATE_CONNECTED, //!< proxy for a remote node, connected  
            STATE_LISTENING  //!< local node, listening
        };

        /** Construct a new Node. */
        EQ_EXPORT Node();

        /** @name Data Access. */
        //@{
        bool operator == ( const Node* n ) const;

        /**  @return the state of this node. */
        State getState()    const { return _state; }
        bool  isConnected() const 
            { return (_state == STATE_CONNECTED || _state == STATE_LISTENING); }

        /** 
         * Get a node by identifier.
         *
         * The node might not be connected.
         *
         * @param id the node identifier.
         * @return the node.
         */
        NodePtr getNode( const NodeID& id ) const;
        //@}

        /** @ Auto-launch parameters. */
        //@{
        /** 
         * Set the command to spawn the process for this node.
         *
         * The default is '"ssh -n %h %c >& %h.%n.log"', with:
         * 
         * %h - hostname
         * %c - command (work dir + program name)
         * %n - unique node identifier
         */
        EQ_EXPORT void setLaunchCommand( const std::string& launchCommand );

        /** @return the command to spawn the process for this node. */
        EQ_EXPORT const std::string& getLaunchCommand() const;

        /** Set the launch timeout in milliseconds. */
        void setLaunchTimeout( const uint32_t time ) { _launchTimeout = time; }

        /** @return the current launch timeout in milliseconds. */
        uint32_t getLaunchTimeout() const { return _launchTimeout; }

        /** Set the quote charactor for the launch command arguments */
        void setLaunchCommandQuote( const char quote )
            { _launchCommandQuote = quote; }

        /* @return the quote charactor for the launch command arguments */
        char getLaunchCommandQuote() const { return _launchCommandQuote; }

        /**
         * Set the program name to start this node.
         * 
         * @param name the program name to start this node.
         */
        EQ_EXPORT void setProgramName( const std::string& name );

        /** @return the program name to start the node. */
        const std::string& getProgramName() const { return _programName; }

        /** 
         * Set the working directory to start this node.
         * 
         * @param name the working directory to start this node.
         */
        EQ_EXPORT void setWorkDir( const std::string& name );

        /** @return the working directory to start the node. */
        const std::string& getWorkDir() const { return _workDir; }

        /** 
         * Set if this node should be launched automatically.
         *
         * This determines if the launch command is used to start the node when
         * it can not be reached using its connections. The default is false.
         */
        void setAutoLaunch( const bool autoLaunch ) { _autoLaunch = autoLaunch;}
        //@}

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
         * The following command line options are recognized by this method:
         * <ul>
         *   <li>--eq-client to launch a client. This is used for remote nodes
         *         which have  been auto-launched by another node, e.g., remote
         *         render clients. This method does not return when this command
         *         line option is present.</li>
         *   <li>'--eq-listen &lt;connection description&gt;' to add listening
         *         connections to this node. This parameter might be used
         *         multiple times (cf. ConnectionDescription::fromString).</li>
         * </ul>
         *
         * Please note that further command line parameters are recognized by
         * eq::init().
         *
         * @param argc the command line argument count.
         * @param argv the command line argument values.
         * @return <code>true</code> if the client was successfully initialized,
         *         <code>false</code> otherwise.
         */
        EQ_EXPORT virtual bool initLocal( const int argc, char** argv );

        /** Exit a local, listening node. */
        virtual bool exitLocal() { return close(); }

        /** Exit a client node. */
        virtual bool exitClient() { return exitLocal(); }

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
        EQ_EXPORT virtual bool listen();

        /** 
         * Close a listening node.
         * 
         * Disconnects all connected node proxies, closes the listening
         * connections and terminates all threads created in listen().
         * 
         * @return <code>true</code> if the node was stopped, <code>false</code>
         *         if it was not stopped.
         */
        EQ_EXPORT virtual bool close();

        /** 
         * Connect a proxy node to this listening node.
         *
         * The connection descriptions of the node are used to connect the
         * node. If this fails, and auto launch is true, the node is started
         * using the launch command.
         *
         * On success, the node is in the connected state, otherwise its state
         * is unchanged.
         *
         * This method is one-sided, that is, the node to be connected should
         * not initiate a connection to this node at the same time.
         *
         * @param node the remote node.
         * @return true if this node was connected, false otherwise.
         * @sa initConnect, syncConnect
         */
        EQ_EXPORT bool connect( NodePtr node );

        /** 
         * Start connecting a node using the available connection descriptions.
         *
         * The connection descriptions of the node are used to connect the
         * node. If this fails, and auto launch is true, the node is started
         * using the launch command.
         *
         * On success, the node is in the launched or connected state, otherwise
         * its state is unchanged.
         *
         * @param node the remote node.
         * @return <code>true</code> if this node is connecting,
         *         <code>false</code> otherwise.
         * @sa syncConnect
         */
        EQ_EXPORT bool initConnect( NodePtr node );

        /** 
         * Synchronize the connection initiated by initConnect().
         *
         * On success, the node is in the connected state, otherwise its state
         * is unchanged.
         *
         * @param node the remote node.
         * @param timeout the timeout, in milliseconds, before the launch
         *                process is considered to have failed.
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         * @sa initConnect
         */
        EQ_EXPORT bool syncConnect( NodePtr node, const uint32_t timeout );

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
        NodePtr connect( const NodeID& nodeID );

        /** 
         * Disconnects a connected node.
         *
         * @param node the remote node.
         * @return <code>true</code> if the node was disconnected correctly,
         *         <code>false</code> otherwise.
         */
        EQ_EXPORT bool close( NodePtr node );
        //@}


        /**
         * @name Connectivity information. 
         */
        //@{
        /** 
         * Returns if the node is local.
         * 
         * @return <code>true</code> if the node is local, <code>false</code>
         *         otherwise.
         */
        bool isLocal() const { return (_state==STATE_LISTENING); }

        /** 
         * Adds a new description how this node can be reached.
         * 
         * @param cd the connection description.
         */
        EQ_EXPORT void addConnectionDescription( ConnectionDescriptionPtr cd );
        
        /** 
         * Removes a connection description.
         * 
         * @param cd the connection description.
         * @return true if the connection description was removed, false
         *         otherwise.
         */
        EQ_EXPORT bool removeConnectionDescription(ConnectionDescriptionPtr cd);

        /** 
         * Returns the number of stored connection descriptions. 
         * 
         * @return the number of stored connection descriptions. 
         */
        EQ_EXPORT const ConnectionDescriptionVector& getConnectionDescriptions()
                            const;

        /** @return the connection to this node. */
        ConnectionPtr getConnection() const { return _outgoing; }

        /** @return the multicast connection to this node, or 0. */
        ConnectionPtr getMulticast();
        //@}

        /**
         * @name Messaging API
         */
        //@{
        /** 
         * Sends a packet to this node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( const Packet& packet )
            {
                ConnectionPtr connection = _getConnection();
                if( !connection )
                    return false;
                return connection->send( packet );
            }

        /** 
         * Sends a packet with a string to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the string, so that it is received as one packet by the node.
         *
         * It is assumed that the last 8 bytes in the packet are usable for the
         * string.  This is used for optimising the send of short strings and on
         * the receiver side to access the string. The node implementation gives
         * examples of this usage.
         *
         * @param packet the packet.
         * @param string the string.
         * @return the success status of the transaction.
         */
        bool send( Packet& packet, const std::string& string )
            {
                ConnectionPtr connection = _getConnection();
                if( !connection )
                    return false;
                return connection->send( packet, string );
            }

        /** 
         * Sends a packet with additional data to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the string, so that it is received as one packet by the node.
         *
         * It is assumed that the last item in the packet is of sizeof(T) and
         * usable for the data.
         *
         * @param packet the packet.
         * @param data the vector containing the data.
         * @return the success status of the transaction.
         */
        template< class T >
        bool send( Packet& packet, const std::vector<T>& data )
            {
                ConnectionPtr connection = _getConnection();
                if( !connection )
                    return false;
                return connection->send( packet, data );
            }

        /** 
         * Sends a packet with additional data to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the data, so that it is received as one packet by the node.
         *
         * It is assumed that the last 8 bytes in the packet are usable for the
         * data.  This is used for optimising the send of short data and on
         * the receiver side to access the data. The node implementation gives
         * examples of this usage.
         *
         * @param packet the packet.
         * @param data the data.
         * @param size the size of the data in bytes.
         * @return the success status of the transaction.
         */
        bool send( Packet& packet, const void* data, const uint64_t size )
            {
                ConnectionPtr connection = _getConnection();
                if( !connection )
                    return false;
                return connection->send( packet, data, size );
            }

        /** 
         * Multicasts a packet to the multicast group of this node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool multicast( const Packet& packet )
            {
                ConnectionPtr connection = getMulticast();
                if( !connection )
                    return false;
                return connection->send( packet );
            }

        /**
         * Flush all pending commands on this listening node.
         *
         * This causes the receiver thread to redispatch all pending commands,
         * which are normally only redispatched when a new command is received.
         */
        void flushCommands() { _incoming.interrupt(); }

        void acquireSendToken( NodePtr toNode );
        void releaseSendToken( NodePtr toNode );
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
        EQ_EXPORT void registerSession( Session* session );

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
        bool mapSession( NodePtr server, Session* session, 
                         const SessionID& id );

        /** 
         * Unmaps a mapped session.
         * 
         * @param session the session.
         * @return <code>true</code> if the session was unmapped,
         *         <code>false</code> if there was an error.
         */
        EQ_EXPORT bool unmapSession( Session* session );

        /** @return the mapped session with the given identifier, or 0. */
        Session* getSession( const SessionID& id );

        bool hasSessions() const { return !_sessions->empty(); }
        //@}

        /** 
         * Runs this node as a client to a server.
         * 
         * @param clientArgs the client arguments as specified by the server.
         * @return the success value of the run.
         */
        EQ_EXPORT virtual bool runClient( const std::string& clientArgs );

        /** Return the command queue to the command thread. */
        CommandQueue* getCommandThreadQueue() 
            { EQASSERT( isLocal( )); return &_commandThreadQueue; }

        /** 
         * @return <code>true</code> if executed from the command handler
         *         thread, <code>false</code> if not.
         */
        bool inCommandThread() const  { return _commandThread->isCurrent(); }
        bool inReceiverThread() const { return _receiverThread->isCurrent(); }

        const NodeID& getNodeID() const { return _id; }

        /** Serialize the node's information. */
        EQ_EXPORT std::string serialize() const;
        /** Deserialize the node information, consumes given data. */
        EQ_EXPORT bool deserialize( std::string& data );

    protected:
        /** Destructs this node. */
        EQ_EXPORT virtual ~Node();

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
        bool _connect( NodePtr node, ConnectionPtr connection );

        /** 
         * Dispatches a packet to the registered command queue.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa invokeCommand
         */
        EQ_EXPORT virtual bool dispatchCommand( Command& command );

        /** 
         * Invokes the command handler method for the packet.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa Dispatcher::invokeCommand
         */
        EQ_EXPORT virtual CommandResult invokeCommand( Command& command );

        /** 
         * The main loop for auto-launched clients. 
         *
         * @sa runClient()
         */
        virtual bool clientLoop() { return true; }

        /** @return the type of the node, used during connect(). */
        virtual uint32_t getType() const { return TYPE_EQNET_NODE; }

        /** 
         * Factory method to create a new node.
         * 
         * @param type the type the node type
         * @return the node.
         * @sa getType()
         */
        EQ_EXPORT virtual NodePtr createNode( const uint32_t type );

    private:
        /** Globally unique node identifier. */
        NodeID _id;

        /** The current state of this node. */
        State _state;

        typedef base::UUIDHash< Session* > SessionHash;
        /** The current mapped sessions of this node. */
        base::Lockable< SessionHash, base::SpinLock > _sessions;

        /** The connection to this node. */
        ConnectionPtr _outgoing;

        /** The multicast connection to this node, can be 0. */
        base::Lockable< ConnectionPtr > _outMulticast;

        /** The connection set of all connections from/to this node. */
        ConnectionSet _incoming;
        friend net::ConnectionPtr (::eqsStartLocalServer( const std::string& ));

        struct MCData
        {
            ConnectionPtr connection;
            NodePtr       node;
        };
        typedef std::vector< MCData > MCDatas;

        /** 
         * Unused multicast connections for this node.
         *
         * On the first multicast send usage, the connection is 'primed' by
         * sending our node identifier to the MC group, removed from this vector
         * and set as _outMulticast.
         */
        MCDatas _multicasts;

        typedef base::UUIDHash< NodePtr > NodeHash;

        /** The connected nodes. */
        base::Lockable< NodeHash > _nodes; // read: all, write: recv only

        /** The node for each connection. */
        typedef base::RefPtrHash< Connection, NodePtr > ConnectionNodeHash;
        ConnectionNodeHash _connectionNodes; // read and write: recv only

        /** The receiver->command command queue. */
        CommandQueue _commandThreadQueue;

        /** Needed for thread-safety during nodeID-based connect() */
        base::Lock _connectMutex;

        /** Determines if the node should be launched automatically. */
        bool _autoLaunch;

        /** The request id for the async launch operation. */
        uint32_t _launchID;

        /** 
         * The amount of time in milliseconds to wait before a node is
         * considered unreachable during start.
         */
        int32_t _launchTimeout;

        /** The character to quote the launch command arguments */
        char _launchCommandQuote;

        /** Command to launch node process. */
        std::string _launchCommand; 

        /** Commands re-scheduled for dispatch. */
        CommandList  _pendingCommands;

        /** The command 'allocator' */
        CommandCache _commandCache;

        /** The list of descriptions on how this node is reachable. */
        ConnectionDescriptionVector _connectionDescriptions;

        /** The name of the program to autolaunch. */
        std::string _programName;
        /** The directory of the program to autolaunch. */
        std::string _workDir;

        /** The receiver thread. */
        class ReceiverThread : public base::Thread
        {
        public:
            ReceiverThread( Node* node ) 
                    : _node( node )
                {}
            
            virtual bool init(){ return _node->_commandThread->start(); }
            virtual void run(){ _node->_runReceiverThread(); }

        private:
            Node* _node;
        };
        ReceiverThread* _receiverThread;

        /** The command handler thread. */
        class CommandThread : public base::Thread
        {
        public:
            CommandThread( Node* node ) 
                    : _node( node )
                {}
            
            virtual void run(){ _node->_runCommandThread(); }

        private:
            Node* _node;
        };
        CommandThread* _commandThread;

        /** true if the send token can be granted, false otherwise. */
        bool _hasSendToken;
        std::deque< Command* > _sendTokenQueue;

        bool _connectSelf();
        void _connectMulticast( NodePtr node );
        EQ_EXPORT void _addConnection( ConnectionPtr connection );
        void _removeConnection( ConnectionPtr connection );
        void _cleanup();

        void _dispatchCommand( Command& command );

        /** 
        * Launch a node using the parameters from the connection
        * description.
        * 
        * @param description the connection description.
        * @return <code>true</code> if the node was launched,
        *         <code>false</code> otherwise.
        */
        bool _launch( NodePtr node, ConnectionDescriptionPtr description );

        /** 
        * Composes the launch command by expanding the variables in the
        * description's launch command string.
        * 
        * @param description the connection description.
        * @param requestID the request identifier to be used by the launched
        *                  node when connecting to this node.
        * @return the expanded launch command.
        */
        std::string _createLaunchCommand( NodePtr node,
                                          ConnectionDescriptionPtr description);
        std::string   _createRemoteCommand( NodePtr node, const char quote );

        /** 
         * Find a connected node using a connection description
         * 
         * @param connectionDescription the connection description for the node.
         * @return the node, or an invalid pointer if no node was found.
         */
        NodePtr _findConnectedNode( const char* connectionDescription );

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

        NodePtr _connect( const NodeID& nodeID, NodePtr server );

        /** Ensures the connectivity of this node. */
        ConnectionPtr _getConnection()
            {
                ConnectionPtr connection = _outgoing;
                if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
                    return connection;
                return 0;
            }

        void _runReceiverThread();
        void   _handleConnect();
        void   _handleDisconnect();
        bool   _handleData();

        void _runCommandThread();
        void   _redispatchCommands();

        /** The command functions. */
        CommandResult _cmdStop( Command& command );
        CommandResult _cmdRegisterSession( Command& command );
        CommandResult _cmdRegisterSessionReply( Command& command );
        CommandResult _cmdMapSession( Command& command );
        CommandResult _cmdMapSessionReply( Command& command );
        CommandResult _cmdUnmapSession( Command& command );
        CommandResult _cmdUnmapSessionReply( Command& command );
        CommandResult _cmdConnect( Command& command );
        CommandResult _cmdConnectReply( Command& command );
        CommandResult _cmdConnectAck( Command& command );
        CommandResult _cmdID( Command& command );
        CommandResult _cmdDisconnect( Command& command );
        CommandResult _cmdGetNodeData( Command& command );
        CommandResult _cmdGetNodeDataReply( Command& command );
        CommandResult _cmdAcquireSendToken( Command& command );
        CommandResult _cmdAcquireSendTokenReply( Command& command );
        CommandResult _cmdReleaseSendToken( Command& command );

        CHECK_THREAD_DECLARE( _cmdThread );
        CHECK_THREAD_DECLARE( _recvThread );
    };

    inline std::ostream& operator << ( std::ostream& os, const Node* node )
    {
        if( node )
            os << "node " << node->getNodeID();
        else
            os << "NULL node";
        
        return os;
    }

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const Node::State state );
}
}

#endif // EQNET_NODE_H
