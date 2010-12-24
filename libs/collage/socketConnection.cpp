
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "socketConnection.h"

#include "connectionDescription.h"

#include <co/base/os.h>
#include <co/base/log.h>
#include <co/base/sleep.h>

#include <limits>
#include <sstream>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#  include <mswsock.h>
#  ifndef WSA_FLAG_SDP
#    define WSA_FLAG_SDP 0x40
#  endif
#  define EQ_RECV_TIMEOUT 250 /*ms*/
#else
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/errno.h>
#  include <sys/socket.h>

#  ifndef AF_INET_SDP
#    define AF_INET_SDP 27
#  endif
#endif

namespace co
{
SocketConnection::SocketConnection( const ConnectionType type )
#ifdef _WIN32
        : _overlappedAcceptData( 0 )
        , _overlappedSocket( INVALID_SOCKET )
        , _overlappedDone( 0 )
#endif
{
#ifdef _WIN32
    memset( &_overlapped, 0, sizeof( _overlapped ));
#endif

    EQASSERT( type == CONNECTIONTYPE_TCPIP || type == CONNECTIONTYPE_SDP );
    _description->type = type;
    _description->bandwidth = (type == CONNECTIONTYPE_TCPIP) ?
                                  102400 : 819200; // 100MB : 800 MB

    EQVERB << "New SocketConnection @" << (void*)this << std::endl;
}

SocketConnection::~SocketConnection()
{
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool SocketConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP ||
              _description->type == CONNECTIONTYPE_SDP );
    if( _state != STATE_CLOSED )
        return false;

    if( _description->port == 0 )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged();

    if( _description->getHostname().empty( ))
        _description->setHostname( "127.0.0.1" );

    sockaddr_in address;
    if( !_parseAddress( address ))
    {
        EQWARN << "Can't parse connection parameters" << std::endl;
        return false;
    }

    if( !_createSocket( ))
        return false;

    if( address.sin_addr.s_addr == 0 )
    {
        EQWARN << "Refuse to connect to 0.0.0.0" << std::endl;
        close();
        return false;
    }

#ifdef _WIN32
    const bool connected = WSAConnect( _readFD, (sockaddr*)&address, 
                                       sizeof( address ), 0, 0, 0, 0 ) == 0;
#else
    const bool connected = (::connect( _readFD, (sockaddr*)&address, 
                                       sizeof( address )) == 0);
#endif

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
               << _description->port << "': " << co::base::sysError 
               << std::endl;
        close();
        return false;
    }

    _initAIORead();
    _state = STATE_CONNECTED;
    _fireStateChanged();
    EQINFO << "Connected " << _description->toString() << std::endl;
    return true;
}

void SocketConnection::close()
{
    if( _state == STATE_CLOSED )
        return;

    if( isListening( ))
        _exitAIOAccept();
    else if( isConnected( ))
        _exitAIORead();

    _state = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

#ifdef _WIN32
    const bool closed = ( ::closesocket(_readFD) == 0 );
#else
    const bool closed = ( ::close(_readFD) == 0 );
#endif

    if( !closed )
        EQWARN << "Could not close socket: " << co::base::sysError 
               << std::endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
#ifdef _WIN32
void SocketConnection::_initAIORead()
{
    _overlapped.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _overlapped.hEvent );

    if( !_overlapped.hEvent )
        EQERROR << "Can't create event for AIO notification: " 
                << co::base::sysError << std::endl;
}

void SocketConnection::_initAIOAccept()
{
    _initAIORead();
    _overlappedAcceptData = malloc( 2*( sizeof( sockaddr_in ) + 16 ));
}

