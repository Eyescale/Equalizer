
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_H
#define EQNET_NODE_H

#include <eq/net/base.h>
#include <eq/net/commands.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/idHash.h>
#include <eq/net/message.h>
#include <eq/net/nodeID.h>
#include <eq/net/requestCache.h>

#include <eq/base/base.h>
#include <eq/base/perThread.h>
#include <eq/base/requestHandler.h>
#include <eq/base/thread.h>

#include <list>

namespace eqNet
{
    class ConnectionDescription;
    class Packet;
    class Session;

    /**
     * Manages a node.
     *
     * A node represents a separate entity in the peer-to-peer network,
     * typically a process on a cluster node or on a shared-memory system. It
     * has at least one Connection through which is reachable. A Node provides
     * the basic communication facilities through message passing.
     */
    class Node : public Base, public eqBase::Referenced
    {
    public:
        enum State 
        {
            STATE_STOPPED,   // initial               
            STATE_LAUNCHED,  // remote node, launched
            STATE_CONNECTED, // remote node, connected  
            STATE_LISTENING  // local node, listening
        };

        enum CreateReason 
        {
            REASON_CLIENT_CONNECT,
            REASON_INCOMING_CONNECT
        };

        /** 
         * Constructs a new Node.
         *
         * @param nCommands the highest command ID to be handled by the node, at
         *                  least <code>CMD_NODE_CUSTOM</code>.
         */
        Node( const uint32_t nCommands = CMD_NODE_CUSTOM );

        /** @name Data Access. */
        //*{
        /** 
         * Returns the state of this node.
         * 
         * @return the state of this node.
         */
        State getState()    const { return _state; }
        bool  isConnected() const { return (_state == STATE_CONNECTED); }

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
         * Initializes this node.
         *
         * The node will spawn a thread locally and listen on the connection for
         * incoming requests. The node will be in the listening state if the
         * method completed successfully. A listening node can connect to other
         * nodes.
         * 
         * @param connection a listening connection for incoming request, can be
         *                   <CODE>NULL</CODE>.
         * @return <code>true</code> if the node could be initialized,
         *         <code>false</code> if not.
         * @sa connect
         */
        bool listen( eqBase::RefPtr<Connection> connection = NULL );

        /** 
         * Stops this node.
         * 
         * If this node is listening, the node will stop listening and terminate
         * its receiver thread.
         * 
         * @return <code>true</code> if the node was stopped, <code>false</code>
         *         if it was not stopped.
         */
        bool stopListening();

        /** 
         * Connects a node to this listening node.
         *
         * @param node the remote node.
         * @param connection the connection to the remote node.
         * @return <code>true</code> if the node was connected correctly,
         *         <code>false</code> otherwise.
         */
        bool connect( eqBase::RefPtr<Node> node, 
                      eqBase::RefPtr<Connection> connection );

        /** 
         * Get a node by identifier.
         * 
         * @param id the node identifier.
         * @return the node.
         */
        eqBase::RefPtr<Node> getNode( const NodeID& id ){ return _nodes[id]; }

        /** 
         * Connect and potentially launch this node using to the available
         * connection descriptions.
         *
         * On success, the node is in the connected state, otherwise it's state
         * is unchanged.
         *
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         * @sa initConnect, syncConnect
         */
        bool connect();

        /** 
         * Start connecting and potentially launching this node using to the
         * available connection descriptions.
         *
         * On success, the node is in the launched or connected state, otherwise
         * it's state is unchanged.
         *
         * @return <code>true</code> if this node is connecting,
         *         <code>false</code> otherwise.
         * @sa syncConnect
         */
        bool initConnect();

        /** 
         * Synchronize the connection initiated by initConnect().
         *
         * On success, the node is in the connected state, otherwise it's state
         * is unchanged.
         *
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         * @sa initConnect
         */
        bool syncConnect();

        /** 
         * Create and connect a node given be an identifier.
         * 
         * @param nodeID the identifier of the node to connect.
         * @param server a node holding connection information to the
         *               destination node.
         * @return the connected node, or an invalid RefPtr if the node could
         *         not be connected.
         */
        eqBase::RefPtr<Node> connect( NodeID& nodeID,
                                      eqBase::RefPtr<Node> server);

        /** 
         * Disconnect this node.
         */
        bool disconnect();

        /** 
         * Disconnects a connected node.
         *
         * @param node the remote node.
         * @return <code>true</code> if the node was disconnected correctly,
         *         <code>false</code> otherwise.
         */
        bool disconnect( Node* node );

