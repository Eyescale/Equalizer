
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
#  define EQ_SOCKET_ERROR getErrorString( GetLastError( ))
#else
#  define EQ_SOCKET_ERROR strerror( errno )
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/socket.h>
#endif

using namespace eqNet;
using namespace eqBase;
using namespace std;


SocketConnection::SocketConnection()
#ifdef WIN32
        : _event( 0 )
#endif
{
    _description =  new ConnectionDescription;
    _description->type = CONNECTIONTYPE_TCPIP;
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
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP );
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

#ifdef WIN32
    const bool connected = WSAConnect( _readFD, (sockaddr*)&socketAddress, 
                                       sizeof(socketAddress), 0, 0, 0, 0 ) == 0;
#else
    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);
#endif

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->hostname << ":"
             << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR << endl;
        close();
        return false;
    }
 
#ifdef WIN32
    if( !_createReadEvent( ))
    {
        close();
        return false;
    }
#endif

    _state = STATE_CONNECTED;
    return true;
}

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

void SocketConnection::_tuneSocket( const Socket fd )
{
    const int on         = 1;
#ifdef WIN32
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));

    const int bufferSize = 4*1024*1024;
    setsockopt( fd, SOL_SOCKET, SO_SNDBUF, 
        reinterpret_cast<const char*>( &bufferSize ), sizeof( bufferSize ));
    setsockopt( fd, SOL_SOCKET, SO_RCVBUF, 
        reinterpret_cast<const char*>( &bufferSize ), sizeof( bufferSize ));
#else
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof( on ));
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ));
#endif
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    _state   = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

#ifdef WIN32
    if( _event )
    {
        CloseHandle( _event );
        _event = 0;
    }

    const bool closed = ( ::closesocket(_readFD) == 0 );
#else
    const bool closed = ( ::close(_readFD) == 0 );
#endif
    if( !closed )
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
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
               << EQ_SOCKET_ERROR << " (" << errno << ") to "
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

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
RefPtr<Connection> SocketConnection::accept()
{
    if( _state != STATE_LISTENING )
        return 0;

    sockaddr_in newAddress;
    socklen_t   newAddressLen = sizeof( newAddress );

#ifdef WIN32
    Socket fd = WSAAccept( _readFD, (sockaddr*)&newAddress, &newAddressLen,0,0);
    if( fd != INVALID_SOCKET || GetLastError() != WSAEWOULDBLOCK )
        SetEvent( _event );
#else
    Socket    fd;
    unsigned  nTries = 1000;
    do
        fd = ::accept( _readFD, (sockaddr*)&newAddress, &newAddressLen );
    while( fd == INVALID_SOCKET && errno == EINTR && --nTries );
#endif

    if( fd == INVALID_SOCKET )
    {
        EQWARN << "accept failed: " << EQ_SOCKET_ERROR << endl;
        return 0;
    }

    _tuneSocket( fd );

    SocketConnection* newConnection = new SocketConnection;

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->type         = CONNECTIONTYPE_TCPIP;
    newConnection->_description->bandwidthKBS = _description->bandwidthKBS;
    newConnection->_description->hostname     = inet_ntoa( newAddress.sin_addr);
    newConnection->_description->TCPIP.port   = ntohs( newAddress.sin_port );

#ifdef WIN32
    if( !newConnection->_createReadEvent( ))
    {
        newConnection->close();
        return 0;
    }
#endif

    EQINFO << "accepted connection from " << inet_ntoa(newAddress.sin_addr) 
           << ":" << newAddress.sin_port <<endl;

    return newConnection;
}

uint16_t SocketConnection::getPort() const
{
    sockaddr_in address;
    socklen_t used = sizeof(address);
    getsockname( _readFD, (struct sockaddr *) &address, &used ); 
    return ntohs(address.sin_port);
}

