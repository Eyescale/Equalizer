
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_DESCRIPTION_H
#define EQNET_CONNECTION_DESCRIPTION_H

#include <eq/base/base.h>
#include <strings.h>
#include <sys/param.h>

namespace eqNet
{
    /** The supported network protocols. */
    enum ConnectionType
    {
        TYPE_TCPIP,   //!< TCP/IP networking.
        TYPE_MPI,     //!< MPI networking.
        TYPE_PIPE,    //!< pipe() based bi-directional connection
        TYPE_UNI_PIPE //!< pipe() based uni-directional connection
    };

    /**
     * Describes the connection to a Node.
     *
     * @sa Node
     */
    struct ConnectionDescription
    {
        ConnectionDescription() 
                : type( TYPE_TCPIP ),
                  bandwidthKBS( 0 ),
                  launchCommand( NULL )
            {
                bzero( &parameters, sizeof(parameters));
            }

        /** The network protocol for the connection. */
        ConnectionType type;

        /** The bandwidth in kilobyte per second for this connection. */
        uint64 bandwidthKBS;
    
        /** 
         * The command to spawn a new process on the node, e.g., 
         * "ssh eile@node1", can be <code>NULL</code>.
         * 
         * %h - hostname
         * %c - command
         */
        const char *launchCommand; 

        /** The host name. */
        char hostname[MAXHOSTNAMELEN+1];

        /** The individual parameters for the connection. */
        union
        {
            /** TCP/IP parameters */
            struct
            {
                /** The listening port. */
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
           << "type " << ( description->type==TYPE_TCPIP ? "TCP/IP" :
                           description->type==TYPE_MPI   ? "MPI" :
                           description->type==TYPE_PIPE  ? "PIPE" : "UNKNOWN" )
           << " bw " << description->bandwidthKBS << "KB/s, launchCommand '"
           << ( description->launchCommand==NULL ? "none" : 
               description->launchCommand ) << "'";
        return os;
    }
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
