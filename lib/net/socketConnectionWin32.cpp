
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <Mswsock.h>

using namespace eq::base;

#ifdef WIN32
namespace eq
{
namespace net
{
SocketConnection::SocketConnection( const ConnectionType type )
        : _overlappedAcceptData( 0 )
        , _overlappedSocket( INVALID_SOCKET )
{
    memset( &_overlapped, 0, sizeof( _overlapped ));

    EQASSERT( type == CONNECTIONTYPE_TCPIP || type == CONNECTIONTYPE_SDP );
    _description =  new ConnectionDescription;
    _description->type = type;
    _description->bandwidth = (type == CONNECTIONTYPE_TCPIP) ?
                                  102400 : 819200;
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
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP ||
              _description->type == CONNECTIONTYPE_SDP );
    if( _state != STATE_CLOSED )
        return false;

    if( _description->TCPIP.port == 0 )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged();

    if( !_createSocket( ))
        return false;

    sockaddr_in socketAddress;
    if( !_parseAddress( socketAddress ))
    {
        EQWARN << "Can't parse connection parameters" << std::endl;
        close();
        return false;
    }

    if( socketAddress.sin_addr.s_addr == 0 )
    {
        EQWARN << "Refuse to connect to 0.0.0.0" << std::endl;
        close();
        return false;
    }

    const bool connected = WSAConnect( _readFD, (sockaddr*)&socketAddress, 
                                       sizeof(socketAddress), 0, 0, 0, 0 ) == 0;
    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
               << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR
               << std::endl;
        close();
        return false;
    }

    _initAIORead();
    _state = STATE_CONNECTED;
    _fireStateChanged();
    return true;
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    if( isListening( ))
        _exitAIOAccept();
    else
        _exitAIORead();

    _state = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    const bool closed = ( ::closesocket(_readFD) == 0 );
    if( !closed )
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << std::endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handle
//----------------------------------------------------------------------
void SocketConnection::_initAIORead()
{
    _overlapped.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _overlapped.hEvent );

    if( !_overlapped.hEvent )
        EQERROR << "Can't create event for AIO notification: " 
                << EQ_SOCKET_ERROR << std::endl;
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

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
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
        EQERROR << "Could not create accept socket: " << EQ_SOCKET_ERROR
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
        EQERROR << "Could not start accept operation: " << EQ_SOCKET_ERROR 
                << ", closing connection" << std::endl;
        close();
    }
}
    
ConnectionPtr SocketConnection::acceptSync()
{
    CHECK_THREAD( _recvThread );
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
        EQWARN << "Accept completion failed: " << EQ_SOCKET_ERROR 
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

    newConnection->_readFD  = _overlappedSocket;
    newConnection->_writeFD = _overlappedSocket;
    newConnection->_initAIORead();
    _overlappedSocket       = INVALID_SOCKET;

    newConnection->_state                   = STATE_CONNECTED;
    newConnection->_description->bandwidth  = _description->bandwidth;
    newConnection->_description->TCPIP.port = ntohs( remote->sin_port );
    newConnection->_description->setHostname( inet_ntoa( remote->sin_addr ));

    EQINFO << "accepted connection from " << inet_ntoa( remote->sin_addr ) 
           << ":" << ntohs( remote->sin_port ) << std::endl;
    return newConnection;
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void SocketConnection::readNB( void* buffer, const uint64_t bytes )
{
    if( _state == STATE_CLOSED )
        return;

    WSABUF wsaBuffer = { EQ_MIN( bytes, 1048576 ),
                         reinterpret_cast< char* >( buffer ) };
    DWORD  got   = 0;
    DWORD  flags = 0;

    ResetEvent( _overlapped.hEvent );
    if( WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, &_overlapped, 0 ) != 0 &&
        GetLastError() != WSA_IO_PENDING )
    {
        EQWARN << "Could not start overlapped receive: " << EQ_SOCKET_ERROR
               << ", closing connection" << std::endl;
        close();
    }
}

int64_t SocketConnection::readSync( void* buffer, const uint64_t bytes )
{
    CHECK_THREAD( _recvThread );

    if( _readFD == INVALID_SOCKET )
    {
        EQERROR << "Invalid read handle" << std::endl;
        return -1;
    }

    DWORD got   = 0;
    DWORD flags = 0;
    if( !WSAGetOverlappedResult( _readFD, &_overlapped, &got, TRUE, &flags ))
    {
        if( GetLastError() == WSASYSCALLFAILURE ) // happens sometimes!?
            return 0;

        EQWARN << "Read complete failed: " << EQ_SOCKET_ERROR 
               << ", closing connection" << std::endl;
        close();
        return -1;
    }

    if( got == 0 )
    {
        EQWARN << "Read operation returned with nothing read"
               << ", closing connection." << std::endl;
        close();
        return -1;
    }

    return got;
}

int64_t SocketConnection::write( const void* buffer, const uint64_t bytes)
{
    if( _writeFD == INVALID_SOCKET )
        return -1;

    DWORD  wrote;
    WSABUF wsaBuffer = 
        { 
            EQ_MIN( bytes, 1048576 ),
            const_cast<char*>( static_cast< const char* >( buffer )) 
        };

    while( true )
    {
        if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // ok
            return wrote;

        // error
        if( GetLastError( ) != WSAEWOULDBLOCK )
        {
            EQWARN << "Error during write: " << EQ_SOCKET_ERROR << std::endl;
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
            EQWARN << "Error during select: " << EQ_SOCKET_ERROR << std::endl;
            return -1;
        }
#endif
    }

    EQUNREACHABLE;
    return -1;
}
}
}
#else
#  error "File is only for WIN32 builds"
#endif
