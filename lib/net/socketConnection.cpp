
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "socketConnection.h"
#include "connectionDescription.h"

#include <eq/base/log.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;


SocketConnection::SocketConnection()
{
    _description = new ConnectionDescription;
    _description->type = Connection::TYPE_TCPIP;
}

SocketConnection::~SocketConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool SocketConnection::connect()
{
    EQASSERT( _description->type == Connection::TYPE_TCPIP );
    if( _state != STATE_CLOSED )
        return false;

    if( _description->TCPIP.port == 0 )
        return false;

    _state = STATE_CONNECTING;

    if( !_createSocket( ))
        return false;

    // TODO: execute launch command

    sockaddr_in socketAddress;
    _parseAddress( socketAddress );

    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->hostname << ":"
             << _description->TCPIP.port << "': " << strerror( errno ) << endl;
        close();
        return false;
    }
    
    _state = STATE_CONNECTED;
    return true;
}

bool SocketConnection::_createSocket()
{
    const int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( fd == -1 )
    {
        EQERROR << "Could not create socket: " << strerror( errno ) << endl;
        return false;
    }

    _tuneSocket( fd );

    _readFD  = fd;
    _writeFD = fd; // TCP/IP sockets are bidirectional
    return true;
}

void SocketConnection::_tuneSocket( const int fd )
{
    const int on         = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof( on ));
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ));
#if 0
    const int bufferSize = 256*1024;
    setsockopt( fd, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof( bufferSize ));
    setsockopt( fd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof( bufferSize ));
#endif
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    _state   = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    const bool closed = ( ::close(_readFD) == 0 );
    if( !closed )
        EQWARN << "Could not close socket: " << strerror( errno ) << endl;

    _readFD  = -1;
    _writeFD = -1;
}

void SocketConnection::_parseAddress( sockaddr_in& socketAddress )
{
    socketAddress.sin_family      = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddress.sin_port        = htons( _description->TCPIP.port );
    memset( &(socketAddress.sin_zero), 0, 8 ); // zero the rest

    if( !_description->hostname.empty( ))
    {
        hostent *hptr = gethostbyname( _description->hostname.c_str() );
        if( hptr )
            memcpy(&socketAddress.sin_addr.s_addr, hptr->h_addr,hptr->h_length);
    }

    EQINFO << "Address " << inet_ntoa( socketAddress.sin_addr )
           << ":" << socketAddress.sin_port << endl;
}

//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool SocketConnection::listen()
{
    EQASSERT( _description->type == Connection::TYPE_TCPIP );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    if( !_createSocket())
        return false;

    sockaddr_in socketAddress;
    const size_t size = sizeof( sockaddr_in ); 

    _parseAddress( socketAddress ); //TODO restrict IP

    const bool bound = (::bind(_readFD, (sockaddr *)&socketAddress, size) == 0);

    if( !bound )
    {
        EQWARN << "Could not bind socket " << _readFD << ": " 
               << strerror( errno ) << " (" << errno << ") to "
               << inet_ntoa( socketAddress.sin_addr )
               << ":" << socketAddress.sin_port << " AF " 
               << (int)socketAddress.sin_family << endl;

        close();
        return false;
    }
    else if( socketAddress.sin_port == 0 )
        EQINFO << "Bound to port " << getPort() << endl;

    const bool listening = (::listen( _readFD, 10 ) == 0);
        
    if( !listening )
    {
        EQWARN << "Could not listen on socket: " << strerror( errno ) << endl;
        close();
        return false;
    }

    // get socket parameters
    sockaddr_in address; // must use new sockaddr_in variable !?!
    socklen_t   used = sizeof(address);
    getsockname( _readFD, (struct sockaddr *)&address, &used ); 

    _description->TCPIP.port = ntohs( address.sin_port );

    if( _description->hostname.empty( ))
    {
        if( address.sin_addr.s_addr == INADDR_ANY )
        {
            char hostname[256];
            gethostname( hostname, 256 );
            _description->hostname = hostname;
        }
        else
            _description->hostname = inet_ntoa( address.sin_addr );
    }
    
    _state       = STATE_LISTENING;

    EQINFO << "Listening on " << _description->hostname << "["
           << inet_ntoa( socketAddress.sin_addr )
           << "]:" << _description->TCPIP.port << endl;
    
    return true;
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
        EQWARN << "accept failed: " << strerror( errno ) << endl;
        return NULL;
    }

    _tuneSocket( fd );

    SocketConnection* newConnection = new SocketConnection;

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->type         = Connection::TYPE_TCPIP;
    newConnection->_description->bandwidthKBS = _description->bandwidthKBS;
    newConnection->_description->hostname     = inet_ntoa( newAddress.sin_addr);
    newConnection->_description->TCPIP.port   = ntohs( newAddress.sin_port );

    EQINFO << "accepted connection from "
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
