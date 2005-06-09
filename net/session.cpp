
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"

using namespace eqNet;

/*
 * Create a new session by connecting to an Equalizer server.
 *
 * @param server the server location.
 * @return the session identifier.
 * @throws ??? if the server could not be contacted.
 */
uint Session::init( const char *server )
{
}
        
/*
 * Joins an existing session on a server.
 *
 * @param server the server location.
 * @param sessionID the session identifier.
 * @return <code>true</code> if the session could be joined,
 *         <code>false</code> if not.
 * @throws ??? if the server could not be contacted.
 */
bool Session::join( const char *server, const uint sessionID )
{
}
//*}

/*
 * @name Managing nodes
 */
//*{
/*
 * Adds a new node to this session.
 *
 * This method adds an additional node to this session, the local node
 * and server are automatically part of this session. Nodes have
 * to be added to at least one Network in order to communicate
 * with the session.
 * 
 * @param sessionID the session identifier.
 * @return the node identifier of the added node.
 * @sa Network::addNode
 */
uint Session::addNode( const uint sessionID )
{
}

/*
 * Returns the number of nodes in this session.
 *
 * @param sessionID the session identifier.
 * @returns the number of nodes in this Session. 
 */
uint Session::nNodes( const uint sessionID )
{
}

/*
 * Get the node id of the numbered node in this session.
 *
 * @param sessionID the session identifier.
 * @param index the index of the node.
 * @return the node identifier.
 */
uint Session::getNodeID( const uint sessionID, const uint index )
{
}

/*
 * Removes a node from this session.
 *
 * @param sessionID the session identifier.
 * @param nodeID the identifier of the node to remove
 * @return <code>true</code> if the node was removed, <code>false</code>
 *         if not.
 * @throws invalid_argument if the node identifier is not known.
 */
bool Session::removeNode( const uint sessionID, const uint nodeID )
{
}
//*}

/*
 * @name Managing Networks
 * 
 * Networks are used to create connectivity between nodes.
 * @sa Network, Node
 */
//*{
/*
 * Adds a new network to this session.
 * 
 * @param sessionID the session identifier.
 * @param protocol the network protocol.
 * @return the network identifier of the added network.
 * @sa addNode
 */
uint Session::addNetwork( const uint sessionID, 
    const Network::Protocol protocol )
{
}

/*
 * Returns the number of networks in this session.
 *
 * @param sessionID the session identifier.
 * @returns the number of networks in this Session. 
 */
uint Session::nNetworks( const uint sessionID )
{
}

/*
 * Get the network id of the numbered network in this session.
 *
 * @param sessionID the session identifier.
 * @param index the index of the network.
 * @return the network identifier.
 */
uint Session::getNetworkID( const uint sessionID, const uint index )
{
}

/*
 * Removes a network from this session.
 *
 * @param sessionID the session identifier.
 * @param networkID the identifier of the network to remove
 * @return <code>true</code> if the network was removed,
 *         <code>false</code> if not.
 * @throws invalid_argument if the network identifier is not known.
 */
bool Session::removeNetwork( const uint sessionID, const uint networkID )
{
}
//*}

/*
 * @name Managing groups
 * 
 * A Group is a collection of nodes used for broadcast communications.
 * @sa Group, Node
 */
//*{
/*
 * Adds a new group to this session.
 *
 * @param sessionID the session identifier.
 * @param nodeIDs the identifiers of the nodes belonging to the group.
 * @param nNodes the number of nodes in the group.
 * @return the group identifier of the added group.
 */
uint Session::addGroup( const uint sessionID, const uint nodeIDs[], 
    const uint nNodes )
{
}

/*
 * Returns the number of groups in this session.
 *
 * @param sessionID the session identifier.
 * @return the number of groups in this Session. 
 */
uint Session::nGroups( const uint sessionID )
{
}

/*
 * Get the group id of the numbered group in this session.
 *
 * @param sessionID the session identifier.
 * @param index the index of the group.
 * @return the group identifier.
 */
uint Session::getGroupID( const uint sessionID, const uint index )
{
}

/*
 * Removes a group from this session.
 *
 * @param sessionID the session identifier.
 * @param groupID the identifier of the group to remove
 * @return <code>true</code> if the group was removed,
 *         <code>false</code> if not.
 * @throws invalid_argument if the group identifier is not known.
 */
bool Session::removeGroup( const uint sessionID, const uint groupID )
{
}
//*}

/*
 * @name Session State Management
 */
//*{
/*
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
bool Session::init(const uint sessionID)
{
}

/*
 * Exits this session.
 *
 * Exiting this session de-initializes all networks in this session.
 *
 * @param sessionID the session identifier.
 * @sa Network::exit, stop
 */
void Session::exit(const uint sessionID)
{
}

/*
 * Start all nodes of all initialized networks in this session.
 *
 * @param sessionID the session identifier.
 * @return <code>true</code> if all node in this session were
 *         successfully started , <code>false</code> if not.
 * @sa Network::start, init
 */
bool Session::start(const uint sessionID)
{
}

/*
 * Stops all nodes of all initialized networks in this session.
 *
 * @param sessionID the session identifier.
 * @sa Network::stop, exit
 */
void Session::stop(const uint sessionID)
{
}
//*}
