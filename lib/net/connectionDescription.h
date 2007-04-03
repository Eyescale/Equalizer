
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTIONDESCRIPTION_H
#define EQNET_CONNECTIONDESCRIPTION_H

#include <eq/net/connectionType.h>

#include <eq/base/base.h>
#include <eq/base/referenced.h>

namespace eqNet
{
    /**
     * Describes the connection to a Node.
     *
     * @sa Node
     */
    class EQ_EXPORT ConnectionDescription : public eqBase::Referenced
    {
    public:
        ConnectionDescription() 
                : type( CONNECTIONTYPE_TCPIP ),
                  bandwidthKBS( 0 ),
                  launchTimeout( 10000 )
            {
                TCPIP.port = 0;
            }

        /** The network protocol for the connection. */
        ConnectionType type;

        ///** The bandwidth in kilobyte per second for this connection. */
        uint64_t bandwidthKBS;

        /** 
         * The amount of time in milliseconds to wait before a node is
         * considered unreachable during start.
         */
        int32_t launchTimeout;

        /** The individual parameters for the connection. */
        union
        {
            /** TCP/IP parameters */
            struct
            {
                /** The listening port. */
                uint16_t port;

            } TCPIP, SDP;

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

        /** @name Data Access */
        //*{
        void setHostname( const std::string& hostname );
        const std::string& getHostname() const;
        void setLaunchCommand( const std::string& launchCommand );
        const std::string& getLaunchCommand() const;
        //*}
    protected:
        virtual ~ConnectionDescription() {}

    private:
        /** 
         * The command to spawn a new process on the node, e.g., 
         * "ssh eile@node1", can be <code>NULL</code>.
         * 
         * %h - hostname
         * %c - command
         * %n - unique node identifier
         */
        std::string _launchCommand; 

        /** The host name. */
        std::string _hostname;


    };
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
