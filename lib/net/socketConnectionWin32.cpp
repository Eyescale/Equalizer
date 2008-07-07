
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <Mswsock.h>

using namespace eq::base;
using namespace std;

namespace eqNet
{
SocketConnection::SocketConnection( const ConnectionType type )
    : _overlappedPending( false )
    , _overlappedAcceptData( 0 )
    , _overlappedSocket( INVALID_SOCKET )
    , _receivedUsedBytes( std::numeric_limits< uint64_t >::max( ))
    , _receivedDataEvent( 0 )
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
    _fireStateChanged();

    if( !_createSocket( ))
        return false;

    sockaddr_in socketAddress;
    if( !_parseAddress( socketAddress ))
    {
        EQWARN << "Can't parse connection parameters" << endl;
        close();
        return false;
    }

    if( socketAddress.sin_addr.s_addr == 0 )
    {
        EQWARN << "Refuse to connect to 0.0.0.0" << endl;
        close();
        return false;
    }

    const bool connected = WSAConnect( _readFD, (sockaddr*)&socketAddress, 
                                       sizeof(socketAddress), 0, 0, 0, 0 ) == 0;
    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
             << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR << endl;
        close();
        return false;
    }
 
    if( !_createReadEvents( ))
    {
        close();
        return false;
    }

    _state = STATE_CONNECTED;
    _pendingBuffer.resize( MIN_BUFFER_SIZE );
    _receivedBuffer.resize( MIN_BUFFER_SIZE );
    _startReceive(); // start first overlapped receive
    _fireStateChanged();
    return true;
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    if( _state == STATE_LISTENING )
        free( _overlappedAcceptData );
    _state = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    if( _overlapped.hEvent )
    {
        CloseHandle( _overlapped.hEvent );
        _overlapped.hEvent = 0;
    }
    if( _receivedDataEvent )
    {
        CloseHandle( _receivedDataEvent );
        _receivedDataEvent = 0;
    }

    const bool closed = ( ::closesocket(_readFD) == 0 );
    if( !closed )
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;

    _overlappedPending = false;
    _pendingBuffer.clear();
    _receivedBuffer.clear();
    _receivedUsedBytes = std::numeric_limits< uint64_t >::max();
    _fireStateChanged();
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
ConnectionPtr SocketConnection::accept()
{
    CHECK_THREAD( _recvThread );
    if( _state != STATE_LISTENING )
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
    newConnection->_pendingBuffer.resize( MIN_BUFFER_SIZE );
    newConnection->_receivedBuffer.resize( MIN_BUFFER_SIZE );

    _overlappedSocket  = INVALID_SOCKET;
    _overlappedPending = false;
    _startAccept();

    if( !newConnection->_createReadEvents( ))
    {
        newConnection->close();
        return 0;
    }

    newConnection->_startReceive(); // start first overlapped receive

    EQINFO << "accepted connection from " << inet_ntoa( remote->sin_addr ) 
           << ":" << ntohs( remote->sin_port ) <<endl;

    return newConnection;
}

bool SocketConnection::_createReadEvents()
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

    // The event to signal that our internal buffer has data is always set!
    _receivedDataEvent = CreateEvent( 0, FALSE, TRUE, 0 );
    if( !_receivedDataEvent )
    {
        EQERROR << "Can't create event for read notification: " 
                << EQ_SOCKET_ERROR << endl;
        return false;
    }

    return true;
}

Connection::ReadNotifier SocketConnection::getReadNotifier() const
{
    CHECK_THREAD( _recvThread );

    if( _state == STATE_CLOSED )
        return 0;

    if( _receivedUsedBytes < _receivedBuffer.size )
        return _receivedDataEvent;

    EQASSERT( _overlappedPending );
    return _overlapped.hEvent;
}

void SocketConnection::_startReceive()
{
    EQASSERT( _state == STATE_CONNECTED );

    if( _overlappedPending )
        return;

    _pendingBuffer.resize( _pendingBuffer.getMaxSize( ));

    WSABUF wsaBuffer = { _pendingBuffer.size, _pendingBuffer.data };
    DWORD  got   = 0;
    DWORD  flags = 0;

    if( WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, &_overlapped, 0 )==0 ||
        GetLastError() == WSA_IO_PENDING )
    {
        _overlappedPending = true;
        return;
    }

    EQWARN << "Could not start overlapped receive: " << EQ_SOCKET_ERROR << endl;
}

