
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include "connectionDescription.h"
#include "pipeConnection.h"
#include "socketConnection.h"
#include "uniPipeConnection.h"

#include <eq/base/log.h>

#include <alloca.h>
#include <errno.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

Connection::Connection()
        : _state( STATE_CLOSED )
{}

Connection::Connection(const Connection& conn)
        : _state( conn._state ),
          _description( conn._description )
{}

Connection::~Connection()
{}

RefPtr<Connection> Connection::create( const Type type )
{
    switch( type )
    {
        case TYPE_TCPIP:
            return new SocketConnection();

        case TYPE_PIPE:
            return new PipeConnection();

        case TYPE_UNI_PIPE:
            return new UniPipeConnection();

        default:
            EQWARN << "Connection type not implemented" << endl;
            return NULL;
    }
}

RefPtr<Connection> Connection::accept( const int timeout )
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
        EQWARN << "Cannot accept on connection, it does not use a file descriptor"
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
            EQWARN << "Error during poll(): " << strerror( errno ) << endl;
            return NULL;

        default: // SUCCESS
            return accept();
    }
}

uint64_t Connection::send( Packet& packet, const void* data, 
                           const uint64_t dataSize ) const
{
    if( dataSize <= 8 ) // fits in existing packet
    {
        memcpy( (char*)(&packet) + packet.size-8, data, dataSize );
        return send( packet );
    }

    uint64_t       size      = packet.size-8 + dataSize;
    size += (4 - size%4);
    char*          buffer    = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size-8 );
    memcpy( buffer + packet.size-8, data, dataSize );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}

eqBase::RefPtr<ConnectionDescription> Connection::getDescription()
{
    return _description;
}

