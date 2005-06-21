
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include "pipeConnection.h"
#include "socketConnection.h"

#include <eq/base/log.h>

#include <alloca.h>
#include <errno.h>

using namespace eqNet;
using namespace eqNet::priv;
using namespace std;

Connection::Connection()
        : _state( STATE_CLOSED )
{
}

Connection* Connection::create( const NetworkProtocol protocol )
{
    switch( protocol )
    {
        case PROTO_TCPIP:
            return new SocketConnection();

        case PROTO_PIPE:
            return new PipeConnection();

        default:
            WARN << "Protocol not implemented" << endl;
            return NULL;
    }
}

Connection* Connection::select( const std::vector<Connection*> &connections, 
    const int timeout, short &event )
{
    static const int events       = POLLIN; // | POLLPRI;
    const int        nConnections = connections.size();

    // prepare pollfd set
    pollfd*         pollFDs = (pollfd*)alloca( nConnections * sizeof( pollfd ));
    for( size_t i=0; i<nConnections; i++ )
    {
        pollFDs[i].fd = connections[i]->getReadFD();
        if( pollFDs[i].fd == -1 )
        {
            WARN << "Cannot select connection " << i
                 << ", connection does not use file descriptor" << endl;
            event = POLLERR;
            return NULL;
        }

        pollFDs[i].events  = events;
        pollFDs[i].revents = 0;
    }

    // poll for a result
    const int ret = poll( pollFDs, nConnections, timeout );
    switch( ret )
    {
        case 0: // TIMEOUT
            event = 0;
            return NULL;

        case -1: // ERROR
            event = POLLERR;
            WARN << "Error during poll(): " << strerror( errno ) << endl;
            return NULL;

        default: // SUCCESS
            for( size_t i=0; i<nConnections; i++ )
            {
                if( pollFDs[i].revents == 0 )
                    continue;

                event = pollFDs[i].revents;
                INFO << "selected connection #" << i << " of " << nConnections
                     << ", event " << event << endl;
                return connections[i];
            }
            WARN << "Error: Unreachable code" << endl;
            event = POLLERR;
            return NULL;
    }
}
