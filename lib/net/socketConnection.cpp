
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "socketConnection.h"

#include "connectionDescription.h"
#include "node.h"

#include <eq/base/base.h>
#include <eq/base/log.h>

#include <errno.h>
#include <sstream>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
#  define EQ_SOCKET_ERROR getErrorString( GetLastError( )) << "(" << GetLastError() << ")"
#  include "socketConnectionWin32.cpp"

#else
#  define EQ_SOCKET_ERROR strerror( errno ) << "(" << errno << ")"
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/socket.h>

#  include "socketConnectionPosix.cpp"
#endif


bool SocketConnection::_createSocket()
{
#ifdef WIN32
    const Socket fd = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0 );
#else
    const Socket fd = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
#endif
    if( fd == INVALID_SOCKET )
    {
        EQERROR << "Could not create socket: " << EQ_SOCKET_ERROR << endl;
        return false;
    }

    _tuneSocket( fd );

    _readFD  = fd;
    _writeFD = fd; // TCP/IP sockets are bidirectional
    return true;
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
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP );

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
               << EQ_SOCKET_ERROR << " to "
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
        EQWARN << "Could not listen on socket: " << EQ_SOCKET_ERROR << endl;
        close();
        return false;
    }

#ifdef WIN32
    if( !_createReadEvent( ))
    {
        close();
        return false;
    }
    // setup data structures needed by AcceptEx
    _overlappedAcceptData = malloc( 2*( sizeof( sockaddr_in ) + 16 ));
#endif

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

uint16_t SocketConnection::getPort() const
{
    sockaddr_in address;
    socklen_t used = sizeof(address);
    getsockname( _readFD, (struct sockaddr *) &address, &used ); 
    return ntohs(address.sin_port);
}

