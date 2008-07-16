
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#define NOMINMAX
#include <limits>

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
#  ifndef WSA_FLAG_SDP
#    define WSA_FLAG_SDP 0x40
#  endif
#  define EQ_SOCKET_ERROR getErrorString( GetLastError( )) << \
    "(" << GetLastError() << ")"
#  include "socketConnectionWin32.cpp"

#else
#  define EQ_SOCKET_ERROR strerror( errno ) << "(" << errno << ")"
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/socket.h>
#  ifndef AF_INET_SDP
#    define AF_INET_SDP 27
#  endif

#  include "socketConnectionPosix.cpp"
#endif

namespace eq
{
namespace net
{
bool SocketConnection::_createSocket()
{
#ifdef WIN32
    const DWORD flags = _description->type == CONNECTIONTYPE_SDP ?
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_SDP :
                            WSA_FLAG_OVERLAPPED;

    const SOCKET fd = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0,0,flags );

    if( _description->type == CONNECTIONTYPE_SDP )
        EQINFO << "Created SDP socket" << endl;
#else
    Socket fd;
    if( _description->type == CONNECTIONTYPE_SDP )
        fd = ::socket( AF_INET_SDP, SOCK_STREAM, IPPROTO_TCP );
    else
        fd = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
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

void SocketConnection::_tuneSocket( const Socket fd )
{
    const int on         = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
}


bool SocketConnection::_parseAddress( sockaddr_in& socketAddress )
{
    socketAddress.sin_family      = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddress.sin_port        = htons( _description->TCPIP.port );
    memset( &(socketAddress.sin_zero), 0, 8 ); // zero the rest

    const std::string& hostname = _description->getHostname();
    if( !hostname.empty( ))
    {
        hostent *hptr = gethostbyname( hostname.c_str( ));
        if( hptr )
            memcpy(&socketAddress.sin_addr.s_addr, hptr->h_addr,hptr->h_length);
        else
        {
            EQWARN << "Can't resolve host " << hostname << endl;
            return false;
        }
    }

    EQINFO << "Address " << inet_ntoa( socketAddress.sin_addr )
           << ":" << ntohs( socketAddress.sin_port ) << endl;
    return true;
}
//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool SocketConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP || 
              _description->type == CONNECTIONTYPE_SDP );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged();

    if( !_createSocket())
        return false;

    sockaddr_in socketAddress;
    const size_t size = sizeof( sockaddr_in ); 

    if( !_parseAddress( socketAddress ))
    {
        EQWARN << "Can't parse connection parameters" << endl;
        close();
        return false;
    }

    const bool bound = (::bind(_readFD, (sockaddr *)&socketAddress, size) == 0);

    if( !bound )
    {
        EQWARN << "Could not bind socket " << _readFD << ": " 
               << EQ_SOCKET_ERROR << " to "
               << inet_ntoa( socketAddress.sin_addr )
               << ":" << ntohs( socketAddress.sin_port ) << " AF " 
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
    if( !_createReadEvents( ))
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

    const std::string& hostname = _description->getHostname();
    if( hostname.empty( ))
    {
        if( address.sin_addr.s_addr == INADDR_ANY )
        {
            char cHostname[256];
            gethostname( cHostname, 256 );
            _description->setHostname( cHostname );
        }
        else
            _description->setHostname( inet_ntoa( address.sin_addr ));
    }
    
    _state = STATE_LISTENING;
#ifdef WIN32
    _startAccept();
#endif
    _fireStateChanged();

    EQINFO << "Listening on " << _description->getHostname() << "["
           << inet_ntoa( socketAddress.sin_addr ) << "]:" 
           << _description->TCPIP.port << " (" << _description->toString()
           << ")" << endl;
    
    return true;
}

uint16_t SocketConnection::getPort() const
{
    sockaddr_in address;
    socklen_t used = sizeof(address);
    getsockname( _readFD, (struct sockaddr *) &address, &used ); 
    return ntohs(address.sin_port);
}

}
}