#ifdef WIN32
bool SocketConnection::_createReadEvent()
{
    // On WIN32, the event is set into signaled state when data arrives. Note 
    // that this notion is different from the Unix select() which returns when
    // data is pending. To emulate the same behaviour race-condition-free, 
    // we do:
    // - create an auto-reset event
    // - WaitForMultipleObjectsEx (select) will auto-reset the event
    // - arriving data will set the event
    // - read (see below) will set the event if there is still data
    //
    // The unavoidable consequence is that read() might fail because pending
    // data was wrongly signaled when a subsequent read reads all remaining 
    // data. This has to be handled by the callee.
    //
    // Note that on listening sockets, data arrived means an incoming connect.

    _event = CreateEvent( 0, FALSE, FALSE, 0 );
    if( !_event )
    {
        EQERROR << "Can't create event for read notification" << EQ_SOCKET_ERROR
                << endl;
        return false;
    }

    if( WSAEventSelect( _readFD, _event, FD_READ | FD_ACCEPT | FD_CLOSE ) ==
        SOCKET_ERROR )
    {
        EQERROR << "Can't select events for read notification" 
                << EQ_SOCKET_ERROR << endl;
        return false;
    }
    return true;
}

int64_t SocketConnection::read( void* buffer, const uint64_t bytes )
{
    if( _readFD == INVALID_SOCKET )
    {
        EQERROR << "Invalid read handle" << endl;
        return -1;
    }

    WSABUF wsaBuffer = { bytes, static_cast<char*>( buffer )};
    DWORD got   = 0;
    DWORD flags = 0;

    int ret = WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, 0, 0 );
    int error = (ret==0) ? 0 : GetLastError();

    if( error == WSAEWOULDBLOCK )
    {
	    // Buffer empty - wait and try again
	    fd_set set;
	    FD_ZERO( &set );
	    FD_SET( _readFD, &set );

        const int result = select( _readFD+1, &set, 0, &set, 0 );
	    if( result <= 0 )
	    {
		    EQWARN << "Error during select: " << EQ_SOCKET_ERROR <<endl;
		    return -1;
	    }
    
        flags = 0;
        ret   = WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, 0, 0 );
        error = (ret==0) ? 0 : GetLastError();
    }

    if( error == WSAEWOULDBLOCK )
        return 0;

    if( error == WSAESHUTDOWN || error == WSAECONNRESET || got == 0 )//EOF
    {
        EQWARN << "Read failed, socket closed" << endl;
        close();
        return -1;
    }

    if( error )
    {
        EQWARN << "Error during read: " << EQ_SOCKET_ERROR << endl;
        return -1;
    }

    // Set event if there is still data on the socket
    unsigned long nBytesPending = 0;
    ioctlsocket( _readFD, FIONREAD, &nBytesPending );
    if( nBytesPending > 0 )
        if( SetEvent( _event ) == 0 ) // error
            EQWARN << "SetEvent failed: " << EQ_SOCKET_ERROR << endl;

    EQASSERT( got > 0 );
    return got;
}

int64_t SocketConnection::write( const void* buffer, const uint64_t bytes) const
{
    if( _writeFD == INVALID_SOCKET )
        return -1;

    WSABUF wsaBuffer = { bytes, const_cast<char*>( 
                                    static_cast< const char* >( buffer ))};
    DWORD wrote;

    if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // success
        return wrote;

    // error
    if( GetLastError( ) != WSAEWOULDBLOCK )
    {
        EQWARN << "Error during write: " << EQ_SOCKET_ERROR << "(" 
               << GetLastError() << ")" << endl;
        return -1;
	}

	// Buffer full - wait and try again
	fd_set set;
	FD_ZERO( &set );
	FD_SET( _writeFD, &set );

	const int result = select( _writeFD+1, 0, &set, 0, 0 );
	if( result <= 0 )
	{
		EQWARN << "Error during select: " << EQ_SOCKET_ERROR <<endl;
		return -1;
	}

    if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // success
        return wrote;

    EQWARN << "Error during write: " << EQ_SOCKET_ERROR << endl;
    return -1;
}
#endif // WIN32
