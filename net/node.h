
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODE_H
#define EQNET_NODE_H

#include <eq/base/base.h>
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
    class Session;
    class Packet;

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
         * Initializes this node.
         *
         * The node will spawn a thread locally and listen on the connection for
         * incoming requests. The node will be in the listening state if the
         * method completed successfully. A listening node can connect to other
         * nodes.
         * 
         * @param connection the connection to listen to.
         * @return <code>true</code> if the node could be initialized,
         *         <code>false</code> if not.
         * @sa connect
         */
        bool listen( Connection* connection ){ return _listen(connection,true);}

        /** 
         * Connects a node to this listening node.
         *
         * @param node the remote node.
         * @param connection the connection to the remote node.
         * @return The connected remote node, or <code>NULL</code> if the node
         *         could not be connected.
         */
        bool connect( Node* node, Connection* connection );

        /** 
         * Returns the state of this node.
         * 
         * @return the state of this node.
         */
        State getState(){ return _state; }

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
                if( _state == STATE_CONNECTED ) // remote
                {
                    const uint64 sent = _connection->send( packet );
                    return ( sent==packet.size );
                }
                // local, TODO: send to receiver thread to avoid deadlocks
                _handlePacket( this, &packet );
                return true;
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
                ASSERT( _state == STATE_CONNECTED ); // TODO: local send
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
                ASSERT( _state == STATE_CONNECTED ); // TODO: local receive
                const uint64 received = _connection->recv( buffer, size );
                return ( received==size );
            }
        //@}

        /**
         * @name Session management
         */
        //*{
        /**
         * Maps a local session object to a named session on this node.
         *
         * @param session the session.
         * @param name the name of the session.
         * @return <code>true</code> if the session was mapped,
         *         <code>false</code> if not.
         */
        bool mapSession( Session* session, const char* name );
        //*}
        
    protected:
        /** The current state of this node. */
        State _state;
        
        /** The connection to this node, for remote nodes. */
        Connection* _connection; // later: array of connections

        /** The connection set of all connection from/to this node, for local
            nodes. */
        ConnectionSet _connectionSet;

        /** Registers requests waiting for a return value. */
        eqBase::RequestHandler _requestHandler;


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
         * Handles the connection of a new node by connection it to this node.
         * 
         * @param connection the incoming connection for the new node.
         * @return the newly connected node, or <code>NULL</code> if the
         *         connection was refused.
         */
        virtual Node* handleNewNode( Connection* connection );

    private:
        /** The unique session identifier counter. */
        uint _sessionID;

        /** The current sessions of this node. */
        IDHash<Session*> _sessions;
        
        /** 
         * Initializes this node.
         * 
         * @param connection the connection to listen to.
         * @param threaded <code>true</code> if the listening should happen
         *                 in a separate thread (non-blocking) or from within
         *                 this method (blocking).
         * @return <code>true</code> if the node could be initialized,
         *         <code>false</code> if not.
         * @sa connect
         */
        bool _listen( Connection* connection, const bool threaded );

        void _packSession( Node* node, const Session* session );

        /** The receiver thread entry function for this node. */
        virtual ssize_t run();
            
        void _handleConnect( ConnectionSet& connectionSet );
        void _handleRequest( Connection* connection, Node* node );
        void _handlePacket( Node* node, const Packet* packet);

        /** The command handler function table. */
        void (eqNet::Node::*_cmdHandler[CMD_NODE_CUSTOM])( Node* node, const Packet* packet );

        void _cmdCreateSession( Node* node, const Packet* packet );
        void _cmdCreateSessionReply( Node* node, const Packet* packet);
        void _cmdNewSession( Node* node, const Packet* packet );

        static uint64 _getMessageSize( const MessageType type, 
                                       const uint64 count );

        friend void ::eqNet_Node_runServer( eqNet::Connection* connection );
        friend inline std::ostream& operator << ( std::ostream& os,
                                                  const Node* node );

    };

    inline std::ostream& operator << ( std::ostream& os, const Node* node )
    {
        if( node )
            os << "node " << (void*)node << ", " << node->_connection;
        else
            os << "NULL node";
        
        return os;
    }
};
#endif // EQNET_NODE_H
