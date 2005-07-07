
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKET_CONNECTION_H
#define EQNET_SOCKET_CONNECTION_H

#include "fdConnection.h"

#include <netinet/in.h>

namespace eqNet
{
    namespace priv
    {
        /**
         * A TCP/IP-based socket connection.
         */
        class SocketConnection : public FDConnection
        {
        public:
            SocketConnection();

            virtual ~SocketConnection();

            virtual bool connect( const eqNet::ConnectionDescription& desc );

            virtual bool listen( eqNet::ConnectionDescription &description );
            virtual Connection* accept();

            virtual void close();

            ushort getPort() const;

        protected:

        private:
            void _createSocket();
            void _parseAddress( const ConnectionDescription &description, 
                sockaddr_in& socketAddress );
        };
    }
}

#endif //EQNET_SOCKET_CONNECTION_H
