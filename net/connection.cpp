
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include "socketConnection.h"

using namespace eqNet;
using namespace std;

Connection::Connection()
        : _state( STATE_CLOSED )
{
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
Connection* Connection::create( ConnectionDescription &description )
{
    Connection* connection;

    switch( description.protocol )
    {
        case Network::PROTO_TCPIP:
            connection = new SocketConnection();
            break;

        default:
            WARN( "Protocol not implemented\n" );
            return NULL;
    }

    connection->_description = description;
    return connection;
}