        /** 
         * Ensures the connectivity of this node.
         * 
         * If the node is not connected, the available connection descriptions
         * are used to connect the node. 
         *
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         */
        bool checkConnection()
            {
                if( _state == STATE_CONNECTED || _state == STATE_LISTENING )
                    return true;
                if( _state == STATE_STOPPED )
                    return connect();
                return false;
            }
        //*}


        /**
         * @name Connectivity information. 
         */
        //*{
        /** 
         * Sets the local node for this execution thread.
         * 
         * The local node is the listening node to which newly opened nodes
         * will be connected. It is thread-specific and will be set by default
         * the first listening node of this thread.
         *
         * @param node the local node for this thread.
         * @sa addConnectionDescription, send
         */
        static void setLocalNode( eqBase::RefPtr<Node> node );

        /** 
         * Returns the local node for this thread.
         *
         * @return the local node for this thread.
         * @sa setLocalNode
         */
        static eqBase::RefPtr<Node> getLocalNode() { return _localNode.get(); }

        /** 
         * Returns if the node is local.
         * 
         * @return <code>true</code> if the node is local, <code>false</false>
         *         otherwise.
         */
        bool isLocal() const { return (_state==STATE_LISTENING); }

        /** 
         * Adds a new description how this node can be reached.
         * 
         * @param cd the connection description.
         */
        void addConnectionDescription(eqBase::RefPtr<ConnectionDescription> cd)
            { _connectionDescriptions.push_back( cd ); }
        
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
        uint32_t nConnectionDescriptions() const 
            { return _connectionDescriptions.size(); }

        /** 
         * Returns a connection description.
         * 
         * @param index the index of the connection description.
         * @return the connection description.
         */
        eqBase::RefPtr<ConnectionDescription> getConnectionDescription(
            const uint32_t index ) const
            { return _connectionDescriptions[index]; }

        /** 
         * Returns the connection to this node.
         * 
         * @return the connection to this node. 
         */
        eqBase::RefPtr<Connection> getConnection() const { return _connection; }
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
                if( !checkConnection() )
                    return false;
                return ( _connection->send( packet ) == packet.size );
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
                if( !checkConnection() )
                    return false;
                return ( _connection->send( packet, string ) >= packet.size );
            }

        /** 
         * Sends a packet with a string to the node.
         * 
         * The data is send as a new packet containing the original packet and
         * the string, so that it is received as one packet by the node.
         *
         * It is assumed that the last item in the packet is of sizeof(T) and
         * usable for the data. The vector's elements are copied using memcpy to
         * the output stream.
         *
         * @param packet the packet.
         * @param data the vector containing the data.
         * @return the success status of the transaction.
         */
        template< class T >
        bool send( Packet& packet, const std::vector<T>& data )
            {
                if( !checkConnection() )
                    return false;
                return ( _connection->send( packet, data ) >= packet.size );
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
                if( !checkConnection() )
                    return false;
                return ( _connection->send( packet, data, size )>=packet.size );
            }

        /** 
         * Notifies that a message is ready to be received.
         *
         * The Node will allocate memory for the message if ptr is
         * <CODE>NULL</CODE> or if more the count elements have been
         * received. It is the applications responsibity to free the memory,
         * even if it was allocated by this method.
         *
         * @param fromNode the node sending the message, or
         *                 <code>NODE_ID_ANY</code>.
         * @param type the type of the message to receive, or
         *             <code>TYPE_ID_ANY</code>.
         * @param ptr the memory address where the received message should be
         *            stored, or <code>NULL</code> if the memory should be
         *            allocated automatically.
         * @param count the maximum number of elements storeable in ptr and the
         *              return value to store the number of received elements.
         * @return the address where the received message was stored, or
         *         <code>NULL</code> if the message was not received.
         */
        //virtual void* notifyReceive( Node* fromNode, const MessageType type,
        //                             const uint64_t count );
        //@}

        /**
         * @name Receiver-thread node connect
         */
        //@{
        /** 
         * Requests the connection of a node.
         * 
         * @param nodeID the identifier of the node to connect.
         * @param server another node holding connection information to the
         *               destination node.
         * @return an identifier for the request for subsequent commands.
         */
        uint32_t startConnectNode( NodeID& nodeID, eqBase::RefPtr<Node> server);

        enum ConnectState
        {
            CONNECT_PENDING,
            CONNECT_SUCCESS,
            CONNECT_FAILURE
        };

        /** 
         * Polls the state of a connection request.
         * 
         * If the return value is CONNECT_SUCCESS or CONNECT_FAILURE, the
         * requestID becomes invalid.
         *
         * @param requestID the request identifier.
         * @return the current state of the connection request.
         */
        ConnectState pollConnectNode( const uint32_t requestID );
        //@}

