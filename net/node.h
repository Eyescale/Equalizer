
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

namespace eqNet
{
    /**
     * Manages a node.
     *
     * A node represents a separate entity within a session, typically
     * a PC in a cluster or a process on a shared-memory system. A
     * node is part of one session and 0-n groups. In order to
     * communicate with other nodes, it needs to run in at least one
     * Group.
     */
    class Node
    {
    public:
        /**
         * @name Administrative API
         */
        //@{
        /**
         * Enables forwarding between two groups on this node.
         *
         * @param nodeID the node identifier.
         * @param group1ID the first groups identifier.
         * @param group2ID the second groups identifier.
         */
        static void enableForwarding( const uint nodeID, const uint group1ID, 
            const uint group2ID );

        /**
         * Disables forwarding between two groups on this node.
         *
         * @param nodeID the node identifier.
         * @param group1ID the first groups identifier.
         * @param group2ID the second groups identifier.
         */
        static void disableForwarding( const uint nodeID, const uint group1ID, 
            const uint group2ID );
        //@}

        /**
         * @name Messaging API
         */
        //@{
        //@}
    };
};


