
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

#include "pgmConnection.h"

#ifdef EQ_PGM

#include "connectionDescription.h"

#include <co/base/os.h>
#include <co/base/log.h>

#include <errno.h>
#include <sstream>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#  include <mswsock.h>
#  include <wsrm.h>
#else
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/PGM.h>
#endif

//#define PRINT_STATS

namespace co
{
PGMConnection::PGMConnection()
        : _readFD( INVALID_SOCKET )
        , _writeFD( INVALID_SOCKET )
#ifdef _WIN32
        , _overlappedAcceptData( 0 )
        , _overlappedSocket( INVALID_SOCKET )
#endif
{
#ifdef _WIN32
    memset( &_overlapped, 0, sizeof( _overlapped ));
#endif

    _description->type = CONNECTIONTYPE_PGM;
    _description->bandwidth = 50 * 1024; // 50MB/s

    EQVERB << "New PGMConnection @" << (void*)this << std::endl;
}

PGMConnection::~PGMConnection()
{
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool PGMConnection::connect()
{
    return listen();
}

//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool PGMConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_PGM );

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

    //-- create a 'listening' read socket
    _readFD = _initSocket( address );
    
    if( _readFD == INVALID_SOCKET || !_setupReadSocket( ))
    {
        close();
        return false;
    }

    const bool listening = (::listen( _readFD, 10 ) == 0);
        
    if( !listening )
    {
        EQWARN << "Could not listen on socket: " << co::base::sysError 
               << std::endl;
        close();
        return false;
    }

    // get listening socket parameters
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

    
    //-- create a connected 'write' socket
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = htonl( INADDR_ANY );
    address.sin_port        = htons( 0 );
    memset( &(address.sin_zero), 0, 8 ); // zero the rest

    _writeFD = _initSocket( address );

    if( _writeFD == INVALID_SOCKET || !_setupSendSocket( ))
    {
        close();
        return false;
    }

    _parseAddress( address );
    
#ifdef _WIN32
    const bool connected = WSAConnect( _writeFD, (sockaddr*)&address, 
                                       size, 0, 0, 0, 0 ) == 0;
#else
    const bool connected = (::connect( _readFD, (sockaddr*)&address, 
                                       size ) == 0);
#endif

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
               << _description->port << "': " << co::base::sysError 
               << std::endl;
        close();
        return false;
    }

    _state = STATE_LISTENING;
    _fireStateChanged();

    EQINFO << "Listening on " << _description->getHostname() << "["
           << inet_ntoa( address.sin_addr ) << "]:" << _description->port
           << " (" << _description->toString() << ")" << std::endl;
    
    return true;
}

void PGMConnection::close()
{
    if( _state == STATE_CLOSED )
        return;

    _printReadStatistics();
    _printSendStatistics();

    if( isListening( ))
        _exitAIOAccept();
    else if( isConnected( ))
        _exitAIORead();

    _state = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    if( _readFD > 0 )
    {
        const std::string& iName = _description->getInterface();
        if( !iName.empty( ))
        {
            unsigned long interface;
            if( !_parseHostname( iName, interface ) ||
                ::setsockopt( _readFD, IPPROTO_RM, RM_DEL_RECEIVE_IF,
                     (char*)&interface, sizeof(uint32_t)) == SOCKET_ERROR )
            {
                EQWARN << "can't delete recv interface " <<  co::base::sysError
                       << std::endl;
            }
        }
#ifdef _WIN32
        const bool closed = ( ::closesocket( _readFD ) == 0 );
#else
        const bool closed = ( ::close( _readFD ) == 0 );
#endif
        
        if( !closed )
            EQWARN << "Could not close read socket: " << co::base::sysError
                   << std::endl;
    }

    if( _writeFD > 0 && isListening( ))
    {
#ifdef _WIN32
        const bool closed = ( ::closesocket( _writeFD ) == 0 );
#else
        const bool closed = ( ::close( _writeFD ) == 0 );
#endif

        if( !closed )
            EQWARN << "Could not close write socket: " << co::base::sysError
                   << std::endl;
    }

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
#ifdef _WIN32
void PGMConnection::_initAIORead()
{
    _overlapped.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _overlapped.hEvent );

    if( !_overlapped.hEvent )
        EQERROR << "Can't create event for AIO notification: " 
                << co::base::sysError << std::endl;
}

void PGMConnection::_initAIOAccept()
{
    _initAIORead();
    _overlappedAcceptData = malloc( 2*( sizeof( sockaddr_in ) + 16 ));
}

