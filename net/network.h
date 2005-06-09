
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NETWORK_H
#define EQNET_NETWORK_H

namespace eqNet
{
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
        /** The supported network protocols. */
        enum Protocol 
        {
            PROTO_TCPIP, //!< TCP/IP networking.
            PROTO_MPI    //!< MPI networking.
        };

        /**
         * Describes a connection of a node to a network.
         */
        struct Connection 
        {
            /** The bandwidth in kilobyte per second for this connection. */
            uint64 bandwidthKBS;
    
            /** 
             * The command to spawn a new process on the node, e.g., "ssh
             * eile@network1".
             */
            const char *rshCommand; 

            /** The individual parameters for the connection. */
            union
            {
                /** TCP/IP parameters */
                struct
                {
                    /** 
                     * The address of the node in the form
                     * '<code>(&lt;IP&gt;|&lt;name&gt;)(:&lt;port&gt;)</code>'.
                     */
                    const char *address;
                } TCPIP;

                /** MPI parameters */
                struct
                {
                } MPI;
            };
        };

        /**
         * Adds a node to this network.
         *
         * @param networkID the network identifier.
         * @param nodeID the node identifier.
         * @param connection the connection parameters.
         * @sa Node::enableForwarding(), Node::disableForwarding()
         */
        static void addNode( const uint networkID, const uint nodeID, 
            const Connection *connection );

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
        static const Connection& getConnection( const uint networkID, 
            const uint nodeID );
        
        /**
         * Initialise this network.
         *
         * Initialising this network prepares the network to be
         * started. Some protocols may contact the nodes to start a
         * process.
         *
         * This nodes in this network will not run, that is, start to
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

    };
};

#endif // EQNET_NETWORK_H
