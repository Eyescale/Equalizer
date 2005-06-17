
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_SOCKET_CONNECTION_H
#define EQNET_SOCKET_CONNECTION_H

#include "fdConnection.h"

#include <netinet/in.h>

namespace eqNet
{
    /**
     * A TCP/IP-based socket connection.
     */
    class SocketConnection : public FDConnection
    {
    public:
        SocketConnection(){}
        virtual ~SocketConnection();

        virtual bool connect();

        virtual bool listen();
        virtual Connection* accept();

        virtual void close();

    protected:

    private:
        void _createSocket();
        void _parseAddress( sockaddr_in& socketAddress );
    };
};

#endif //EQNET_SOCKET_CONNECTION_H
