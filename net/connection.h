
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_CONNECTION_H
#define EQNET_CONNECTION_H

#include "eq/base/base.h"

#include <stdexcept>
#include <netinet/in.h>

namespace eqNet
{
    enum State {
        STATE_CLOSED,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_LISTENING
    };

    #define DEFAULT_PORT 4242

    /**
     * The connection error exception, thrown by the Connection methods.
     */
    struct connection_error : public std::runtime_error
    {
        connection_error(const std::string& msg) : std::runtime_error(msg){}
    };

    /**
     * A base class to provide communication to other hosts.
     */
    class Connection
    {
    public:
        Connection();
        virtual ~Connection();

        bool connect( const char* address );
        bool listen( const char* address );

        size_t read( const void* buffer, const size_t bytes );
        size_t write( const void* buffer, const size_t bytes );

    private:
        int   _fd;     //!< The socket file descriptor.
        State _state;  //!< The connections state

        int  _createSocket();
        void _deleteSocket();
        void _parseAddress( sockaddr_in& socketAddress, 
            const char* address );
    };
};

#endif //EQNET_CONNECTION_H
