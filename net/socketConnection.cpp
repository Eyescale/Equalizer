
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "socketConnection.h"
#include "connectionDescription.h"

#include <eq/base/log.h>

#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;


SocketConnection::SocketConnection()
{
}

SocketConnection::~SocketConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool SocketConnection::connect( RefPtr<ConnectionDescription> description )
{
    ASSERT( description->type == TYPE_TCPIP );
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    if( !_createSocket( ))
        return false;

    // TODO: execute launch command

    sockaddr_in socketAddress;
    _parseAddress( description, socketAddress );

    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( !connected )
    {
        WARN << "Could not connect to '" << description->hostname << ":"
             << description->parameters.TCPIP.port << "': "
             << strerror( errno ) << endl;
        close();
        return false;
    }
    
    _description = description;
    _state = STATE_CONNECTED;
    return true;
}

bool SocketConnection::_createSocket()
{
    const int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( fd == -1 )
    {
        ERROR << "Could not create socket: " << strerror( errno ) << endl;
        return false;
    }

    const int on = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on) );

    _readFD  = fd;
    _writeFD = fd; // TCP/IP sockets are bidirectional
    return true;
}

void SocketConnection::close()
{
    if( _readFD == -1 )
        return;

    const bool closed = ( ::close(_readFD) == 0 );
    if( !closed )
        WARN << "Could not close socket: " << strerror( errno ) << endl;

    _readFD  = -1;
    _writeFD = -1;
    _state   = STATE_CLOSED;
}

void SocketConnection::_parseAddress( RefPtr<ConnectionDescription> description,
                                      sockaddr_in& socketAddress )
{
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddress.sin_port = htons( description->parameters.TCPIP.port );

    if( !description->hostname.empty( ))
    {
        hostent *hptr = gethostbyname( description->hostname.c_str() );
        if( hptr )
            memcpy(&socketAddress.sin_addr.s_addr, hptr->h_addr,hptr->h_length);
    }

    INFO << "Address " << ((socketAddress.sin_addr.s_addr >> 24 ) & 0xff)
         << "."  << ((socketAddress.sin_addr.s_addr >> 16 ) & 0xff)
         << "."  << ((socketAddress.sin_addr.s_addr >> 8 ) & 0xff)
         << "."  << (socketAddress.sin_addr.s_addr & 0xff)
         << ", port " << socketAddress.sin_port << endl;
}

//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool SocketConnection::listen( RefPtr<ConnectionDescription> description )
{
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    if( !_createSocket())
        return false;

    sockaddr_in socketAddress;
    const size_t size = sizeof( sockaddr_in ); 

    _parseAddress( description, socketAddress ); //TODO restrict IP

    const bool bound = (::bind(_readFD, (sockaddr *)&socketAddress, size) == 0);

    if( !bound )
    {
        WARN << "Could not bind socket: " << strerror( errno ) << " (" << errno 
             << ")" << endl;
        close();
        return false;
    }
    else if( socketAddress.sin_port == 0 )
        INFO << "Bound to port " << getPort() << endl;

    const bool listening = (::listen( _readFD, 10 ) == 0);
        
    if( !listening )
    {
        WARN << "Could not listen on socket: " << strerror( errno ) << endl;
        close();
        return false;
    }

    char hostname[256];
    gethostname( hostname, 256 );
    description->hostname              = hostname;
    description->parameters.TCPIP.port = getPort();

    _description = description;
    _state       = STATE_LISTENING;
    return listening;
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
RefPtr<Connection> SocketConnection::accept()
{
    if( _state != STATE_LISTENING )
        return NULL;

    sockaddr_in newAddress;
    socklen_t   newAddressLen = sizeof( newAddress );
    int         fd;

    do
        fd = ::accept( _readFD, (sockaddr*)&newAddress, &newAddressLen );
    while( fd == -1 && errno == EINTR );

    if( fd == -1 )
    {
        WARN << "accept failed: " << strerror( errno ) << endl;
        return NULL;
    }

    RefPtr<ConnectionDescription> description = new ConnectionDescription;

    description->type                  = TYPE_TCPIP;
    description->bandwidthKBS          = _description->bandwidthKBS;
    description->hostname              = inet_ntoa( newAddress.sin_addr );
    description->parameters.TCPIP.port = newAddress.sin_port;

    SocketConnection* newConnection = new SocketConnection;

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_description = description;
    newConnection->_state       = STATE_CONNECTED;

    INFO << "accepted connection from "
         << inet_ntoa(newAddress.sin_addr) << ":" << newAddress.sin_port <<endl;

    return newConnection;
}

ushort SocketConnection::getPort() const
{
    sockaddr_in address;
    socklen_t used = sizeof(address);
    getsockname( _readFD, (struct sockaddr *) &address, &used ); 
    return ntohs(address.sin_port);
}
