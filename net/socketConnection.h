
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
        SocketConnection();

        virtual bool connect(eqBase::RefPtr<ConnectionDescription> description);

        virtual bool listen( eqBase::RefPtr<ConnectionDescription> description);
        virtual Connection* accept();

        virtual void close();

        ushort getPort() const;

    protected:
        virtual ~SocketConnection();

    private:
        bool _createSocket();
        void _parseAddress( eqBase::RefPtr<ConnectionDescription> description, 
                            sockaddr_in& socketAddress );
    };
}

#endif //EQNET_SOCKET_CONNECTION_H
