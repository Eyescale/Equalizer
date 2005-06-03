
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
         * Adds a new node to the session.
         *
         * This method adds additional nodes to the session, the local node and
         * server are automatically part of the session.
         * 
         * @param sessionID the session identifier.
         * @param connection the connection description for the new node.
         */
        static uint addNode( const uint sessionID, 
            const Node::Connection *connection );

        /**
         * @param sessionID the session identifier.
         * @returns the number of nodes in this Session. 
         */
        static uint nNodes( const uint sessionID );

        /**
         * Get the node id of the numbered node in the session.
         *
         * @param sessionID the session identifier.
         * @return the node id.
         */
        static uint getNodeID( const uint sessionID, const uint index );

        /**
         * Removes a node from the session.
         *
         * @param sessionID the session identifier.
         * @param nodeID the identifier of the node to remove
         * @return <code>true</code> if the node was removed, <code>false</code>
         *         if not.
         * @throws invalid_argument if the node identifier is not known.
         */
        static bool removeNode( const uint sessionID, const uint nodeID );
    };
};
