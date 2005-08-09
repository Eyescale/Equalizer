
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_DESCRIPTION_H
#define EQNET_CONNECTION_DESCRIPTION_H

#include "network.h"

#include <strings.h>
#include <sys/param.h>

namespace eqNet
{
    /**
     * Describes the connection of a Node to a Network.
     *
     * @sa Network, Node
     */
    struct ConnectionDescription
    {
        ConnectionDescription() 
                : bandwidthKBS(0),
                  launchCommand(NULL)
            {
                bzero( &parameters, sizeof(parameters));
            }

        /** The bandwidth in kilobyte per second for this connection. */
        uint64 bandwidthKBS;
    
        /** 
         * The command to spawn a new process on the node, e.g., 
         * "ssh eile@node1", can be <code>NULL</code>.
         * 
         * %a - TCP/IP address
         * %c - command
         */
        const char *launchCommand; 

        /** The individual parameters for the connection. */
        union
        {
            /** TCP/IP parameters */
            struct
            {
                /** The host name. */
                char hostname[MAXHOSTNAMELEN+1];

                /** The port. */
                ushort port;

                /** 
                 * The amount of time in milliseconds to wait before a node is
                 * considered unreachable during start.
                 */
                int launchTimeout;
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
        } parameters;
    };

    /** 
     * Prints the connection description to a std::ostream.
     * 
     * @param os the output stream.
     * @param description the connection description.
     * @return the output stream.
     */
    inline std::ostream& operator << ( std::ostream& os, 
        ConnectionDescription* description)
    {
        os << "connection description " << (void*)description <<  ": "
           << "bw " << description->bandwidthKBS << "KB/s, launchCommand '"
           << ( description->launchCommand==NULL ? "none" : 
               description->launchCommand ) << "'";
        return os;
    }
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
