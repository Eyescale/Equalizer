
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include <string.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/errno.h>
#include <sys/socket.h>

using namespace eqNet;
using namespace std;

Connection::Connection()
        : _fd( -1 ),
          _state( STATE_CLOSED )
{
}

Connection::~Connection()
{
    _deleteSocket();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool Connection::connect( const char *address )
{
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    _fd = _createSocket();

    sockaddr_in socketAddress;
    _parseAddress( socketAddress, address );

    const bool connected = (::connect( _fd, (struct sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( connected )
        _state = STATE_CONNECTED;
    else
    {
        WARN( "Could not connect socket: %s\n", strerror( errno ));
        _deleteSocket();
    }
    
    return connected;
}

int Connection::_createSocket()
{
    const int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( fd == -1 )
    {
        string error = strerror( errno );
        throw connection_error("Could not create socket: " + error);
    }

    const int on = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on) );

    return fd;
}

void Connection::_deleteSocket()
{
    if( _fd == -1 )
        return;

    const bool closed = ( close(_fd) == 0 );
    if( !closed )
        WARN( "Could not close socket: %s\n", strerror( errno ));

    _fd = -1;
    _state = STATE_CLOSED;
}

void Connection::_parseAddress( sockaddr_in& socketAddress,
    const char* address )
{
    uint32_t ip = INADDR_ANY;
    short port = DEFAULT_PORT;

    if( address != NULL )
    {
        char *portName = strdup( address );
        char *ipName = strsep( &portName, ":" );

        if( strlen( ipName ) > 0 )
            ip = atoi( ipName );

        if( portName != NULL )
        {
            port = (short)atoi( portName );
            if( port == 0 ) port = DEFAULT_PORT;
        }

        free( ipName );
    }

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( ip );
    socketAddress.sin_port = htons( port );
}

//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool Connection::listen( const char *address )
{
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    _fd = _createSocket();

    sockaddr_in socketAddress;
    _parseAddress( socketAddress, address );

    const bool bound = (::bind( _fd, (const struct sockaddr *)&socketAddress,
            sizeof(socketAddress) ) == 0);

    if( !bound )
    {
        WARN( "Could not bind socket: %s\n", strerror( errno ));
        _deleteSocket();
        return false;
    }

    const bool listening = (::listen( _fd, 10 ) == 0);
        
    if( listening )
        _state = STATE_LISTENING;
    else
    {
        WARN( "Could not listen on socket: %s\n", strerror( errno ));
        _deleteSocket();
    }

    return listening;
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
size_t Connection::read( const void* buffer, const size_t bytes )
{
    if( _state != STATE_CONNECTED )
        return 0;

    char* ptr = (char*)buffer;
    size_t bytesLeft = bytes;

    while( bytesLeft )
    {
        ssize_t bytesRead = ::read( _fd, ptr, bytesLeft );
        
        if( bytesRead == 0 ) // EOF
        {
            _deleteSocket();
            return bytes - bytesLeft;
        }

        if( bytesRead == -1 ) // error
        {
            if( errno == EINTR ) // if interrupted, try again
                bytesRead = 0;
            else
            {
                WARN( "Error during socket read: %s\n", strerror( errno ));
                return bytes - bytesLeft;
            }
        }
        
        bytesLeft -= bytesRead;
        ptr += bytesRead;
    }

    return bytes;
}

size_t Connection::write( const void* buffer, const size_t bytes )
{
}
