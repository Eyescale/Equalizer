
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_DESCRIPTION_H
#define EQNET_CONNECTION_DESCRIPTION_H

#include <eq/net/network.h>

#include <strings.h>

namespace eqNet
{
    /**
     * Describes the connection of a Node to a Network.
     *
     * @sa Network, Node
     */
    struct ConnectionDescription
    {
        ConnectionDescription() : bandwidthKBS(0), launchCommand(NULL)
            {
                bzero( &parameters, sizeof(parameters));
            }

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
        os << "    ConnectionDescription " << (void*)description <<  ": "
           << "bw " << description->bandwidthKBS << "KB/s, launchCommand '"
           << ( description->launchCommand==NULL ? "none" : 
               description->launchCommand ) << "'" << std::endl;
        return os;
    }
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
