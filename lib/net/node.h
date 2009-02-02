
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_H
#define EQNET_NODE_H

#include <eq/net/dispatcher.h>               // base class
#include <eq/net/commandQueue.h>             // member
#include <eq/net/connectionSet.h>            // member
#include <eq/net/nodeID.h>                   // member
#include <eq/net/nodeType.h>                 // for TYPE_EQNET_NODE enum

#include <eq/base/base.h>
#include <eq/base/perThread.h>
#include <eq/base/requestHandler.h>
#include <eq/base/thread.h>

#include <list>

#pragma warning(disable : 4190)
extern "C" EQSERVER_EXPORT eq::net::ConnectionPtr eqsStartLocalServer();
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
     * A node represents a separate entity in the peer-to-peer network,
     * typically a process on a cluster node or on a shared-memory system. It
     * has at least one Connection through which is reachable. A Node provides
     * the basic communication facilities through message passing.
     */
    class EQ_EXPORT Node : public Dispatcher, public base::Referenced
    {
    public:
        enum State 
        {
            STATE_STOPPED,   // initial               
            STATE_LAUNCHED,  // remote node, launched
            STATE_CONNECTED, // remote node, connected  
            STATE_LISTENING  // local node, listening
        };

        /** 
         * Constructs a new Node.
         */
        Node();

        /** @name Data Access. */
        //*{
        bool operator == ( const Node* n ) const;

        /** 
         * Returns the state of this node.
         * 
         * @return the state of this node.
         */
        State getState()    const { return _state; }
        bool  isConnected() const 
            { return (_state == STATE_CONNECTED || _state == STATE_LISTENING); }

        void setAutoLaunch( const bool autoLaunch ) { _autoLaunch = autoLaunch;}

        /** 
         * Set the program name to start this node.
         * 
         * @param name the program name to start this node.
         */
        void setProgramName( const std::string& name ) { _programName = name; }

        /** 
         * Set the working directory to start this node.
         * 
         * @param name the working directory to start this node.
         */
        void setWorkDir( const std::string& name ) { _workDir = name; }
        //*}

        /**
         * @name State Changes
         *
         * The following methods affect the state of the node by changing it's
         * connectivity to the network.
         */
        //*{
        /** 
         * Initialize a local, listening node.
         *
         * This function does not return when the command line option
         * '--eq-client' is present. This is used for remote nodes which have
         * been auto-launched by another node, e.g., remote render clients.
         *
         * @param argc the command line argument count.
         * @param argv the command line argument values.
         * @return <code>true</code> if the client was successfully initialized,
         *         <code>false</code> otherwise.
         */
        virtual bool initLocal( const int argc, char** argv );

        /** Exit a local, listening node. */
        virtual bool exitLocal() { return stopListening(); }

        /** Exit a client node. */
        virtual bool exitClient() { return exitLocal(); }

        /** 
         * Initializes this node.
         *
         * The node will spawn a thread locally and listen on all connections
         * described for incoming commands. The node will be in the listening
         * state if the method completed successfully. A listening node can
         * connect to other nodes.
         * 
         * @return <code>true</code> if the node could be initialized,
         *         <code>false</code> if not.
         * @sa connect
         */
        virtual bool listen();

        /** 
         * Stops this node.
         * 
         * If this node is listening, the node will stop listening and terminate
         * its receiver thread.
         * 
         * @return <code>true</code> if the node was stopped, <code>false</code>
         *         if it was not stopped.
         */
        virtual bool stopListening();

        /** 
         * Connects a node to this listening node.
         *
         * @param node the remote node.
         * @param connection the connection to the remote node.
         * @return <code>true</code> if the node was connected correctly,
         *         <code>false</code> otherwise.
         */
        bool connect( NodePtr node, 
                      ConnectionPtr connection );

        /** 
         * Get a node by identifier.
         * 
         * @param id the node identifier.
         * @return the node.
         */
        NodePtr getNode( const NodeID& id ) const;

        /** 
         * Connect and potentially launch a node to this listening node, using
         * the connection descriptions of the node.
         *
         * On success, the node is in the connected state, otherwise its state
         * is unchanged.
         *
         * @param node the remote node.
         * @return <code>true</code> if this node was connected,
         *         <code>false</code> otherwise.
         * @sa initConnect, syncConnect
         */
        bool connect( NodePtr node );

        /** 
         * Start connecting and potentially launching a node using the
         * available connection descriptions.
         *
         * On success, the node is in the launched or connected state, otherwise
         * its state is unchanged.
         *
         * @param node the remote node.
         * @return <code>true</code> if this node is connecting,
         *         <code>false</code> otherwise.
         * @sa syncConnect
         */
        bool initConnect( NodePtr node );

        /** 
         * Synchronize the connection initiated by initConnect().
         *
         * On success, the node is in the connected state, otherwise its state
         * is unchanged.
         *
         * @param node the remote node.
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         * @sa initConnect
         */
        bool syncConnect( NodePtr node );

        /** 
         * Create and connect a node given by an identifier.
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
        bool disconnect( NodePtr node );

        /** 
         * Ensures the connectivity of this node.
         *
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         */
        ConnectionPtr checkConnection()
            {
                ConnectionPtr connection = _connection;
                if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
                    return _connection;
                return 0;
            }
        //*}


        /**
         * @name Connectivity information. 
         */
        //*{
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
        void addConnectionDescription( ConnectionDescriptionPtr cd );
        
        /** 
         * Removes a connection description.
         * 
         * @param index the index of the connection description.
         */
        void removeConnectionDescription( const uint32_t index );

        /** 
         * Returns the number of stored connection descriptions. 
         * 
         * @return the number of stored connection descriptions. 
         */
        const ConnectionDescriptionVector& getConnectionDescriptions() const 
            { return _connectionDescriptions; }

        /** 
         * Returns the connection to this node.
         * 
         * @return the connection to this node. 
         */
        ConnectionPtr getConnection() const { return _connection; }
        //*}

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
                ConnectionPtr connection = checkConnection();
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
                ConnectionPtr connection = checkConnection();
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
                ConnectionPtr connection = checkConnection();
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
                ConnectionPtr connection = checkConnection();
                if( !connection )
                    return false;
                return connection->send( packet, data, size );
            }

        /**
         * Flush all pending commands on this listening node.
         *
         * This causes the receiver thread to redispatch all pending commands,
         * which are normally only redispatched when a new command is received.
         */
        void flushCommands() { _connectionSet.interrupt(); }

        void acquireSendToken( NodePtr toNode );
        void releaseSendToken( NodePtr toNode );
        //@}

        /**
         * @name Session management
         */
        //*{
        /**
         * Register a new session using this node as the session server.
         *
         * This method assigns the session identifier. The node has to be local.
         *
         * @param session the session.
         * @return <code>true</code> if the session was mapped,
         *         <code>false</code> if not.
         */
        bool registerSession( Session* session );

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
                         const uint32_t id );

        /** 
         * Unmaps a mapped session.
         * 
         * @param session the session.
         * @return <code>true</code> if the session was unmapped,
         *         <code>false</code> if there was an error.
         */
        bool unmapSession( Session* session );

        /** @return the mapped session with the given identifier, or 0. */
        Session* getSession( const uint32_t id );

        bool hasSessions() const { return !_sessions.empty(); }
        //*}

        /** 
         * Runs this node as a client to a server.
         * 
         * @param clientArgs the client arguments as specified by the server.
         * @return the success value of the run.
         */
        virtual bool runClient( const std::string& clientArgs );

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

    protected:
        /** Destructs this node. */
        virtual ~Node();

        /** 
         * Dispatches a packet to the registered command queue.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa invokeCommand
         */
        virtual bool dispatchCommand( Command& command );

        /** 
         * Invokes the command handler method for the packet.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa Dispatcher::invokeCommand
         */
        virtual CommandResult invokeCommand( Command& command );

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
        virtual NodePtr createNode( const uint32_t type )
        { EQASSERTINFO( type == TYPE_EQNET_NODE, type ); return new Node(); }

        /** Serialize the node's information. */
        std::string serialize() const;
        /** Deserialize the node information, consumes data. */
        bool deserialize( std::string& data );

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

    private:
        /** Determines if the node should be launched automatically. */
        bool _autoLaunch;

        /** Globally unique node identifier. */
        NodeID _id;

        /** The current state of this node. */
        State _state;

        /** The current mapped sessions of this node. */
        SessionHash _sessions;

        /** The connection to this node, for remote nodes. */
        ConnectionPtr _connection;

        /** The connection set of all connections from/to this node. */
        ConnectionSet _connectionSet;
        friend eq::net::ConnectionPtr (::eqsStartLocalServer());

        /** The connected nodes. */
        NodeIDHash< NodePtr > _nodes;

        /** The node for each connection. */
        typedef base::RefPtrHash< Connection, NodePtr > ConnectionNodeHash;
        ConnectionNodeHash _connectionNodes;

        /** The receiver->command command queue. */
        CommandQueue _commandThreadQueue;

        /** Needed for thread-safety during nodeID-based connect() */
        base::Lock _connectMutex;

        /** The cache to store the last received command for reuse */
        Command* _receivedCommand;

        /** The request id for the async launch operation. */
        uint32_t _launchID;

        /** The remaining timeout for the launch operation. */
        base::Clock _launchTimeout;

        /** Commands re-scheduled for dispatch. */
        std::list< Command* > _pendingCommands;
        CommandCache          _commandCache;

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
            
            virtual void* run(){ return _node->_runReceiverThread(); }

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
            
            virtual void* run(){ return _node->_runCommandThread(); }

        private:
            Node* _node;
        };
        CommandThread* _commandThread;

        /** true if the send token can be granted, false otherwise. */
        bool _hasSendToken;

        bool _listenToSelf();
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
        bool _launch( NodePtr node, 
            ConnectionDescriptionPtr description );

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
        std::string   _createRemoteCommand( NodePtr node, 
            const char quote );

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
                          const uint32_t sessionID );

        /** 
         * Removes an unmapped session from this node.
         * 
         * @param session the session.
         */
        void _removeSession( Session* session );

        /** Generates a new, unique session identifier. */
        uint32_t _generateSessionID();

        NodePtr _connect( const NodeID& nodeID, NodePtr server );

        void* _runReceiverThread();
        void    _handleConnect();
        void    _handleDisconnect();
        bool    _handleData();

        void* _runCommandThread();
        void    _redispatchCommands();

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
        CommandResult _cmdDisconnect( Command& command );
        CommandResult _cmdGetNodeData( Command& command );
        CommandResult _cmdGetNodeDataReply( Command& command );
        CommandResult _cmdAcquireSendToken( Command& command );
        CommandResult _cmdAcquireSendTokenReply( Command& command );
        CommandResult _cmdReleaseSendToken( Command& command );

        CHECK_THREAD_DECLARE( _thread );
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
