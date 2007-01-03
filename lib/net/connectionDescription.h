
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_DESCRIPTION_H
#define EQNET_CONNECTION_DESCRIPTION_H

#include "connection.h"

#include <eq/base/base.h>
#include <eq/base/referenced.h>
#include <sys/param.h>

namespace eqNet
{
    /**
     * Describes the connection to a Node.
     *
     * @sa Node
     */
    class ConnectionDescription : public eqBase::Referenced
    {
    public:
        ConnectionDescription() 
                : type( Connection::TYPE_TCPIP ),
                  bandwidthKBS( 0 ),
                  launchTimeout( 10000 )
            {
                TCPIP.port = 0;
            }

        /** The network protocol for the connection. */
        Connection::Type type;

        ///** The bandwidth in kilobyte per second for this connection. */
        uint64_t bandwidthKBS;

        /** 
         * The command to spawn a new process on the node, e.g., 
         * "ssh eile@node1", can be <code>NULL</code>.
         * 
         * %h - hostname
         * %c - command
         * %n - unique node identifier
         */
        std::string launchCommand; 

        /** 
         * The amount of time in milliseconds to wait before a node is
         * considered unreachable during start.
         */
        int32_t launchTimeout;

        /** The host name. */
        std::string hostname;

        /** The individual parameters for the connection. */
        union
        {
            /** TCP/IP parameters */
            struct
            {
                /** The listening port. */
                ushort port;

            } TCPIP;
            
            /** MPI parameters */
            struct
            {
            } MPI;

            /** pipe parameters */
            struct
            {
                /** Read-only parameter to identify an open pipe connection. */
                int fd;
            } Pipe;
        };

        /** @return this description as a string. */
        std::string toString();

        /** 
         * Reads the connection description from a string.
         * 
         * @param data the string containing the connection description.
         * @return <code>true</code> if the information was read correctly, 
         *         <code>false</code> if not.
         */
        bool fromString( const std::string& data );

    protected:
        virtual ~ConnectionDescription() {}
    };
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