void SocketConnection::_exitAIOAccept()
{
    if( _overlappedAcceptData )
    {
        free( _overlappedAcceptData );
        _overlappedAcceptData = 0;
    }
    
    _exitAIORead();
}
void SocketConnection::_exitAIORead()
{
    if( _overlapped.hEvent )
    {
        CloseHandle( _overlapped.hEvent );
        _overlapped.hEvent = 0;
    }
}
#else
void SocketConnection::_initAIOAccept(){ /* NOP */ }
void SocketConnection::_exitAIOAccept(){ /* NOP */ }
void SocketConnection::_initAIORead(){ /* NOP */ }
void SocketConnection::_exitAIORead(){ /* NOP */ }
#endif

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
#ifdef _WIN32
void SocketConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );

    // Create new accept socket
    const DWORD flags = _description->type == CONNECTIONTYPE_SDP ?
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_SDP :
                            WSA_FLAG_OVERLAPPED;

    EQASSERT( _overlappedAcceptData );
    EQASSERT( _overlappedSocket == INVALID_SOCKET );
    _overlappedSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0,
                                   flags );

    if( _overlappedSocket == INVALID_SOCKET )
    {
        EQERROR << "Could not create accept socket: " << co::base::sysError
                << ", closing listening socket" << std::endl;
        close();
        return;
    }

    const int on = 1;
    setsockopt( _overlappedSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
        reinterpret_cast<const char*>( &on ), sizeof( on ));

    // Start accept
    ResetEvent( _overlapped.hEvent );
    DWORD got;
    if( !AcceptEx( _readFD, _overlappedSocket, _overlappedAcceptData, 0,
                   sizeof( sockaddr_in ) + 16, sizeof( sockaddr_in ) + 16,
                   &got, &_overlapped ) &&
        GetLastError() != WSA_IO_PENDING )
    {
        EQERROR << "Could not start accept operation: " 
                << co::base::sysError << ", closing connection" << std::endl;
        close();
    }
}
    
ConnectionPtr SocketConnection::acceptSync()
{
    EQ_TS_THREAD( _recvThread );
    if( _state != STATE_LISTENING )
        return 0;

    EQASSERT( _overlappedAcceptData );
    EQASSERT( _overlappedSocket != INVALID_SOCKET );
    if( _overlappedSocket == INVALID_SOCKET )
        return 0;

    // complete accept
    DWORD got   = 0;
    DWORD flags = 0;
    if( !WSAGetOverlappedResult( _readFD, &_overlapped, &got, TRUE, &flags ))
    {
        EQWARN << "Accept completion failed: " << co::base::sysError 
               << ", closing socket" << std::endl;
        close();
        return 0;
    }

    sockaddr_in* local     = 0;
    sockaddr_in* remote    = 0;
    int          localLen  = 0;
    int          remoteLen = 0;
    GetAcceptExSockaddrs( _overlappedAcceptData, 0, sizeof( sockaddr_in ) + 16,
                          sizeof( sockaddr_in ) + 16, (sockaddr**)&local, 
                          &localLen, (sockaddr**)&remote, &remoteLen );
    _tuneSocket( _overlappedSocket );

    SocketConnection* newConnection = new SocketConnection(_description->type );
    ConnectionPtr connection( newConnection ); // to keep ref-counting correct

    newConnection->_readFD  = _overlappedSocket;
    newConnection->_writeFD = _overlappedSocket;
    newConnection->_initAIORead();
    _overlappedSocket       = INVALID_SOCKET;

    newConnection->_state                  = STATE_CONNECTED;
    newConnection->_description->bandwidth = _description->bandwidth;
    newConnection->_description->port      = ntohs( remote->sin_port );
    newConnection->_description->setHostname( inet_ntoa( remote->sin_addr ));

    EQINFO << "accepted connection from " << inet_ntoa( remote->sin_addr ) 
           << ":" << ntohs( remote->sin_port ) << std::endl;
    return connection;
}

#else // !_WIN32

void SocketConnection::acceptNB(){ /* NOP */ }
 
