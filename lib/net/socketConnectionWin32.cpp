
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <Mswsock.h>

using namespace eqNet;
using namespace eqBase;
using namespace std;

SocketConnection::SocketConnection( const ConnectionType type )
    : _overlappedBuffer( 0 ),
      _overlappedAcceptData( 0 ),
      _overlappedSocket( INVALID_SOCKET ),
      _overlappedPending( false )
{
    memset( &_overlapped, 0, sizeof( _overlapped ));

    EQASSERT( type == CONNECTIONTYPE_TCPIP || type == CONNECTIONTYPE_SDP );
    _description =  new ConnectionDescription;
    _description->type = type;
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

    if( !_createSocket( ))
        return false;

    // TODO: execute launch command

    sockaddr_in socketAddress;
    _parseAddress( socketAddress );

    const bool connected = WSAConnect( _readFD, (sockaddr*)&socketAddress, 
                                       sizeof(socketAddress), 0, 0, 0, 0 ) == 0;
    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
             << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR << endl;
        close();
        return false;
    }
 
    if( !_createReadEvent( ))
    {
        close();
        return false;
    }

    _state = STATE_CONNECTED;
    return true;
}

void SocketConnection::_tuneSocket( const Socket fd )
{
    const int on         = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));

#if 1
    const int bufferSize = 1*1024*1024;
    setsockopt( fd, SOL_SOCKET, SO_SNDBUF, 
        reinterpret_cast<const char*>( &bufferSize ), sizeof( bufferSize ));
    setsockopt( fd, SOL_SOCKET, SO_RCVBUF, 
        reinterpret_cast<const char*>( &bufferSize ), sizeof( bufferSize ));
#endif
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    if( _state == STATE_LISTENING )
        free( _overlappedAcceptData );
    _state   = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    if( _overlapped.hEvent )
    {
        CloseHandle( _overlapped.hEvent );
        _overlapped.hEvent = 0;
    }

    const bool closed = ( ::closesocket(_readFD) == 0 );
    if( !closed )
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
RefPtr<Connection> SocketConnection::accept()
{
    if( _state != STATE_LISTENING )
        return 0;

    if( !_overlappedPending && !_startAccept( ))
        return 0;

    // complete accept
    DWORD got   = 0;
    DWORD flags = 0;
    if( !WSAGetOverlappedResult( _readFD, &_overlapped, &got, TRUE, &flags ))
    {
        EQWARN << "Accept complete failed: " << EQ_SOCKET_ERROR 
               << ", socket closed" << endl;
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

    SocketConnection* newConnection = new SocketConnection( _description->type);

    newConnection->_readFD      = _overlappedSocket;
    newConnection->_writeFD     = _overlappedSocket;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->bandwidthKBS = _description->bandwidthKBS;
    newConnection->_description->setHostname( inet_ntoa( remote->sin_addr ));
    newConnection->_description->TCPIP.port   = ntohs( remote->sin_port );

    _overlappedSocket  = INVALID_SOCKET;
    _overlappedPending = false;

    if( !newConnection->_createReadEvent( ))
    {
        newConnection->close();
        return 0;
    }

    EQINFO << "accepted connection from " << inet_ntoa( remote->sin_addr ) 
           << ":" << ntohs( remote->sin_port ) <<endl;

    return newConnection;
}

bool SocketConnection::_createReadEvent()
{
    // Win32 uses overlapped (async) IO. Populate the overlapped structure used
    // for ConnectionSet::select, recv and accept.

    _overlapped.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    if( !_overlapped.hEvent )
    {
        EQERROR << "Can't create event for read notification: " 
                << EQ_SOCKET_ERROR << endl;
        return false;
    }
    return true;
}

Connection::ReadNotifier SocketConnection::getReadNotifier()
{
    CHECK_THREAD( _recvThread );

    if( _overlappedPending )
        return _overlapped.hEvent;

    _overlappedPending = true;

    if( _state == STATE_LISTENING )
    {
        if( _startAccept( ))
            return _overlapped.hEvent;
    }
    else // start overlapped receive
    {
        EQASSERT( _state == STATE_CONNECTED );

        WSABUF wsaBuffer = { 8, reinterpret_cast<char*>( &_overlappedBuffer )};
        DWORD got   = 0;
        DWORD flags = 0;

        if( WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, &_overlapped, 
                     0 ) == 0 ||
            GetLastError() == WSA_IO_PENDING )
        {
            return _overlapped.hEvent;
        }
    }

    EQERROR << "Could not start overlapped operation: " << EQ_SOCKET_ERROR 
            << ", closing socket" << endl;
    close();
    return 0;
}