void PGMConnection::_exitAIOAccept()
{
    if( _overlappedAcceptData )
    {
        free( _overlappedAcceptData );
        _overlappedAcceptData = 0;
    }
    
    _exitAIORead();
}
void PGMConnection::_exitAIORead()
{
    if( _overlapped.hEvent )
    {
        CloseHandle( _overlapped.hEvent );
        _overlapped.hEvent = 0;
    }
}
#else
void PGMConnection::_initAIOAccept(){ /* NOP */ }
void PGMConnection::_exitAIOAccept(){ /* NOP */ }
void PGMConnection::_initAIORead(){ /* NOP */ }
void PGMConnection::_exitAIORead(){ /* NOP */ }
#endif

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
#ifdef _WIN32
void PGMConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );

    // Create new accept socket
    const DWORD flags = WSA_FLAG_OVERLAPPED;

    EQASSERT( _overlappedAcceptData );
    EQASSERT( _overlappedSocket == INVALID_SOCKET );
    _overlappedSocket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_RM, 0, 0,
                                   flags );

    if( _overlappedSocket == INVALID_SOCKET )
    {
        EQERROR << "Could not create accept socket: " << co::base::sysError
                << ", closing connection" << std::endl;
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
        EQERROR << "Could not start accept operation: " << co::base::sysError 
                << ", closing connection" << std::endl;
        close();
    }
}
    
ConnectionPtr PGMConnection::acceptSync()
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
               << ", closing connection" << std::endl;
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

    PGMConnection* newConnection = new PGMConnection;
    ConnectionPtr connection( newConnection ); // to keep ref-counting correct

    newConnection->_readFD  = _overlappedSocket;
    _overlappedSocket       = INVALID_SOCKET;

    newConnection->_setupReadSocket();
    newConnection->_writeFD = _writeFD;
    newConnection->_initAIORead();
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description = _description;

    EQINFO << "accepted connection " << (void*)newConnection << " from " 
           << inet_ntoa( remote->sin_addr ) << ":" << ntohs( remote->sin_port )
           << std::endl;
    return connection;
}

#else // !_WIN32

void PGMConnection::acceptNB(){ /* NOP */ }
 
ConnectionPtr PGMConnection::acceptSync()
{
    TBD
    if( _state != STATE_LISTENING )
        return 0;

    sockaddr_in newAddress;
    socklen_t   newAddressLen = sizeof( newAddress );

    SOCKET    fd;
    unsigned  nTries = 1000;
    do
        fd = ::accept( _readFD, (sockaddr*)&newAddress, &newAddressLen );
    while( fd == INVALID_PGM && errno == EINTR && --nTries );

    if( fd == INVALID_PGM )
    {
        EQWARN << "accept failed: " << co::base::sysError << std::endl;
        return 0;
    }

    _tunePGM( fd );

    PGMConnection* newConnection = new PGMConnection( _description->type);

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->bandwidth = _description->bandwidth;
    newConnection->_description->setHostname( inet_ntoa( newAddress.sin_addr ));
    newConnection->_description->TCPIP.port   = ntohs( newAddress.sin_port );

    EQVERB << "accepted connection from " << inet_ntoa(newAddress.sin_addr) 
           << ":" << ntohs( newAddress.sin_port ) << std::endl;

    return newConnection;
}

#endif // !_WIN32



#ifdef _WIN32
//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void PGMConnection::readNB( void* buffer, const uint64_t bytes )
{
    if( _state == STATE_CLOSED || _readFD == INVALID_SOCKET )
        return;

    WSABUF wsaBuffer = { EQ_MIN( bytes, 65535 ),
                         reinterpret_cast< char* >( buffer ) };
    DWORD  got   = 0;
    DWORD  flags = 0;

    ResetEvent( _overlapped.hEvent );
    if( WSARecv( _readFD, &wsaBuffer, 1, &got, &flags, &_overlapped, 0 )!= 0 &&
        GetLastError() != WSA_IO_PENDING )
    {
        EQWARN << "Could not start overlapped receive: " << co::base::sysError
               << ", closing connection" << std::endl;
        close();
    }
}

int64_t PGMConnection::readSync( void* buffer, const uint64_t bytes,
                                 const bool ignored )
{
    EQ_TS_THREAD( _recvThread );

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

        EQWARN << "Read complete failed: " << co::base::sysError 
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
#ifdef PRINT_STATS
    if( got == bytes )
        _printReadStatistics();
#endif

    return got;
}

int64_t PGMConnection::write( const void* buffer, const uint64_t bytes)
{
    if( _writeFD == INVALID_SOCKET ||
        ( _state != STATE_CONNECTED && _state != STATE_LISTENING ))
    {
        return -1;
    }

    DWORD  wrote;
    WSABUF wsaBuffer = 
        {   EQ_MIN( bytes, 65535 ),
            const_cast<char*>( static_cast< const char* >( buffer )) 
        };

    while( true )
    {
        if( WSASend( _writeFD, &wsaBuffer, 1, &wrote, 0, 0, 0 ) ==  0 ) // ok
        {
#ifdef PRINT_STATS
            if( wrote == bytes )
                _printSendStatistics();
#endif
            return wrote;
        }

        // error
        if( GetLastError( ) != WSAEWOULDBLOCK )
        {
            EQWARN << "Error during write: " << co::base::sysError << std::endl;
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
            EQWARN << "Error during select: " << co::base::sysError << std::endl;
            return -1;
        }
#endif
    }

    EQUNREACHABLE;
    return -1;
}
#endif // _WIN32