ConnectionPtr SocketConnection::acceptSync()
{
    if( _state != STATE_LISTENING )
        return 0;

    sockaddr_in newAddress;
    socklen_t   newAddressLen = sizeof( newAddress );

    Socket    fd;
    unsigned  nTries = 1000;
    do
        fd = ::accept( _readFD, (sockaddr*)&newAddress, &newAddressLen );
    while( fd == INVALID_SOCKET && errno == EINTR && --nTries );

    if( fd == INVALID_SOCKET )
    {
      EQWARN << "accept failed: " << co::base::sysError << std::endl;
        return 0;
    }

    _tuneSocket( fd );

    SocketConnection* newConnection = new SocketConnection( _description->type);

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->bandwidth = _description->bandwidth;
    newConnection->_description->setHostname( inet_ntoa( newAddress.sin_addr ));
    newConnection->_description->port      = ntohs( newAddress.sin_port );

    EQVERB << "accepted connection from " << inet_ntoa(newAddress.sin_addr) 
           << ":" << ntohs( newAddress.sin_port ) << std::endl;

    return newConnection;
}

#endif // !_WIN32



#ifdef _WIN32
//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void SocketConnection::readNB( void* buffer, const uint64_t bytes )
{
    if( _state == STATE_CLOSED )
        return;

    WSABUF wsaBuffer = { EQ_MIN( bytes, 65535 ),
                         reinterpret_cast< char* >( buffer ) };
    DWORD  flags = 0;

    ResetEvent( _overlapped.hEvent );
    _overlappedDone = 0;

    const int result = WSARecv( _readFD, &wsaBuffer, 1, &_overlappedDone,
                                &flags, &_overlapped, 0 );
    if( result == 0 ) // got data already
    {
        if( _overlappedDone == 0 ) // socket closed
        {
            EQINFO << "Got EOF, closing connection" << std::endl;
            close();
        }
        SetEvent( _overlapped.hEvent );
    }
    else if( GetLastError() != WSA_IO_PENDING )
    {
        EQWARN << "Could not start overlapped receive: " << co::base::sysError
               << ", closing connection" << std::endl;
        close();
    }
}

int64_t SocketConnection::readSync( void* buffer, const uint64_t bytes,
                                    const bool block )
{
    EQ_TS_THREAD( _recvThread );

    if( _readFD == INVALID_SOCKET )
    {
        EQERROR << "Invalid read handle" << std::endl;
        return READ_ERROR;
    }

    if( _overlappedDone > 0 )
        return _overlappedDone;

    DWORD got   = 0;
    DWORD flags = 0;
    DWORD startTime = 0;

    while( true )
    {
        if( WSAGetOverlappedResult(_readFD, &_overlapped, &got, block, &flags ))
            return got;

        const int err = WSAGetLastError();
        if( err == ERROR_SUCCESS || got > 0 )
        {
            EQWARN << "Got " << co::base::sysError << " with " << got
                   << " bytes on " << _description << std::endl;
            return got;
        }

        if( startTime == 0 )
            startTime = GetTickCount();

        switch( err )
        {
            case WSA_IO_INCOMPLETE:
                return READ_TIMEOUT;

            case WSASYSCALLFAILURE:  // happens sometimes!?
            case WSA_IO_PENDING:
                if( GetTickCount() - startTime > EQ_RECV_TIMEOUT ) // timeout   
                {
                    EQWARN << "Error timeout " << std::endl;
                    return READ_ERROR;
                }

                EQWARN << "WSAGetOverlappedResult error loop"
                       << std::endl;
                co::base::sleep( 1 ); // one millisecond to recover
                break;

            default:
                EQWARN << "Got " << co::base::sysError 
                       << ", closing connection" << std::endl;
                close();
                return READ_ERROR;
        }
    }
}

int64_t SocketConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED || _writeFD == INVALID_SOCKET )
        return -1;

    DWORD  wrote;
    WSABUF wsaBuffer = 
        { 
            EQ_MIN( bytes, 65535 ),
            const_cast<char*>( static_cast< const char* >( buffer )) 
        };

    while( true )
    {
        if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // ok
            return wrote;

        // error
        if( GetLastError( ) != WSAEWOULDBLOCK )
        {
            EQWARN << "Error during write: " << co::base::sysError << " on "
                   << _description << std::endl;
            return -1;
        }

        // Buffer full - try again
#if 1
        // Wait for writable socket
        fd_set set;
        FD_ZERO( &set );
        FD_SET( _writeFD, &set );

        const int result = select( _writeFD+1, 0, &set, 0, 0 );
        if( result <= 0 )
        {
            EQWARN << "Error during select: " << co::base::sysError 
                   << std::endl;
            return -1;
        }
#endif
    }

    EQUNREACHABLE;
    return -1;
}
#endif // _WIN32

