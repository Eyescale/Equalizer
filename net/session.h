
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */


/** 
 * @namespace eqNet
 * @brief The equalizer networking abstraction layer.
 */
namespace eqNet
{
    /**
     * Manages a session.
     *
     * A session represents a group of nodes managed by a central server. The
     * server ensures that all identifiers used during communication are
     * unique. 
     *
     * Nodes are added to a Group in order to describe how they are
     * connected to a session. Nodes can be participants in several
     * groups.
     *
     * Server addresses are specified in the form
     * '<code>(&lt;IP&gt;|&lt;name&gt;)(:&lt;port&gt;)</code>'. If the
     * server address is <code>NULL</code>, the environment variable
     * <code>EQSERVER</code> is used to determine the server address. If this
     * variable is not set, the local server on the default port is
     * contacted. If no server is running on the local machine, a new server is
     * created, serving only this application.
     */
    class Session
    {
    public:
        /**
         * Initializes the network by connecting to an Equalizer server.
         *
         * @param server the server location.
         * @return the session id.
         * @throws ??? if the server could not be contacted.
         */
        static uint init( const char *server );
        
        /**
         * Joins an existing session on a server.
         *
         * @param server the server location.
         * @param sessionID the session id.
         * @return <code>true</code> if the session could be joined,
         *         <code>false</code> if not.
         * @throws ??? if the server could not be contacted.
         */
        static bool join( const char *server, const uint sessionID );

        /**
         * Adds a new group to this session.
         * 
         * Groups 
         * @param sessionID the session identifier.
         * @param protocol the groups network protocol.
         * @sa addNode
         */
        static uint addGroup( const uint sessionID, 
            const Group::Protocol protocol );

        /**
         * Returns the number of groups in this session.
         *
         * @param sessionID the session identifier.
         * @returns the number of groups in this Session. 
         */
        static uint nGroups( const uint sessionID );

        /**
         * Get the group id of the numbered group in this session.
         *
         * @param sessionID the session identifier.
         * @param index the index of the group.
         * @return the group id.
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

        /**
         * Adds a new node to this session.
         *
         * This method adds additional nodes to this session, the local node and
         * server are automatically part of this session. Nodes have
         * to be added to at least one Group in order to communicate
         * with the session.
         * 
         * @param sessionID the session identifier.
         * @sa Group::addNode
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
         * @return the node id.
         */
        static uint getNodeID( const uint sessionID, const uint index );

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

        /**
         * Initialise this session.
         *
         * Initialising this session initialises all groups in this
         * session. Afterwards, the nodes have to be started before
         * they can communicate with other nodes in this session.
         *
         * @param sessionID the session identifier.
         * @return <code>true</code> if all groups in this session
         *         were successfully initialised, <code>false</code>
         *         if not.
         * @sa Group::init, start
         */
        static bool init(const uint sessionID);

        /**
         * Exits this session.
         *
         * Exiting this session de-initializes all groups in this session.
         *
         * @param sessionID the session identifier.
         * @sa Group::exit, stop
         */
        static void exit(const uint sessionID);

        /**
         * Start all nodes of all initialized groups in this session.
         *
         * @param sessionID the session identifier.
         * @return <code>true</code> if all node in this session were
         *         successfully started , <code>false</code> if not.
         * @sa Group::start, init
         */
        static bool start(const uint sessionID);

        /**
         * Stops all nodes of all initialized groups in this session.
         *
         * @param sessionID the session identifier.
         * @sa Group::stop, exit
         */
        static void stop(const uint sessionID);
    };
};
