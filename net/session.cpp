
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "session.h"

#include "connection.h"
#include "networkPriv.h"
#include "nodePriv.h"
#include "server.h"

#include <eq/base/log.h>
#include <eq/net/connectionDescription.h>

#include <alloca.h>

using namespace eqNet;
using namespace std;

Session::Session()
        : _id( INVALID_ID )
{
}

uint Session::create( const char* server )
{
    Session *session = new Session();
    session->_create( server );
    return session->_getID();
}

void Session::_create( const char* serverAddress )
{
    Server* server = _openServer( serverAddress );
    
    if( server == NULL )
    {
        WARN << "Could not contact server" << endl;
        //_id = INVALID_ID;
        return;
    }

    
//     Node* server = new Node();
//     _nodes.push_back( server );

//     Node* node = new Node();
//     _nodes.push_back( node );

//     Network* network = new Network(Network::PROTO_TCPIP);
    
}

Server* Session::_openServer( const char* serverAddress )
{
    priv::Network*    network = priv::Network::create( INVALID_ID,
        PROTO_TCPIP );
    priv::Node*       server  = new priv::Node( INVALID_ID );
    priv::Node*       local   = new priv::Node( INVALID_ID-1 );
    ConnectionDescription serverConnection;
    ConnectionDescription localConnection;
    
    if( serverAddress )
        serverConnection.TCPIP.address = serverAddress;
    else
    {
        // If the server address is <code>NULL</code>, the environment
        // variable EQSERVER is used to determine the server address.
        const char* env = getenv( "EQSERVER" );
        if( env )
            serverConnection.TCPIP.address = env;
        else
        {
            // If the environment variable is not set, the local server on the
            // default port is contacted.
            char *address = (char *)alloca( 16 );
            sprintf( address, "localhost:%d", DEFAULT_PORT );
            serverConnection.TCPIP.address = address;
        }
    }

//     Network::addNode( network, server, serverConnection );
//     Network::addNode( network, local, localConnection );
    
//     if( !Network::init(network) || !Network::start(network) )
//     {
//         // TODO delete network;
//     }
        
//     Connection* connection = Connection::create( Network::PROTO_TCPIP );

//     if( !connection->connect(serverConnection))
//     {
//         // If the server can not be contacted, a new server is created, serving
//         // only this application.
//         delete connection;

//         connection = Connection::create( Network::PROTO_PIPE );
        
//         serverConnection.launchCommand = "Server::run";
//         if( !connection->connect(serverConnection))
//         {
//             delete connection;
//             return NULL;
//         }
//     }

    return NULL;
}
        
/*
 * Joins an existing session on a server.
 *
 * @param server the server location.
 * @param sessionID the session identifier.
 * @return <code>true</code> if the session could be joined,
 *         <code>false</code> if not.
 */
bool Session::join( const char *server, const uint sessionID )
{
    return false;
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
    return INVALID_ID;
}

/*
 * Returns the number of nodes in this session.
 *
 * @param sessionID the session identifier.
 * @returns the number of nodes in this Session. 
 */
uint Session::nNodes( const uint sessionID )
{
    return 0;
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
    return 0;
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
    return false;
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
void Session::addNetwork( const uint sessionID, const uint networkID )
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
    return 0;
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
    return 0;
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
    return false;
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
    return 0;
}

/*
 * Returns the number of groups in this session.
 *
 * @param sessionID the session identifier.
 * @return the number of groups in this Session. 
 */
uint Session::nGroups( const uint sessionID )
{
    return 0;
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
    return 0;
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
    return false;
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
    return false;
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
    return false;
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