        /**
         * @name Session management
         */
        //*{
        /**
         * Maps a local session object to a named session.
         *
         * @param server the node serving the session.
         * @param session the session.
         * @param name the name of the session.
         * @return <code>true</code> if the session was mapped,
         *         <code>false</code> if not.
         */
        bool mapSession( eqBase::RefPtr<Node> server, Session* session, 
                         const std::string& name );

        /**
         * Maps a local session object to an existing session.
         *
         * @param server the node serving the session.
         * @param session the session.
         * @param id the identifier of the session.
         * @return <code>true</code> if the session was mapped,
         *         <code>false</code> if not.
         */
        bool mapSession( eqBase::RefPtr<Node> server, Session* session, 
                         const uint32_t id );

        /** 
         * Unmaps a mapped session.
         * 
         * @param session the session.
         * @return <code>true</code> if the session was unmapped,
         *         <code>false</code> if there was an error.
         */
        bool unmapSession( Session* session );

        /** @return the mapped session with the given identifier, or NULL. */
        Session* getSession( const uint32_t id ) { return _sessions[id]; }

        /** 
         * Adds an already mapped session to this node.
         * 
         * @param session the session.
         * @param server the node serving the session.
         * @param sessionID the identifier of the session.
         * @param name the name of the session.
         */
        void addSession( Session* session, eqBase::RefPtr<Node> server, 
                         const uint32_t sessionID, const std::string& name );

        /** 
         * Removes an unmapped session from this node.
         * 
         * @param session the session.
         */
        void removeSession( Session* session );
        //*}
        /** 
         * Runs this node as a client to a server.
         * 
         * @param clientArgs the client arguments as specified by the server.
         * @return the success value of the run.
         */
        bool runClient( const std::string& clientArgs );

        /** 
         * @return <code>true</code> if executed from the receiver thread,
         *         <code>false</code> if not.
         */
        bool inReceiverThread() const { return _receiverThread->isCurrent(); }

        const NodeID& getNodeID(){ return _id; }

    protected:
        /** Destructs this node. */
        virtual ~Node();

        /** Determines if the node should be launched automatically. */
        bool _autoLaunch;

        /** true if this node was launched automatically. */
        bool _autoLaunched;

        /** 
         * Dispatches a packet to the appropriate object or to handlePacket.
         * 
         * @param node the node which send the packet.
         * @param packet the packet.
         * @return the result of the operation.
         * @sa handlePacket, Base::handleCommand
         */
        CommandResult dispatchPacket( Node* node, const Packet* packet );

        /** 
         * The main loop for auto-launched clients. 
         *
         * @sa runClient()
         */
        virtual void clientLoop() {}

        /** 
         * Handles a packet which has been received by this node for a custom
         * data type.
         * 
         * @param node the node which send the packet.
         * @param packet the packet.
         * @return the result of the operation.
         * @sa dispatchPacket
         */
        virtual CommandResult handlePacket( Node* node, const Packet* packet )
            { return COMMAND_ERROR; }

        /** 
         * Handles the connection of a new node by connecting it to this node.
         * 
         * @param connection the incoming connection for the new node.
         */
        virtual void handleConnect( eqBase::RefPtr<Connection> connection );

        /** 
         * Handles the disconnection of a new node by disconnecting it from this
         * node.
         * 
         * @param node the disconnected node.
         */
        virtual void handleDisconnect( Node* node );

        /** 
         * Push a command to be handled by another entity, typically a thread.
         * 
         * @param node the node which send the packet.
         * @param packet the packet.
         * @return <code>true</code> if the command was pushed,
         *         <code>false</code> if not.
         */
        virtual bool pushCommand( Node* node, const Packet* packet )
            { return false; }

        /** 
         * Push a command to be handled immediately by another entity, typically
         * a thread. 
         * 
         * @param node the node which send the packet.
         * @param packet the packet.
         * @return <code>true</code> if the command was pushed,
         *         <code>false</code> if not.
         */
        virtual bool pushCommandFront( Node* node, const Packet* packet )
            { return false; }

        /** 
         * Factory method to create a new node.
         * 
         * @param the reason for the node creation.
         * @return the node.
         */
        virtual eqBase::RefPtr<Node> createNode( const CreateReason reason )
            { return new Node(); }

        CommandResult _cmdStop( Node* node, const Packet* packet );

    private:
        /** per-thread local node */
        static eqBase::PerThread<Node*> _localNode;

        /** Globally unique node identifier. */
        NodeID _id;

        /** The current state of this node. */
        State _state;

        /** The connected nodes. */
        NodeIDHash< eqBase::RefPtr<Node> > _nodes;

        /** The current mapped sessions of this node. */
        IDHash<Session*> _sessions;

