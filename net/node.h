
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

namespace eqNet
{
    /**
     * Manages a node.
     *
     * A node represents a separate entity within a session, typically a PC in a
     * cluster or a process on a shared-memory system.
     */
    class Node
    {
    public:
        /** The supported network protocols. */
        enum Protocol 
        {
            PROTO_TCPIP, //!< TCP/IP networking.
            PROTO_MPI    //!< MPI networking.
        };

        /**
         * Describes a connection for a node within a session.
         */
        struct Connection 
        {
            Protocol protocol; //!< The network protocol.
            /**
             * The network identifier, all nodes within the same protocol and
             * network are directly connected. 
             */
            uint networkID = 0;

            /** The bandwidth in kilobyte per second for this connection. */
            uint64 bandwidthKBS;
    
            /** 
             * The command to spawn a new process on the node, e.g., "ssh
             * eile@node1".
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

        // add, get, remove protocol per node
        static void addConnection( const uint nodeID, 
            const Connection *connection );

        
        static uint nConnections( const uint nodeID );

        static const Connection& getConnection( const uint index );
        
        //static bool removeConnection( const uint index );
        //static uint removeConnections( const Protocol protocol );

        /**
         * Enables forwarding between two protocols on this node. 
         */
        static void enableForwarding( const uint nid, const uint connIndex1, 
            const uint connIndex2 );
        void disableForwarding( const uint nid, const uint connIndex1, 
            const uint connIndex2 );

        bool   /*success*/ init();
        void               exit();

        bool   /*success*/ initNode( uint nid ); // late init may not be possible
        void   /*success*/ exitNode( uint nid ); // early exit may not be possible

        bool   /*success*/ startNode( uint nid );
        void               stopNode( uint nid );
    };
};


