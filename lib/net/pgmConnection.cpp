
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

#include "PGMConnection.h"

#ifdef EQ_PGM

#include "connectionDescription.h"

#include <eq/base/base.h>
#include <eq/base/log.h>

#include <errno.h>
#include <sstream>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
#  include <Mswsock.h>
#  include <wsrm.h>
#else
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <sys/PGM.h>
#endif

namespace eq
{
namespace net
{
PGMConnection::PGMConnection()
#ifdef WIN32
        : _overlappedAcceptData( 0 )
        , _overlappedSocket( INVALID_SOCKET )
#endif
{
#ifdef WIN32
    memset( &_overlapped, 0, sizeof( _overlapped ));
#endif

    _description =  new ConnectionDescription;
    _description->type = CONNECTIONTYPE_MCIP_PGM;
    _description->bandwidth = 102400;

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
    EQASSERTINFO( false,
                  base::disableFlush <<
                  "Multicast connections are not connected." << std::endl <<
                  "Each member listens on the group address." << std::endl <<
                  "The send operation triggers the accept on each member," <<
                  std::endl << 
                  "which creates a connected connection for the sender." <<
                  std::endl << base::enableFlush );
    return false;
}

void PGMConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    if( isListening( ))
        _exitAIOAccept();
    else
        _exitAIORead();

    _state = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    if( _readFD > 0 )
    {
#ifdef WIN32
        const bool closed = ( ::closesocket( _readFD ) == 0 );
#else
        const bool closed = ( ::close( _readFD ) == 0 );
#endif
        
        if( !closed )
            EQWARN << "Could not close read socket: " << base::sysError
                   << std::endl;
    }

    if( _writeFD > 0 && isListening( ))
    {
#ifdef WIN32
        const bool closed = ( ::closesocket( _writeFD ) == 0 );
#else
        const bool closed = ( ::close( _writeFD ) == 0 );
#endif

        if( !closed )
            EQWARN << "Could not close write socket: " << base::sysError
                   << std::endl;
    }

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
#ifdef WIN32
void PGMConnection::_initAIORead()
{
    _overlapped.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _overlapped.hEvent );

    if( !_overlapped.hEvent )
        EQERROR << "Can't create event for AIO notification: " 
                << base::sysError << std::endl;
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
#ifdef WIN32
void PGMConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );

    // Create new accept socket
    const DWORD flags = WSA_FLAG_OVERLAPPED;

    EQASSERT( _overlappedAcceptData );
    EQASSERT( _overlappedSocket == INVALID_SOCKET );
    _overlappedSocket = WSASocket( AF_INET, SOCK_RDM, IPPROTO_RM, 0,0, flags );

    if( _overlappedSocket == INVALID_SOCKET )
    {
        EQERROR << "Could not create accept socket: " << base::sysError
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
        EQERROR << "Could not start accept operation: " << base::sysError 
                << ", closing connection" << std::endl;
        close();
    }
}
    
ConnectionPtr PGMConnection::acceptSync()
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
        EQWARN << "Accept completion failed: " << base::sysError 
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
    _tuneSocket( _overlappedSocket );

    PGMConnection* newConnection = new PGMConnection;
    ConnectionPtr connection( newConnection ); // to keep ref-counting correct

    newConnection->_readFD  = _overlappedSocket;
    newConnection->_writeFD = _writeFD;
    newConnection->_initAIORead();
    _overlappedSocket       = INVALID_SOCKET;

    newConnection->_state                   = STATE_CONNECTED;
    newConnection->_description->bandwidth  = _description->bandwidth;
    newConnection->_description->MCIP.port  = ntohs( remote->sin_port );
    newConnection->_description->setHostname( inet_ntoa( remote->sin_addr ));

    EQINFO << "accepted connection from " << inet_ntoa( remote->sin_addr ) 
           << ":" << ntohs( remote->sin_port ) << std::endl;
    return connection;
}

#else // !WIN32

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
        EQWARN << "accept failed: " << base::sysError << std::endl;
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

#endif // !WIN32



#ifdef WIN32
//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void PGMConnection::readNB( void* buffer, const uint64_t bytes )
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
        EQWARN << "Could not start overlapped receive: " << base::sysError
               << ", closing connection" << std::endl;
        close();
    }
}

int64_t PGMConnection::readSync( void* buffer, const uint64_t bytes )
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

        EQWARN << "Read complete failed: " << base::sysError 
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