SOCKET PGMConnection::_initSocket( sockaddr_in address )
{
#ifdef _WIN32
    const DWORD flags = WSA_FLAG_OVERLAPPED;
    const SOCKET fd   = WSASocket( AF_INET, SOCK_STREAM, 
                                   IPPROTO_RM, 0, 0, flags );
#else
    const Socket fd = ::socket( AF_INET, TBD, TBD );
#endif

    if( fd == INVALID_SOCKET )
    {
        EQERROR << "Could not create socket: " << co::base::sysError << std::endl;
        return INVALID_SOCKET;
    }

    const bool bound = (::bind( fd, (sockaddr*)&address, sizeof(address)) == 0);

    if( !bound )
    {
        EQWARN << "Could not bind socket " << fd << ": " << co::base::sysError
               << " to " << inet_ntoa( address.sin_addr ) << ":" 
               << ntohs( address.sin_port ) << " AF " << (int)address.sin_family
               << std::endl;

        close();
        return INVALID_SOCKET;
    }


    const int on = 1;
    setsockopt( _readFD, SOL_SOCKET, SO_REUSEADDR, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    return fd;
}

bool PGMConnection::_setupReadSocket()
{
    _enableHighSpeed( _readFD );
    return( _setReadBufferSize( 65535 ) &&
            _setReadInterface( ));
}

bool PGMConnection::_setupSendSocket()
{
    _enableHighSpeed( _writeFD );
    return( _setFecParameters( _writeFD, 255, 4, true, 0 ) &&
            _setSendRate() &&
            _setSendBufferSize( 65535 ) &&
            _setSendInterface( ));
}


bool PGMConnection::_parseAddress( sockaddr_in& address )
{
    if( _description->port == 0 )
        _description->port = EQ_DEFAULT_PORT;
    if( _description->getHostname().empty( ))
        _description->setHostname( "239.255.42.42" );

    EQASSERT( _description->port != 0 );

    address.sin_family      = AF_INET;
    address.sin_port        = htons( _description->port );
    memset( &(address.sin_zero), 0, 8 ); // zero the rest

    if( !_parseHostname( _description->getHostname(), address.sin_addr.s_addr ))
        return false;

    EQVERB << "Address " << inet_ntoa( address.sin_addr ) << ":" 
           << ntohs( address.sin_port ) << std::endl;
    return true;
}

bool PGMConnection::_parseHostname( const std::string& hostname,
                                    unsigned long& address )
{
    address = htonl( INADDR_ANY );
    if( hostname.empty( ))
        return true;

    hostent *hptr = gethostbyname( hostname.c_str( ));
    if( !hptr )
    {
        EQWARN << "Can't resolve host " << hostname << std::endl;
        return false;
    }

    memcpy( &address, hptr->h_addr, hptr->h_length );
    return true;
}

bool PGMConnection::_setSendBufferSize( const int newSize )
{
    EQASSERT(newSize >= 0);
    
    if ( ::setsockopt( _writeFD, SOL_SOCKET, SO_SNDBUF, 
                       ( char* )&newSize, sizeof( int )) == SOCKET_ERROR ) 
    {
        EQWARN << "can't SetSendBufferSize, error: " 
               <<  co::base::sysError << std::endl;
        return false;
    }

    return true;
}

bool PGMConnection::_setSendRate()
{
    if( _description->bandwidth == 0 )
        return true;

    RM_SEND_WINDOW  sendWindow;
    sendWindow.RateKbitsPerSec   = _description->bandwidth * 8;
    sendWindow.WindowSizeInBytes = 0;
    sendWindow.WindowSizeInMSecs = 10000; // Set window size to 10s

    EQINFO << "Setting PGM send rate to " << sendWindow.RateKbitsPerSec
           << " kBit/s" << std::endl;

    if( ::setsockopt( _writeFD, IPPROTO_RM, RM_RATE_WINDOW_SIZE, 
                      (char*)&sendWindow, 
                      sizeof(RM_SEND_WINDOW)) == SOCKET_ERROR ) 
    {
        EQWARN << "can't set send rate, error: " <<  co::base::sysError
               << std::endl;
        return false ;
    }
    return true;
}

bool PGMConnection::_setSendInterface()
{
    const std::string& iName = _description->getInterface();
    if( iName.empty( ))
        return true;

    unsigned long interface;
    if( !_parseHostname( iName, interface ) ||
        ::setsockopt( _writeFD, IPPROTO_RM, RM_SET_SEND_IF,
                     (char*)&interface, sizeof(uint32_t)) == SOCKET_ERROR )
    {
        EQWARN << "can't set send interface " <<  co::base::sysError << std::endl;
        return false;
    }
    return true;
}

/* Function: SetFecParamters
   Description:
      This routine sets the requested FEC parameters on a sender socket.
      A client does not have to do anything special when the sender enables or 
      disables FEC
    blockSize 
          Maximum number of packets that can be sent for any group, 
          including original data and parity packets. 
          Maximum and default value is 255.
    groupSize 
          Number of packets to be treated as one group for the purpose of 
          computing parity packets. Group size must be a power of two. 
          In lossy networks, keep the group size relatively small.
    ondemand 
          Specifies whether the sender is enabled for sending parity repair 
          packets. When TRUE, receivers should only request parity repair 
          packets.
    proactive 
          Number of packets to send proactively with each group. 
          Use this option when the network is dispersed, 
          and upstream NAK requests are expensive.
*/
bool PGMConnection::_setFecParameters( const SOCKET fd, 
                                       const int blocksize, 
                                       const int groupsize, 
                                       const int ondemand, 
                                       const int proactive )
{
    RM_FEC_INFO fec;
    memset(&fec, 0, sizeof( fec ));
    fec.FECBlockSize              = blocksize;
    fec.FECProActivePackets       = proactive;
    fec.FECGroupSize              = groupsize;
    fec.fFECOnDemandParityEnabled = ondemand;
    if ( ::setsockopt( fd, IPPROTO_RM, RM_USE_FEC, 
                      (char *)&fec, sizeof( RM_FEC_INFO )))
    {
        EQWARN << "can't set error correction parameters " 
               << co::base::sysError << std::endl;
        return false;
    }
    return true;
}

bool PGMConnection::_setReadBufferSize( int newSize )
{
    if ( ::setsockopt( _readFD, SOL_SOCKET, SO_RCVBUF,
                      (char*)&newSize, sizeof(int)) == SOCKET_ERROR ) 
    {
        EQWARN << "can't set receive buffer size, error: " 
               <<  co::base::sysError << std::endl;
        return false;
    }
    return true;
}

bool PGMConnection::_setReadInterface()
{
    const std::string& iName = _description->getInterface();
    if( iName.empty( ))
        return true;

    unsigned long interface;
    if( !_parseHostname( iName, interface ) ||
        ::setsockopt( _readFD, IPPROTO_RM, RM_ADD_RECEIVE_IF,
                     (char*)&interface, sizeof(uint32_t)) == SOCKET_ERROR )
    {
        EQWARN << "can't add recv interface " <<  co::base::sysError 
               << std::endl;
        return false;
    }
    return true;
}

bool PGMConnection::_enableHighSpeed(  SOCKET fd )
{
    ULONG HighSpeedLanEnabled = 1;

    if ( ::setsockopt( fd, IPPROTO_RM, RM_HIGH_SPEED_INTRANET_OPT , 
                 (char*)&HighSpeedLanEnabled, sizeof( ULONG )) == SOCKET_ERROR )
    {
        EQWARN << "can't EnableHighSpeedLanOption, error: " 
               <<  co::base::sysError << std::endl;
        return false;
    }

    return true;
}

void PGMConnection::_printReadStatistics()
{
    RM_RECEIVER_STATS stats;
    socklen_t len = sizeof( stats );

    if( getsockopt( _readFD, IPPROTO_RM, RM_RECEIVER_STATISTICS, 
                   (char*)&stats, &len ) != 0 )
    {
        return;
    }

    EQWARN << stats.NumDuplicateDataPackets << " dups, " 
           << stats.NumRDataPacketsReceived << " retransmits of "
           << stats.NumODataPacketsReceived << " packets, "
           << stats.NumDataPacketsBuffered << " not yet read, "
           << stats.RateKBitsPerSecLast / 8192 << "MB/s" << std::endl;
}

void PGMConnection::_printSendStatistics()
{
    RM_SENDER_STATS stats;
    socklen_t len = sizeof( stats );

    if( getsockopt( _writeFD, IPPROTO_RM, RM_SENDER_STATISTICS, 
        (char*)&stats, &len ) != 0 )
    {
        return;
    }

    EQWARN << stats.NaksReceived << " NAKs, " 
           << stats.NaksReceivedTooLate << " late NAKs "
           << stats.RepairPacketsSent << " repair packets, "
           << stats.TotalODataPacketsSent << " data packets, "
           << stats.TotalODataPacketsSent / 8192 << "MB/s" << std::endl;
}

}

#endif // EQ_PGM
