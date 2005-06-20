
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_DESCRIPTION_H
#define EQNET_CONNECTION_DESCRIPTION_H

#include <eq/net/network.h>

namespace eqNet
{
    /**
     * Describes the connection of a Node to a Network.
     *
     * @sa Network, Node
     */
    struct ConnectionDescription
    {
        ConnectionDescription() : protocol(Network::PROTO_TCPIP), 
                                  bandwidthKBS(0), launchCommand(NULL)
            {
                TCPIP.address = NULL;
            }

        /** The network protocol for this connection. */
        Network::Protocol protocol;
        
        /** The bandwidth in kilobyte per second for this connection. */
        uint64 bandwidthKBS;
    
        /** 
         * The command to spawn a new process on the node, e.g., "ssh
         * eile@node1", can be <code>NULL</code>.
         */
        const char *launchCommand; 

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

            /** pipe parameters */
            struct
            {
                /** The name of the entry function for the forked process. */
                const char *entryFunc;
            } PIPE;
        };
    };
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