void SocketConnection::_startAccept()
{
    EQASSERT( _state == STATE_LISTENING );
    if( _overlappedPending )
        return;

    _overlappedPending = true;

    // Create new accept socket
    const DWORD flags = _description->type == CONNECTIONTYPE_SDP ?
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_SDP :
                            WSA_FLAG_OVERLAPPED;

    _overlappedSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0,
        flags );

    if( _description->type == CONNECTIONTYPE_SDP )
        EQINFO << "Created SDP accept socket" << endl;

    if( _overlappedSocket == INVALID_SOCKET )
    {
        EQERROR << "Could not create accept socket: " << EQ_SOCKET_ERROR
                << ", closing socket" << endl;
        close();
        return;
    }

    const int on = 1;
    setsockopt( _overlappedSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
        reinterpret_cast<const char*>( &on ), sizeof( on ));

    // Start accept
    DWORD got;
    if( !AcceptEx( _readFD, _overlappedSocket, _overlappedAcceptData, 0,
                   sizeof( sockaddr_in ) + 16, sizeof( sockaddr_in ) + 16,
                   &got, &_overlapped ) &&
         GetLastError() != WSA_IO_PENDING )
    {
        EQERROR << "Could not start accept operation: " << EQ_SOCKET_ERROR 
                << ", closing connection" << endl;
        close();
    }
}
    
int64_t SocketConnection::read( void* buffer, const uint64_t bytes )
{
    CHECK_THREAD( _recvThread );

    int64_t bytesCopied = 0;

    // First use data left in already received buffer
    if( _receivedUsedBytes < _receivedBuffer.size )
    {
        const uint64_t left = _receivedBuffer.size - _receivedUsedBytes;
        bytesCopied = EQ_MIN( left, bytes );
        EQASSERT( bytesCopied > 0 );

        memcpy( buffer, _receivedBuffer.data + _receivedUsedBytes,
                bytesCopied );
        _receivedUsedBytes += bytesCopied;

        if( _receivedUsedBytes == _receivedBuffer.size )
            _fireStateChanged(); // the read notifier changed (internal->nw)
    }
    else // then complete the pending read operation
    {
        EQASSERT( _overlappedPending );
        if( _readFD == INVALID_SOCKET )
        {
            EQERROR << "Invalid read handle" << endl;
            return -1;
        }

        DWORD got   = 0;
        DWORD flags = 0;
        if( !WSAGetOverlappedResult( _readFD, &_overlapped, &got, TRUE, &flags ))
        {
            if( GetLastError() == WSASYSCALLFAILURE ) // happens sometimes!?
                return 0;

            EQWARN << "Read complete failed: " << EQ_SOCKET_ERROR 
                << ", closing connection" << endl;
            close();
            return -1;
        }
        if( got == 0 )
        {
            EQWARN << "Read operation returned with nothing read"
                   << ", closing connection." << endl;
            close();
            return -1;
        }

        EQASSERTINFO( got <= _pendingBuffer.size, 
                      got << " max " << _pendingBuffer.size );

        // get data from received buffer
        _receivedBuffer.swap( _pendingBuffer );
        _receivedBuffer.resize( got );

        bytesCopied = EQ_MIN( bytes, got );
        EQASSERT( bytesCopied > 0 );

        memcpy( buffer, _receivedBuffer.data, bytesCopied );
        _receivedUsedBytes = bytesCopied;

        // Tune receive buffer and kick off new receive if needed
        _overlappedPending = false;
        _pendingBuffer.resize( EQ_MIN( bytes, MAX_BUFFER_SIZE ));
        _startReceive();

        if( _receivedUsedBytes < _receivedBuffer.size )
            _fireStateChanged(); // the read notifier changed (nw->internal)
    }

    if( _receivedUsedBytes >= _receivedBuffer.size && !_overlappedPending )
    {
        EQERROR << "Internal buffer exhausted and no read operation "
                << "is pending, closing connection" << endl;
        close();
    }

    SetEvent( _receivedDataEvent ); // WaitForMultipleObjects resets the event
    return bytesCopied;
}

int64_t SocketConnection::write( const void* buffer, const uint64_t bytes) const
{
    if( _writeFD == INVALID_SOCKET )
        return -1;

    WSABUF wsaBuffer = { bytes,
                      const_cast<char*>( static_cast< const char* >( buffer ))};
    DWORD wrote;

    while( true )
    {
        if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // success
            return wrote;

        // error
        if( GetLastError( ) != WSAEWOULDBLOCK )
        {
            EQWARN << "Error during write: " << EQ_SOCKET_ERROR << endl;
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
	    	EQWARN << "Error during select: " << EQ_SOCKET_ERROR <<endl;
		    return -1;
	    }
#endif
    }

    EQUNREACHABLE;
    return -1;
}
}
