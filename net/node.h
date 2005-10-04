
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_H
#define EQNET_NODE_H

#include <eq/base/base.h>
#include <eq/base/refPtr.h>
#include <eq/base/requestHandler.h>
#include <eq/base/thread.h>

#include "base.h"
#include "commands.h"
#include "connection.h"
#include "connectionSet.h"
#include "idHash.h"
#include "message.h"

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
    class Node : public eqBase::Thread, public eqNet::Base
    {
    public:
        enum State {
            STATE_STOPPED,   // initial
            STATE_CONNECTED, // remote node, connected
            STATE_LISTENING  // local node, listening
        };

        /** 
         * Constructs a new Node.
         */
        Node();

        /** 
         * Returns the state of this node.
         * 
         * @return the state of this node.
         */
        State getState(){ return _state; }


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
        bool connect( Node* node, eqBase::RefPtr<Connection> connection );

        /** 
         * Disconnects a connected node.
         *
         * @param node the remote node.
         * @return <code>true</code> if the node was disconnected correctly,
         *         <code>false</code> otherwise.
         */
        bool disconnect( Node* node );
        //*}


        /**
         * @name Connectivity information. 
         */
        //*{
        /** 
         * Sets the local node for this thread.
         * 
         * The local node is the listening node to which newly opened nodes
         * will be connected. It is thread-specific and will be set by default
         * the first listening node of this thread.
         *
         * @param node the local node for this thread.
         * @sa addConnectionDescription, send
         */
        void setLocalNode( Node* node ){ Thread::setSpecific( node ); }

        /** 
         * Returns the local node for this thread.
         *
         * @return the local node for this thread.
         * @sa setLocalNode
         */
        static Node* getLocalNode() { return (Node*)Thread::getSpecific(); }

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
        void removeConnectionDescription( const uint32 index );

        /** 
         * Returns the number of stored connection descriptions. 
         * 
         * @return the number of stored connection descriptions. 
         */
        uint32 nConnectionDescriptions() const 
            { return _connectionDescriptions.size(); }

        /** 
         * Returns a connection description.
         * 
         * @param index the index of the connection description.
         * @return the connection description.
         */
        eqBase::RefPtr<ConnectionDescription> getConnectionDescription(
            const uint32 index )
            { return _connectionDescriptions[index]; }
        //*}


        /**
         * @name Messaging API
         *
         * The messaging API provides basic point-to-point communications
         * between nodes. (Broadcast communications are handled by special
         * nodes?)
         */
        //@{
        /**
         * Sends a message to this node.
         *
         * @param type the type of the message elements.
         * @param ptr the memory address of the message elements.
         * @param count the number of message elements.
         * @return the success status of the transaction.
         */
        bool send( const MessageType type, const void *ptr, const uint64 count);

        /** 
         * Sends a packet to this node.
         * 
         * @param packet the packet.
         * @return the success status of the transaction.
         */
        bool send( const Packet& packet )
            {
                if( _state==STATE_STOPPED )
                    _connect();
                if( _state!=STATE_CONNECTED && _state!=STATE_LISTENING )
                    return false;

                const uint64 sent = _connection->send( packet );
                return ( sent==packet.size );
            }

        /** 
         * Sends data to this node.
         * 
         * The data is not further encapsulated, and the receiving node should
         * be prepared to receive the data by sending a packet describing the
         * data beforehand.
         *
         * @param size the size of the data in bytes.
         * @return the success status of the transaction.
         */
        bool send( const void* data, const uint64 size )
            {
                if( !_checkConnection() )
                    return false;

                const uint64 sent = _connection->send( data, size );
                return ( sent==size );
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
        //                             const uint64 count );

        /** 
         * Receives data from this node.
         *
         * This method receives raw data from this node and should only be
         * called upon receiption of a packet describing the data to be
         * received. It is potentially blocking.
         * 
         * @param buffer the buffer to store the data.
         * @param size the size of the data in bytes.
         * @return the success status of the transaction.
         */
        bool recv( const void* buffer, const uint64 size )
            {
                ASSERT( _state == STATE_CONNECTED || _state == STATE_LISTENING);
                const uint64 received = _connection->recv( buffer, size );
                return ( received==size );
            }
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
        bool mapSession( Node* server, Session* session, 
                         const std::string& name );
        //*}
        
    protected:
        /** The current state of this node. */
        State _state;
        
        /** The connection to this node, for remote nodes. */
        eqBase::RefPtr<Connection> _connection; // later: array of connections

        /** The connection set of all connection from/to this node, for local
            nodes. */
        ConnectionSet _connectionSet;

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** Determines if the node should be launched automatically. */
        bool _autoLaunch;

        /** The list of descriptions on how this node is reachable. */
        std::vector< eqBase::RefPtr<ConnectionDescription> >
            _connectionDescriptions;

        /** 
         * Handles a custom packet which has been received by this node.
         * 
         * This method is called for all packets which have a datatype unknown
         * to the node.
         * 
         * @param node the node which send the packet.
         * @param packet the packet.
         */
        virtual void handlePacket( Node* node, const Packet* packet ){}

        /** 
         * Handles a packet with a custom command which has been received by
         * this node.
         * 
         * This method is called for all node packets which have a command
         * unknown to the node.
         * 
         * @param node the node which sent the packet.
         * @param packet the packet.
         */
        virtual void handleCommand( Node* node, const NodePacket* packet ){}

        /** 
         * Handles the connection of a new node by connecting it to this node.
         * 
         * @param connection the incoming connection for the new node.
         * @return the newly connected node, or <code>NULL</code> if the
         *         connection was refused.
         */
        virtual Node* handleConnect( eqBase::RefPtr<Connection> connection );

        /** 
         * Handles the disconnection of a new node by disconnecting it from this
         * node.
         * 
         * @param node the disconnected node.
         */
        virtual void handleDisconnect( Node* node );

    private:
        /** The unique session identifier counter. */
        uint _sessionID;

        /** The current sessions of this node. */
        IDHash<Session*> _sessions;

        Session* _findSession( const std::string& name ) const;

        bool _listenToSelf();

        /** 
         * Ensures the connectivity of this node.
         * 
         * If the node is not connected, the available connection descriptions
         * are used to connect the node. 
         *
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         */
        bool _checkConnection()
            {
                if( _state==STATE_CONNECTED || _state==STATE_LISTENING )
                    return true;
                if( _state==STATE_STOPPED )
                    return _connect();
                return false;
            }

        /** 
         * Connects this node according to the available connection
         * descriptions.
         *
         * @return <code>true</code> if this node is connected,
         *         <code>false</code> otherwise.
         */
        bool _connect();

        /** 
         * Launches the node using the parameters from the connection
         * description.
         * 
         * @param description the connection description.
         */
        void _launch( eqBase::RefPtr<ConnectionDescription> description );

        /** 
         * Composes the launch command by expanding the variables in the
         * description's launch command string.
         * 
         * @param description the connection description.
         * @return the expanded launch command.
         */
        std::string _createLaunchCommand( eqBase::RefPtr<ConnectionDescription> 
                                          description );

        /** The receiver thread entry function for this node. */
        virtual ssize_t run();
            
        void _handleConnect( ConnectionSet& connectionSet );
        void _handleDisconnect( ConnectionSet& connectionSet );
        void _handleRequest( Node* node );
        void _handlePacket( Node* node, const Packet* packet);

        /** The command handler function table. */
        void (eqNet::Node::*_cmdHandler[CMD_NODE_CUSTOM])( Node* node, const Packet* packet );

        void _cmdStop( Node* node, const Packet* packet );
        void _cmdMapSession( Node* node, const Packet* packet );
        void _cmdMapSessionReply( Node* node, const Packet* packet);
        void _cmdSession( Node* node, const Packet* packet );

        static uint64 _getMessageSize( const MessageType type, 
                                       const uint64 count );

        friend inline std::ostream& operator << ( std::ostream& os,
                                                  const Node* node );

    };

    inline std::ostream& operator << ( std::ostream& os, const Node* node )
    {
        if( node )
            os << "node " << (void*)node << ", " << node->_connection.get();
        else
            os << "NULL node";
        
        return os;
    }
};
#endif // EQNET_NODE_H
