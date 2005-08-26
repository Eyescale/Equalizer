
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include "pipeConnection.h"
#include "socketConnection.h"

#include <eq/base/log.h>

#include <alloca.h>
#include <errno.h>

using namespace eqNet;
using namespace std;

Connection::Connection()
        : _state( STATE_CLOSED )
{}

Connection::Connection(const Connection& conn)
        : _state( conn._state )
{}

Connection* Connection::create( const ConnectionType type )
{
    switch( type )
    {
        case TYPE_TCPIP:
            return new SocketConnection();

        case TYPE_PIPE:
            return new PipeConnection();

        default:
            WARN << "Connection type not implemented" << endl;
            return NULL;
    }
}

Connection* Connection::accept( const int timeout )
{
    if( _state != STATE_LISTENING )
        return NULL;

    // prepare pollfd 'set'
    pollfd pollFD;
    pollFD.fd      = getReadFD();
    pollFD.events  = POLLIN;
    pollFD.revents = 0;

    if( pollFD.fd == -1 )
    {
        // Could implement the same using a setjmp() + alarm().
        WARN << "Cannot accept on connection, it does not use a file descriptor"
             << endl;
        return NULL;
    }

    // poll for a connection
    const int ret = poll( &pollFD, 1, timeout );
    switch( ret )
    {
        case 0: // TIMEOUT
            return NULL;

        case -1: // ERROR
            WARN << "Error during poll(): " << strerror( errno ) << endl;
            return NULL;

        default: // SUCCESS
            return accept();
    }
}
