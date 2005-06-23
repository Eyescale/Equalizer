
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/base/base.h>
#include <eq/net/global.h>
#include <eq/net/network.h>

namespace eqNet
{
    class Connection;
    class Node;
    class Server;

    /**
     * Manages a session.
     *
     * A session represents a group of nodes managed by a central server. The
     * server ensures that all identifiers used during communication are
     * unique. 
     *
     * Nodes are added to a Network in order to describe how they are
     * connected to a session. Nodes can be participants in several
     * networks.
     *
     * Server addresses are specified in the form
     * '<code>(&lt;IP&gt;|&lt;name&gt;)(:&lt;port&gt;)</code>'. If the
     * server address is <code>NULL</code>, the environment variable
     * <code>EQSERVER</code> is used to determine the server address. If this
     * variable is not set, the local server on the default port is
     * contacted. If the server can not be contacted, a new server is
     * created, serving only this application.
     */
    class Session
    {
    public:
        /**
         * @name Getting a Session
         */
        //*{
        
        /**
         * Create a new session by connecting to an Equalizer server.
         *
         * @param server the server location.
         * @return the session identifier.
         */
        static uint create( const char *server );
        
        /**
         * Joins an existing session on a server.
         *
         * @param server the server location.
         * @param sessionID the session identifier.
         * @return <code>true</code> if the session could be joined,
         *         <code>false</code> if not.
         * @throws ??? if the server could not be contacted.
         */
        static bool join( const char *server, const uint sessionID );
        //*}

        /**
         * @name Managing nodes
         */
        //*{
        /**
         * Adds a new node to this session.
         *
         * This method adds an additional node to this session, the local node
         * and server are automatically part of this session. Nodes have
         * to be added to at least one Network in order to communicate
         * with the session.
         * 
         * @param sessionID the session identifier.
         * @return the node identifier.
         * @sa Node, Network::addNode
         */
        static uint addNode( const uint sessionID );

        /**
         * Returns the number of nodes in this session.
         *
         * @param sessionID the session identifier.
         * @returns the number of nodes in this Session. 
         */
        static uint nNodes( const uint sessionID );

        /**
         * Get the node id of the numbered node in this session.
         *
         * @param sessionID the session identifier.
         * @param index the index of the node.
         * @return the node identifier.
         */
        static uint getNodeID( const uint sessionID, const uint index );

        /**
         * Returns the node identifier of the local node.
         *
         * @return the node identifier of the local node.
         */
        static uint getLocalNodeID();

        /**
         * Removes a node from this session.
         *
         * @param sessionID the session identifier.
         * @param nodeID the identifier of the node to remove
         * @return <code>true</code> if the node was removed, <code>false</code>
         *         if not.
         * @throws invalid_argument if the node identifier is not known.
         */
        static bool removeNode( const uint sessionID, const uint nodeID );
        //*}

        /**
         * @name Managing Networks
         * 
         * Networks are used to create connectivity between nodes.
         * @sa Network, Node
         */
        //*{
        /**
         * Adds a new network to this session.
         *
         * The network between the server and the local node is
         * automatically added to the session.
         * 
         * @param sessionID the session identifier.
         * @param protocol the network protocol.
         * @sa addNode
         */
        static uint addNetwork( const uint sessionID, 
            const NetworkProtocol protocol );

        /**
         * Returns the number of networks in this session.
         *
         * @param sessionID the session identifier.
         * @returns the number of networks in this Session. 
         */
        static uint nNetworks( const uint sessionID );

        /**
         * Get the network id of the numbered network in this session.
         *
         * @param sessionID the session identifier.
         * @param index the index of the network.
         * @return the network identifier.
         */
        static uint getNetworkID( const uint sessionID, const uint index );

        /**
         * Removes a network from this session.
         *
         * @param sessionID the session identifier.
         * @param networkID the identifier of the network to remove
         * @return <code>true</code> if the network was removed,
         *         <code>false</code> if not.
         * @throws invalid_argument if the network identifier is not known.
         */
        static bool removeNetwork( const uint sessionID, const uint networkID );
        //*}

        /**
         * @name Managing groups
         * 
         * A Group is a collection of nodes used for broadcast
         * communications. Broadcast primitives are typically not efficient
         * across Network boundaries. 
         * @sa Group, Node
         */
        //*{
        /**
         * Adds a new group to this session.
         *
         * @param sessionID the session identifier.
         * @param nodeIDs the identifiers of the nodes belonging to the group.
         * @param nNodes the number of nodes in the group.
         * @return the group identifier of the added group.
         */
        static uint addGroup( const uint sessionID, const uint nodeIDs[], 
            const uint nNodes );

        /**
         * Returns the number of groups in this session.
         *
         * @param sessionID the session identifier.
         * @return the number of groups in this Session. 
         */
        static uint nGroups( const uint sessionID );

        /**
         * Get the group id of the numbered group in this session.
         *
         * @param sessionID the session identifier.
         * @param index the index of the group.
         * @return the group identifier.
         */
        static uint getGroupID( const uint sessionID, const uint index );

        /**
         * Removes a group from this session.
         *
         * @param sessionID the session identifier.
         * @param groupID the identifier of the group to remove
         * @return <code>true</code> if the group was removed,
         *         <code>false</code> if not.
         * @throws invalid_argument if the group identifier is not known.
         */
        static bool removeGroup( const uint sessionID, const uint groupID );
        //*}

        /**
         * @name Session State Management
         */
        //*{
        /**
         * Initialise this session.
         *
         * Initialising this session initialises all networks in this
         * session. Afterwards, the nodes have to be started before
         * they can communicate with other nodes in this session.
         *
         * @param sessionID the session identifier.
         * @return <code>true</code> if all networks in this session
         *         were successfully initialised, <code>false</code>
         *         if not.
         * @sa Network::init, start
         */
        static bool init(const uint sessionID);

        /**
         * Exits this session.
         *
         * Exiting this session de-initializes all networks in this session.
         *
         * @param sessionID the session identifier.
         * @sa Network::exit, stop
         */
        static void exit(const uint sessionID);

        /**
         * Start all nodes of all initialized networks in this session.
         *
         * @param sessionID the session identifier.
         * @return <code>true</code> if all node in this session were
         *         successfully started , <code>false</code> if not.
         * @sa Network::start, init
         */
        static bool start(const uint sessionID);

        /**
         * Stops all nodes of all initialized networks in this session.
         *
         * @param sessionID the session identifier.
         * @sa Network::stop, exit
         */
        static void stop(const uint sessionID);
        //*}
    };
}

#endif // EQNET_SESSION_H