int64_t PGMConnection::write( const void* buffer, const uint64_t bytes)
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
            EQWARN << "Error during write: " << base::sysError << std::endl;
            return -1;
        }

        // Buffer full - try again
#if 1
        // Wait for writable PGM
        fd_set set;
        FD_ZERO( &set );
        FD_SET( _writeFD, &set );

        const int result = select( _writeFD+1, 0, &set, 0, 0 );
        if( result <= 0 )
        {
            EQWARN << "Error during select: " << base::sysError << std::endl;
            return -1;
        }
#endif
    }

    EQUNREACHABLE;
    return -1;
}
#endif // WIN32

SOCKET PGMConnection::_initSocket( sockaddr_in address )
{
#ifdef WIN32
    const DWORD flags = WSA_FLAG_OVERLAPPED;
    const SOCKET fd = WSASocket( AF_INET, SOCK_RDM, IPPROTO_RM, 0,0, flags );
#else
    Socket fd = ::socket( AF_INET, TBD, TBD );
#endif

    if( fd == INVALID_SOCKET )
    {
        EQERROR << "Could not create socket: " << base::sysError << std::endl;
        return INVALID_SOCKET;
    }

    _tuneSocket( fd );

    const bool bound = (::bind( fd, (sockaddr*)&address, sizeof(address)) == 0);

    if( !bound )
    {
        EQWARN << "Could not bind socket " << _readFD << ": " 
               << base::sysError << " to "
               << inet_ntoa( address.sin_addr )
               << ":" << ntohs( address.sin_port ) << " AF " 
               << (int)address.sin_family << std::endl;

        close();
        return false;
    }

    return fd;
}

void PGMConnection::_tuneSocket( const SOCKET fd )
{
#if 0 // TBD
    const int on         = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    setsockopt( fd, SOL_PGM, SO_REUSEADDR, 
                reinterpret_cast<const char*>( &on ), sizeof( on ));
    
#ifdef WIN32
    const int size = 128768;
    setsockopt( fd, SOL_PGM, SO_RCVBUF, 
                reinterpret_cast<const char*>( &size ), sizeof( size ));
    setsockopt( fd, SOL_PGM, SO_SNDBUF,
                reinterpret_cast<const char*>( &size ), sizeof( size ));
#endif
#endif
}

bool PGMConnection::_parseAddress( sockaddr_in& address )
{
    if( _description->MCIP.port == 0 )
        _description->MCIP.port = EQ_DEFAULT_PORT;
    EQASSERT( _description->MCIP.port != 0 );

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = htonl( INADDR_ANY );
    address.sin_port        = htons( _description->MCIP.port );
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

    EQINFO << "Address " << inet_ntoa( address.sin_addr )
           << ":" << ntohs( address.sin_port ) << std::endl;
    return true;
}
//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool PGMConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_MCIP_PGM );

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
    if( _readFD == INVALID_SOCKET )
        return false;
    
    const bool bound = (::bind( _readFD, (sockaddr *)&address, size ) == 0);

    if( !bound )
    {
        EQWARN << "Could not bind socket " << _readFD << ": " 
               << base::sysError << " to "
               << inet_ntoa( address.sin_addr )
               << ":" << ntohs( address.sin_port ) << " AF " 
               << (int)address.sin_family << std::endl;

        close();
        return false;
    }

    const bool listening = (::listen( _readFD, 10 ) == 0);
        
    if( !listening )
    {
        EQWARN << "Could not listen on PGM: "<< base::sysError << std::endl;
        close();
        return false;
    }


    // get listening socket parameters
    socklen_t used = size;
    getsockname( _readFD, (struct sockaddr *)&address, &used ); 

    _description->MCIP.port = ntohs( address.sin_port );

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
    if( _writeFD == INVALID_SOCKET )
    {
        close();
        return false;
    }

    _parseAddress( address );
    
#ifdef WIN32
    const bool connected = WSAConnect( _writeFD, (sockaddr*)&address, 
                                       size, 0, 0, 0, 0 ) == 0;
#else
    const bool connected = (::connect( _readFD, (sockaddr*)&address, 
                                       size ) == 0);
#endif

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
               << _description->MCIP.port << "': " << base::sysError
               << std::endl;
        close();
        return false;
    }

    _state = STATE_LISTENING;
    _fireStateChanged();

    EQINFO << "Listening on " << _description->getHostname() << "["
           << inet_ntoa( address.sin_addr ) << "]:" 
           << _description->TCPIP.port << " (" << _description->toString()
           << ")" << std::endl;
    
    return true;
}

}
}

#endif // EQ_PGM
