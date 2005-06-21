
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NETWORK_H
#define EQNET_NETWORK_H

#include <eq/base/base.h>
#include <eq/net/message.h>

namespace eqNet
{
    struct ConnectionDescription;

    /** The supported network protocols. */
    enum NetworkProtocol 
    {
        PROTO_TCPIP,  //!< TCP/IP networking.
        PROTO_MPI,    //!< MPI networking.
        PROTO_PIPE    //!< anonymous pipe to a forked process
    };


    /**
     * Manages a network within a session.
     *
     * A network represents a collection of nodes within a Session,
     * which are directly connected using the same network
     * protocol. All nodes within a network are initialized and exited
     * collectively, but can be started and stopped individually. Some
     * protocols allow adding or removing nodes to an initialized or
     * running network, while other protocols do not allow this
     * operation.
     *
     * @sa Session, Node
     */
    class Network
    {
    public:

        /** @name Managing nodes */
        //@{
        /**
         * Adds a node to this network.
         *
         * @param networkID the network identifier.
         * @param nodeID the node identifier.
         * @param connection the connection parameters.
         * @sa Node::enableForwarding(), Node::disableForwarding()
         */
        static void addNode( const uint networkID, const uint nodeID, 
            const ConnectionDescription& connection );

        /**
         * Returns the number of nodes for this network.
         *
         * @param networkID the network identifier.
         * @return the number of nodes.
         */
        static uint nNodes( const uint networkID );

        /**
         * Returns the node identifier for a numbered node.
         *
         * @param networkID the network identifier.
         * @param index the index of the node.
         * @return the node identifier.
         */
        static uint getNode( const uint networkID, const uint index );
        
        //static bool removeNode( const uint index );

        /**
         * Returns the connection information for a node.
         *
         * @param networkID the network identifier.
         * @param nodeID the node identifier.
         * @return the connection information.
         */
        static const ConnectionDescription& getConnection( 
            const uint networkID, const uint nodeID );

        
        /** 
         * Set the gateway node to another network.
         * 
         * @param networkID the network identifier.
         * @param toNetworkID the identifier of the foreign network.
         * @param nodeID the identifier of the gateway node.
         */
        static void setGateway( const uint networkID, const uint toNetworkID,
            const uint nodeID );
        //@}

        /**
         * @name State Management
         */
        //@{
        /**
         * Initialise this network.
         *
         * Initialising this network prepares the network to be
         * started. Some protocols may contact the nodes to start a
         * process.
         *
         * The nodes in this network will not run, that is, start to
         * execute the entry function, until they have been started. 
         *
         * @param networkID the network identifier.
         * @return <code>true</code> if the network was successfully
         *         initialized, <code>false</code> if not.
         * @sa Node::init(), start()
         */
        static bool init(const uint networkID);

        /**
         * Exits this network.
         *
         * @param networkID the network identifier.
         * @sa Node::exit(), stop()
         */
        static void exit(const uint networkID);

        /**
         * Start all nodes in this initialized network.
         *
         * @param networkID the network identifier.
         * @return <code>true</code> if all initialized nodes in this
         *         network were successfully started, <code>false</code>
         *         if not.
         * @sa startNode(), init()
         */
        static bool start(const uint networkID);

        /**
         * Starts a node in this initialized network.
         *
         * @param networkID the network identifier.
         * @param nodeID the node identifier.
         * @return <code>true</code> if the node was successfully
         *         started, <code>false</code> if not.
         * @sa start(), init()
         */
        static bool startNode(const uint networkID, const uint nodeID);

        /**
         * Stops all running nodes in this initialized network.
         *
         * @param networkID the network identifier.
         * @sa stopNode(), exit()
         */
        static void stop(const uint networkID);

        /**
         * Stops a running node.
         *
         * @param networkID the network identifier.
         * @param nodeID the node identifier.
         * @sa stop(), exit()
         */
        static void stopNode(const uint networkID, const uint nodeID);
        //@}

        /**
         * @name Messaging API
         *
         * The messaging API provides basic point-to-point communications
         * between nodes. Broadcast communications are handled by the Group
         * class.
         */
        //@{

        /**
         * Sends a message to a node using this network.
         *
         * @param toNodeID the identifier of the node to send the
         *                 message to, it has to be running in this network.
         * @param type the type of the message elements.
         * @param ptr the memory address of the message elements.
         * @param count the number of message elements.
         * @param flags the transmission flags.
         */
        static void send( const uint toNodeID, const Message::Type type,
            const void *ptr, const uint64 count, const uint flags );

        /** 
         * Receives a message from a node using this network.
         * 
         * @param fromNodeID the identifier of the node sending the message, or
         *                   <code>NODE_ANY</code>.
         * @param type the type of the message to receive, or
         *                   <code>TYPE_ANY</code>.
         * @param ptr the memory address where the received message will be
         *                   stored, or <code>NULL</code> if the memory should
         *                   be allocated automatically.
         * @param count the address where to store the number of received
         *                   elements.
         * @param timeout the timeout in milliseconds before a receive is
         *                   cancelled.
         * @return the address where the received message was stored, or
         *                   <code>NULL</code> if the message was not received.
         */
        static void* recv( const uint fromNodeID, const Message::Type type, 
            const void *ptr, const uint64 *count, const float timeout );
        //@}
    };
}

#endif // EQNET_NETWORK_H
