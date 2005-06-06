
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

namespace eqNet
{
    /**
     * Manages a network group.
     *
     * A group represents a collection of nodes within a Session,
     * which are directly connected using the same protocol. All nodes
     * within a group are initialized and exited collectively, but can
     * be started and stopped individually. Some protocols allow
     * adding or removing nodes to an initialized or running group
     * while other protocols do not allow this operation.
     *
     * @sa Session, Node
     */
    class Group
    {
    public:
        /** The supported network protocols. */
        enum Protocol 
        {
            PROTO_TCPIP, //!< TCP/IP networking.
            PROTO_MPI    //!< MPI networking.
        };

        /**
         * Describes a connection of a node to a group.
         */
        struct Connection 
        {
            /** The bandwidth in kilobyte per second for this connection. */
            uint64 bandwidthKBS;
    
            /** 
             * The command to spawn a new process on the node, e.g., "ssh
             * eile@group1".
             */
            const char *rshCommand; 

            union //!< The individual parameters for the connection.
            {
                struct TCPIP //!< TCP/IP parameters.
                {
                    /** 
                     * The address of the node in the form
                     * '<code>(&lt;IP&gt;|&lt;name&gt;)(:&lt;port&gt;)</code>'.
                     */
                    const char *address;
                };
                struct MPI //!< MPI parameters
                {
                };
            };
        };

        /**
         * Adds a node to this group.
         *
         * @param groupID the group identifier.
         * @param nodeID the node identifier.
         * @param connection the connection parameters.
         * @sa Node::enableForwarding(), Node::disableForwarding()
         */
        static void addNode( const uint groupID, const uint nodeID, 
            const Connection *connection );

        /**
         * Returns the number of nodes for this group.
         *
         * @param groupID the group identifier.
         * @return the number of nodes.
         */
        static uint nNodes( const uint groupID );

        /**
         * Returns the node identifier for a numbered node.
         *
         * @param groupID the group identifier.
         * @param index the index of the node.
         * @return the node identifier.
         */
        static uint getNode( const uint groupID, const uint index );
        
        //static bool removeNode( const uint index );

        /**
         * Returns the connection information for a node.
         *
         * @param groupID the group identifier.
         * @param nodeID the node identifier.
         * @return the connection information.
         */
        static const Connection& getConnection( const uint groupID, 
            const uint nodeID );
        
        /**
         * Initialise this group.
         *
         * Initialising this group prepares the group to be
         * started. Some protocols may contact the nodes to start a
         * process.
         *
         * This nodes in this group will not run, that is, start to
         * execute the entry function, until they have been started. 
         *
         * @param groupID the group identifier.
         * @return <code>true</code> if the group was successfully
         *         initialized, <code>false</code> if not.
         * @sa Node::init(), start()
         */
        static bool init(const uint groupID);

        /**
         * Exits this group.
         *
         * @param groupID the group identifier.
         * @sa Node::exit(), stop()
         */
        static void exit(const uint groupID);

        /**
         * Start all nodes in this initialized group.
         *
         * @param groupID the group identifier.
         * @return <code>true</code> if all initialized nodes in this
         *         group were successfully started, <code>false</code>
         *         if not.
         * @sa startNode(), init()
         */
        static bool start(const uint groupID);

        /**
         * Starts a node in this initialized group.
         *
         * @param groupID the group identifier.
         * @param nodeID the node identifier.
         * @return <code>true</code> if the node was successfully
         *         started, <code>false</code> if not.
         * @sa start(), init()
         */
        static bool startNode(const uint groupID, const uint nodeID);

        /**
         * Stops all running nodes in this initialized group.
         *
         * @param groupID the group identifier.
         * @sa stopNode(), exit()
         */
        static void stop(const uint groupID);

        /**
         * Stops a running node.
         *
         * @param groupID the group identifier.
         * @param nodeID the node identifier.
         * @sa stop(), exit()
         */
        static void stopNode(const uint groupID, const uint nodeID);

    };
};