bool SocketConnection::_createSocket()
{
#ifdef _WIN32
    const DWORD flags = _description->type == CONNECTIONTYPE_SDP ?
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_SDP :
                            WSA_FLAG_OVERLAPPED;

    const SOCKET fd = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0,0,flags );

    if( _description->type == CONNECTIONTYPE_SDP )
        EQINFO << "Created SDP socket" << std::endl;
#else
    Socket fd;
    if( _description->type == CONNECTIONTYPE_SDP )
        fd = ::socket( AF_INET_SDP, SOCK_STREAM, IPPROTO_TCP );
    else
        fd = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
#endif

    if( fd == INVALID_SOCKET )
    {
        EQERROR << "Could not create socket: " 
                << co::base::sysError << std::endl;
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
    
#ifdef _WIN32
    const int size = 128768;
    setsockopt( fd, SOL_SOCKET, SO_RCVBUF, 
                reinterpret_cast<const char*>( &size ), sizeof( size ));
    setsockopt( fd, SOL_SOCKET, SO_SNDBUF,
                reinterpret_cast<const char*>( &size ), sizeof( size ));
#endif
}

bool SocketConnection::_parseAddress( sockaddr_in& address )
{
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = htonl( INADDR_ANY );
    address.sin_port        = htons( _description->port );
    memset( &(address.sin_zero), 0, 8 ); // zero the rest

    const std::string& hostname = _description->getHostname();
    if( !hostname.empty( ))
    {
        hostent *hptr = gethostbyname( hostname.c_str( ));
        if( hptr )
            memcpy( &address.sin_addr.s_addr, hptr->h_addr, hptr->h_length );
        else
        {
            EQWARN << "Can't resolve host " << hostname << std::endl;
            return false;
        }
    }

    EQVERB << "Address " << inet_ntoa( address.sin_addr ) << ":" 
           << ntohs( address.sin_port ) << std::endl;
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

    sockaddr_in address;
    const size_t size = sizeof( sockaddr_in ); 

    if( !_parseAddress( address ))
    {
        EQWARN << "Can't parse connection parameters" << std::endl;
        return false;
    }

    if( !_createSocket())
        return false;

    const bool bound = (::bind(_readFD, (sockaddr *)&address, size) == 0);

    if( !bound )
    {
        EQWARN << "Could not bind socket " << _readFD << ": " 
               << co::base::sysError << " to " << inet_ntoa( address.sin_addr )
               << ":" << ntohs( address.sin_port ) << " AF "
               << (int)address.sin_family << std::endl;

        close();
        return false;
    }
    else if( address.sin_port == 0 )
        EQINFO << "Bound to port " << _getPort() << std::endl;

    const bool listening = (::listen( _readFD, SOMAXCONN ) == 0);
        
    if( !listening )
    {
        EQWARN << "Could not listen on socket: " 
               << co::base::sysError << std::endl;
        close();
        return false;
    }

    // get socket parameters
    socklen_t used = size;
    getsockname( _readFD, (struct sockaddr *)&address, &used ); 

    _description->port = ntohs( address.sin_port );

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
    
    _initAIOAccept();
    _state = STATE_LISTENING;
    _fireStateChanged();

    EQINFO << "Listening on " << _description->getHostname() << "["
           << inet_ntoa( address.sin_addr ) << "]:" << _description->port
           << " (" << _description->toString() << ")" << std::endl;
    
    return true;
}

uint16_t SocketConnection::_getPort() const
{
    sockaddr_in address;
    socklen_t used = sizeof( address );
    getsockname( _readFD, (struct sockaddr *) &address, &used ); 
    return ntohs(address.sin_port);
}

}
