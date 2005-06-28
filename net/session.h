
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SESSION_H
#define EQNET_SESSION_H

#include <eq/base/base.h>
#include "base.h"
#include "global.h"
#include "network.h"

namespace eqNet
{
    namespace priv
    {
        class Session;
    }
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
     * connected to a session. A Node can be a participant in several
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
    class Session : public Base
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
         * @return the session, or <code>NULL</code> if the session could not be
         *         created.
         */
        static Session* create( const char* server );
        
        /**
         * Joins an existing session on a server.
         *
         * @param server the server location.
         * @param sessionID the session identifier.
         * @return <code>true</code> if the session could be joined,
         *         <code>false</code> if not.
         * @throws ??? if the server could not be contacted.
         */
        static bool join( const char* server, const uint sessionID );
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
         * @return the node identifier.
         * @sa Node, Network::addNode
         */
        uint newNode();

        /**
         * Returns the number of nodes in this session.
         *
         * @returns the number of nodes in this Session. 
         */
        uint nNodes();

        /**
         * Get the node id of the numbered node in this session.
         *
         * @param index the index of the node.
         * @return the node identifier.
         */
        uint getNodeID( const uint index );

        /**
         * Returns the node identifier of the local node.
         *
         * @return the node identifier of the local node.
         */
        uint getLocalNodeID();

        /**
         * Removes a node from this session.
         *
         * @param nodeID the identifier of the node to remove
         * @return <code>true</code> if the node was removed, <code>false</code>
         *         if not.
         * @throws invalid_argument if the node identifier is not known.
         */
        bool deleteNode( const uint nodeID );
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
         * @param protocol the network protocol.
         * @sa addNode
         */
        uint newNetwork( 
            const NetworkProtocol protocol );

        /**
         * Returns the number of networks in this session.
         *
         * @returns the number of networks in this Session. 
         */
        uint nNetworks();

        /**
         * Get the network id of the numbered network in this session.
         *
         * @param index the index of the network.
         * @return the network identifier.
         */
        uint getNetworkID( const uint index );

        /**
         * Deletes a network of this session.
         *
         * @param networkID the identifier of the network to remove
         * @return <code>true</code> if the network was removed,
         *         <code>false</code> if not.
         * @throws invalid_argument if the network identifier is not known.
         */
        bool deleteNetwork( const uint networkID );
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
         * @param nodeIDs the identifiers of the nodes belonging to the group.
         * @param nNodes the number of nodes in the group.
         * @return the group identifier of the added group.
         */
        uint addGroup( const uint nodeIDs[], 
            const uint nNodes );

        /**
         * Returns the number of groups in this session.
         *
         * @return the number of groups in this Session. 
         */
        uint nGroups();

        /**
         * Get the group id of the numbered group in this session.
         *
         * @param index the index of the group.
         * @return the group identifier.
         */
        uint getGroupID( const uint index );

        /**
         * Removes a group from this session.
         *
         * @param groupID the identifier of the group to remove
         * @return <code>true</code> if the group was removed,
         *         <code>false</code> if not.
         * @throws invalid_argument if the group identifier is not known.
         */
        bool removeGroup( const uint groupID );
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
         * @return <code>true</code> if all networks in this session
         *         were successfully initialised, <code>false</code>
         *         if not.
         * @sa Network::init, start
         */
        bool init();

        /**
         * Exits this session.
         *
         * Exiting this session de-initializes all networks in this session.
         *
         * @sa Network::exit, stop
         */
        void exit();

        /**
         * Start all nodes of all initialized networks in this session.
         *
         * @return <code>true</code> if all node in this session were
         *         successfully started , <code>false</code> if not.
         * @sa Network::start, init
         */
        bool start();

        /**
         * Stops all nodes of all initialized networks in this session.
         *
         * @sa Network::stop, exit
         */
        void stop();
        //*}

    protected:
        /** 
         * Constructs a new session.
         * 
         * @param id the identifier of the session.
         */
        Session(const uint id) : Base(id) {}
    };
}

#endif // EQNET_SESSION_H