bool SocketConnection::_startAccept()
{
    if( !_createAcceptSocket( ))
        return false;

    DWORD got;
    if( !AcceptEx( _readFD, _overlappedSocket, _overlappedAcceptData, 0,
                   sizeof( sockaddr_in ) + 16, sizeof( sockaddr_in ) + 16,
                   &got, &_overlapped ) &&
         GetLastError() != WSA_IO_PENDING )
    {
        EQERROR << "Could not start accept operation: " << EQ_SOCKET_ERROR 
                << endl;
        return false;
    }
    return true;
}

bool SocketConnection::_createAcceptSocket()
{
    const DWORD flags = _description->type == CONNECTIONTYPE_SDP ?
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_SDP :
                            WSA_FLAG_OVERLAPPED;

    _overlappedSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0,
                                   flags );

    if( _description->type == CONNECTIONTYPE_SDP )
        EQINFO << "Created SDP accept socket" << endl;

    if( _overlappedSocket == INVALID_SOCKET )
    {
        EQERROR << "Could not create accept socket: " << EQ_SOCKET_ERROR <<endl;
        return false;
    }
    const int on         = 1;
    setsockopt( _overlappedSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    return true;
}
    
int64_t SocketConnection::read( void* buffer, const uint64_t bytes )
{
    CHECK_THREAD( _recvThread );
    if( _readFD == INVALID_SOCKET )
    {
        EQERROR << "Invalid read handle" << endl;
        return -1;
    }

    DWORD got   = 0;
    DWORD flags = 0;
    if( _overlappedPending )
    {
        EQASSERT( bytes >= 8 ); // Eq use case
    }
    else // kick of new recv
    {
        WSABUF wsaBuffer = { MIN( 512*1024, bytes ), 
                             static_cast<char*>( buffer )};

        ResetEvent( _overlapped.hEvent );
        int ret   = WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, &_overlapped,
                             0 );
        int error = (ret==0) ? 0 : GetLastError();

        if( error == WSAESHUTDOWN || error == WSAECONNRESET ) //EOF
        {
            EQWARN << "Read failed: " << EQ_SOCKET_ERROR << ", socket closed"
                   << endl;
            close();
            return -1;
        }

        if( error && error != WSA_IO_PENDING )
        {
            EQWARN << "Error during read: " << EQ_SOCKET_ERROR << endl;
            return -1;
        }
    }

    if( !WSAGetOverlappedResult( _readFD, &_overlapped, &got, TRUE, &flags )
        || got == 0 )
    {
        EQWARN << "Read complete failed: " << EQ_SOCKET_ERROR 
               << ", socket closed" << endl;
        close();
        return -1;
    }

    if( _overlappedPending ) // get data from internal buffer
    {
        EQASSERT( got == 8, EQ_SOCKET_ERROR );
        *static_cast<uint64_t*>(buffer) = _overlappedBuffer;
        _overlappedPending = false;
    }

    // Set event if there is still data on the socket
    unsigned long nBytesPending = 0;
    ioctlsocket( _readFD, FIONREAD, &nBytesPending );
    if( nBytesPending > 0 )
        if( SetEvent( _overlapped.hEvent ) == 0 ) // error
            EQWARN << "SetEvent failed: " << EQ_SOCKET_ERROR << endl;

    EQASSERT( got > 0 );
    return got;
}

int64_t SocketConnection::write( const void* buffer, const uint64_t bytes) const
{
    if( _writeFD == INVALID_SOCKET )
        return -1;

    WSABUF wsaBuffer = { MIN( 512*1024, bytes ),
        const_cast<char*>( static_cast< const char* >( buffer ))};
    DWORD wrote;

    if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // success
        return wrote;

    // error
    if( GetLastError( ) != WSAEWOULDBLOCK )
    {
        EQWARN << "Error during write: " << EQ_SOCKET_ERROR << endl;
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