        /** The connection to this node, for remote nodes. */
        eqBase::RefPtr<Connection> _connection;

        /** The listening connection. */
        eqBase::RefPtr<Connection> _listener;

        /** The connection set of all connections from/to this node. */
        ConnectionSet _connectionSet;

        /** The node for each connection. */
        eqBase::PtrHash< Connection*, eqBase::RefPtr<Node> > _connectionNodes;

        /** The request id for the async launch operation. */
        uint32_t _launchID;

        /** Packets re-scheduled for dispatch. */
        std::list<Request*> _pendingRequests;
        RequestCache        _requestCache;

        /** The list of descriptions on how this node is reachable. */
        std::vector< eqBase::RefPtr<ConnectionDescription> >
            _connectionDescriptions;

        /** The name of the program to autolaunch. */
        std::string _programName;
        /** The directory of the program to autolaunch. */
        std::string _workDir;

        /** true as long as the client loop is active. */
        bool _clientRunning;

        /** The pending connection requests from startConnectNode(). */
        IDHash<NodeID> _connectionRequests;
        /** The identifier for the next connection request. */
        uint32_t       _nextConnectionRequestID;

#ifdef CHECK_THREADSAFETY
        mutable pthread_t _threadID;
#endif

        bool _listenToSelf();
        void _cleanup();

        /** 
         * Launches the node using the parameters from the connection
         * description.
         * 
         * @param description the connection description.
         * @return <code>true</code> if the node was launched,
         *         <code>false</code> otherwise.
         */
        bool _launch( eqBase::RefPtr<ConnectionDescription> description );

        /** 
         * Composes the launch command by expanding the variables in the
         * description's launch command string.
         * 
         * @param description the connection description.
         * @param requestID the request identifier to be used by the launched
         *                  node when connecting to this node.
         * @return the expanded launch command.
         */
        std::string _createLaunchCommand( eqBase::RefPtr<ConnectionDescription> 
                                          description );
        std::string   _createRemoteCommand();


        /** 
         * Performs the initial connection handshake and returns the peer's
         * connect packet.
         *
         * The returned packet has to be free'd by the caller.
         * 
         * @param connection the connection.
         * @return the peer's connect packet, or <code>NULL</code> if the
         *         connection handshake failed.
         */
        NodeConnectPacket* _performConnect( eqBase::RefPtr<Connection>
                                            connection );
        NodeConnectPacket*   _readConnectReply( eqBase::RefPtr<Connection> 
                                                connection );

        /** 
         * Find a connected node using a connection description
         * 
         * @param connectionDescription the connection description for the node.
         * @return the node, or <code>NULL</code> if no node was found.
         */
        eqBase::RefPtr<Node> _findConnectedNode( const char* 
                                                 connectionDescription );

        /** 
         * Find a named, mapped session.
         * 
         * @param name the session name.
         * @return the session, or <code>NULL</code> if the session is not
         *         mapped on this node.
         */
        Session* _findSession( const std::string& name ) const;

        /** Generates a new, unique session identifier. */
        uint32_t Node::_generateSessionID();

        /** The receiver thread. */
        class ReceiverThread : public eqBase::Thread
        {
        public:
            ReceiverThread( Node* node ) 
                    : eqBase::Thread( Thread::PTHREAD ),
                      _node( node )
                {}
            
            virtual ssize_t run(){ return _node->_runReceiver(); }

        private:
            Node* _node;
        };
        ReceiverThread* _receiverThread;

        ssize_t _runReceiver();
        void      _handleConnect();
        void      _handleDisconnect();
        void        _addConnectedNode( eqBase::RefPtr<Node> node, 
                                       eqBase::RefPtr<Connection> connection );
        bool      _handleRequest( Node* node );
        void      _redispatchPackets();

        /** The command functions. */
        CommandResult _cmdMapSession( Node* node, const Packet* packet );
        CommandResult _cmdMapSessionReply( Node* node, const Packet* packet );
        CommandResult _cmdUnmapSession( Node* node, const Packet* packet );
        CommandResult _cmdUnmapSessionReply( Node* node, const Packet* packet );
        CommandResult _cmdGetConnectionDescription( Node* node, 
                                                    const Packet* packet );
        CommandResult _cmdGetConnectionDescriptionReply( Node* node, 
                                                         const Packet* packet );

        static uint64_t _getMessageSize( const MessageType type, 
                                         const uint64_t count );

        friend std::ostream& operator << ( std::ostream& os, const Node* node );
    };

    inline std::ostream& operator << ( std::ostream& os, const Node* node )
    {
        if( node )
            os << "node " << node->_id;
        else
            os << "NULL node";
        
        return os;
    }
};
#endif // EQNET_NODE_H
