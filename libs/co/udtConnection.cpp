// -*- mode: c++ -*-
/* Copyright (c) 2012, Computer Integration & Programming Solutions, Corp. and
 *                     United States Naval Research Laboratory
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
#include "udtConnection.h"

#include "connectionDescription.h"
#include "global.h"
#include "base/thread.h"
#include "base/scopedMutex.h"
#include "base/clock.h"

#ifdef _WIN32
#    include <winsock2.h>
#    include <ws2tcpip.h>
#else
#    include <netinet/in.h>
#    include <arpa/inet.h>
#    include <netdb.h>
#    include <sys/eventfd.h>
#endif

#include <set>
#include <sstream>

#include <ccc.h>

#define UDT_POLL_INT 1000 // idle connection status/quit poll interval (ms)

namespace co
{
namespace
{
template< class T > inline std::string to_string( const T &t )
{
    std::stringstream ss; ss << t; return ss.str( );
}

static bool _parseAddress( ConnectionDescriptionPtr description,
    struct sockaddr &address, const bool passive )
{
    bool ok = false;
    const char *node = NULL, *service = NULL;
    struct addrinfo hints, *res = NULL;

    ::memset( (void *)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

    const std::string &hostname = description->getHostname( );
    if( !hostname.empty( ))
        node = hostname.c_str( );
    else if( passive )
        hints.ai_flags |= AI_PASSIVE;
    const std::string port = to_string<uint16_t>( description->port );
    if( 0u != description->port )
        service = port.c_str( );

    const int errcode = ::getaddrinfo( node, service, &hints, &res );
    if( 0 != errcode )
    {
        EQERROR << "getaddrinfo : " << ::gai_strerror( errcode ) << std::endl;
        goto out;
    }

    if( NULL != res->ai_next )
        EQWARN << "Multiple getaddrinfo results, using first." << std::endl;

    ::memcpy( (void *)&address, (const void *)res->ai_addr, res->ai_addrlen );
    ok = true;

out:
    if( NULL != res )
        ::freeaddrinfo( res );

    return ok;
}

#ifdef EQ_GCC_4_5_OR_LATER
#  pragma GCC diagnostic ignored "-Wunused-result"
#endif

static void notify( co::Connection::Notifier n )
{
#ifdef _WIN32
    EQASSERT( NULL != n );

    SetEvent( n );
#else
    static const uint64_t ONE = 1ULL;

    EQASSERT( 0 <= n );

#   ifdef EQ_RELEASE_ASSERT
    EQCHECK( ::write( n, (const void *)&ONE, sizeof(ONE) ) == sizeof(ONE) );
#   else
    ::write( n, (const void *)&ONE, sizeof(ONE) );
#   endif
#endif
}

static void acknowledge( co::Connection::Notifier n )
{
#ifdef _WIN32
    EQASSERT( NULL != n );

    ResetEvent( n );
#else
    EQASSERT( 0 <= n );

    uint64_t dummy;

#   ifdef EQ_RELEASE_ASSERT
    EQCHECK( ::read( n, (void *)&dummy, sizeof(dummy) ) == sizeof(dummy) );
#   else
    ::read( n, (void *)&dummy, sizeof(dummy) );
#   endif
#endif
}

class CTCP: public CCC
{
public:
    virtual void init( )
    {
        EQINFO << "UDT CCC: CTCP" << std::endl;

        m_bSlowStart = true;
        m_issthresh = 83333;

        m_dPktSndPeriod = 0.0;
        m_dCWndSize = 2.0;

        setACKInterval( 2 );
        setRTO( 1000000 );
    }

    virtual void onACK( const int32_t &ack )
    {
        if( ack == m_iLastACK )
        {
            if( 3 == ++ m_iDupACKCount )
                DupACKAction( );
            else if( m_iDupACKCount > 3 )
                m_dCWndSize += 1.0;
            else
                ACKAction( );
        }
        else
        {
            if( m_iDupACKCount >= 3 )
                m_dCWndSize = m_issthresh;

            m_iLastACK = ack;
            m_iDupACKCount = 1;

            ACKAction( );
        }
    }

    virtual void onTimeout( )
    {
        m_issthresh = getPerfInfo( )->pktFlightSize / 2;
        if( m_issthresh < 2 )
            m_issthresh = 2;

        m_bSlowStart = true;
        m_dCWndSize = 2.0;
    }
protected:
    virtual void ACKAction( )
    {
        if( m_bSlowStart )
        {
            m_dCWndSize += 1.0;

            if( m_dCWndSize >= m_issthresh )
                m_bSlowStart = false;
        }
        else
            m_dCWndSize += 1.0/m_dCWndSize;
    }

    virtual void DupACKAction( )
    {
        m_bSlowStart = false;

        m_issthresh = getPerfInfo( )->pktFlightSize / 2;
        if( m_issthresh < 2 )
            m_issthresh = 2;

        m_dCWndSize = m_issthresh + 3;
    }
protected:
    int m_issthresh;
    bool m_bSlowStart;

    int m_iDupACKCount;
    int m_iLastACK;
}; // CTCP

class CUDPBlast: public CCC
{
public:
    virtual void init( )
    {
        EQINFO << "UDT CCC: CUDPBlast" << std::endl;

        m_dPktSndPeriod = 1000000;
        m_dCWndSize = 83333.0;
    }
public:
    void setRate( double mbps )
    {
        m_dPktSndPeriod = ( m_iMSS * 8.0 ) / mbps;
    }
}; // CUDPBlast
} // namespace

//#define CONGESTION_CONTROL_CLASS CTCP
//#define CONGESTION_CONTROL_CLASS CUDPBlast

#define UDTLASTERROR( msg ) \
    ( msg ) << " : " << \
        UDT::getlasterror( ).getErrorMessage( ) << " (" << \
        UDT::getlasterror( ).getErrorCode( ) << ")"

/**
 * UDTSOCKET is an opaque UDT "handle", and is not select'able by the
 * application.  UDT has its own internal epoll mechanism by which one can
 * poll for events, we create a thread to do this and pass the events back to
 * Collage via an eventfd(2).
 *
 * For more info, see thread "threading model and real socket integration" on
 * the UDT open discussion list.
 */
class UDTConnection::UDTConnectionThread : public base::Thread
{
public:
    UDTConnectionThread( UDTConnection *connection )
        : _running( false )
        , _connection( connection )
        , _eid( UDT::ERROR )
    {
        EQASSERT( _connection );
    }
    virtual ~UDTConnectionThread( );

    virtual bool init( );
    virtual void run( );

    void requestStop( );
private:
    bool _running;
    UDTConnection *_connection;

    int _eid; // UDT epoll id
}; // UDTConnectionThread

// UDTConnection

UDTConnection::UDTConnection( )
    : _udt( UDT::INVALID_SOCK )
#ifdef _WIN32
    , _notifier( NULL )
#else
    , _notifier( -1 )
#endif
    , _poller( NULL )
{
#ifdef EQ_RELEASE_ASSERT
    EQCHECK( UDT::ERROR != UDT::startup( ));
#else
    UDT::startup( );
#endif

    _description->type = CONNECTIONTYPE_UDT;
    _description->bandwidth = 102400000; // 1Gbps

    EQVERB << "New UDTConnection @" << (void *)this << std::endl;
}

// caller: application
bool UDTConnection::connect( )
{
    struct sockaddr address;
    CCC *cc = NULL;
    int len;

    EQASSERT( CONNECTIONTYPE_UDT == _description->type );
    if( STATE_CLOSED != _state )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged( );

    if( !_parseAddress( _description, address, false ))
        goto err;

    EQASSERT( UDT::INVALID_SOCK == _udt );
    _udt = UDT::socket( AF_INET, SOCK_STREAM, 0 );
    if( UDT::INVALID_SOCK == _udt )
    {
        EQERROR << UDTLASTERROR( "UDT::socket" ) << std::endl;
        goto err;
    }

    if( !tuneSocket( ))
        goto err;

    if( UDT::ERROR == UDT::connect( _udt, &address, sizeof( address )))
    {
        EQERROR << UDTLASTERROR( "UDT::connect" ) << std::endl;
        goto err;
    }

    // Do this after connect, otherwise connect itself becomes non-blocking
    static const bool OFF = false;
    if( !setSockOpt( UDT_RCVSYN,
        static_cast<const void *>( &OFF ), sizeof(OFF) ))
        goto err;

    if( UDT::ERROR != UDT::getsockopt( _udt, 0, UDT_CC, &cc, &len ))
    {
        if( NULL != cc )
        {
            CUDPBlast *ccblast = dynamic_cast<CUDPBlast *>( cc );
            if( NULL != ccblast )
                ccblast->setRate( _description->bandwidth / 1000. );
        }
    }

    if( initialize( ))
    {
        _state = STATE_CONNECTED;
        _fireStateChanged( );
        return true;
    }
err:
    return false;
}

// caller: application
bool UDTConnection::listen( )
{
    struct sockaddr address;

    EQASSERT( CONNECTIONTYPE_UDT == _description->type );
    if( STATE_CLOSED != _state )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged( );

    if( !_parseAddress( _description, address, true ))
        goto err;

    EQASSERT( UDT::INVALID_SOCK == _udt );
    _udt = UDT::socket( AF_INET, SOCK_STREAM, 0 );
    if( UDT::INVALID_SOCK == _udt )
    {
        EQERROR << UDTLASTERROR( "UDT::socket" ) << std::endl;
        goto err;
    }

    if( !tuneSocket( ))
        goto err;

    if( UDT::ERROR == UDT::bind( _udt, &address, sizeof( address )))
    {
        EQERROR << UDTLASTERROR( "UDT::bind" ) << std::endl;
        goto err;
    }

    if( UDT::ERROR == UDT::listen( _udt, SOMAXCONN ))
    {
        EQERROR << UDTLASTERROR( "UDT::listen" ) << std::endl;
        goto err;
    }

    if( initialize( ))
    {
        _state = STATE_LISTENING;
        _fireStateChanged( );
        return true;
    }
err:
    return false;
}

// caller: application
void UDTConnection::close( )
{
    UDTConnectionThread *poller = NULL;

    {
        base::ScopedMutex<> mutex( _app_mutex );

        if( STATE_CLOSED != _state )
        {
            _state = STATE_CLOSING;
            _fireStateChanged( );

            // Let the event thread proceed if its blocked
            _app_block.set( true );

#ifdef _WIN32
            if(( NULL != _notifier ) && ( 0 == CloseHandle( _notifier )))
                EQWARN << "CloseHandle : " << base::sysError << std::endl;
            _notifier = NULL;
#else
            if(( 0 <= _notifier ) && ( 0 != ::close( _notifier )))
                EQWARN << "close : " << base::sysError << std::endl;
            _notifier = -1;
#endif

            poller = _poller;
            _poller = NULL;

            _state = STATE_CLOSED;
            _fireStateChanged( );
        }
    }

    // Do this outside of the above mutex since otherwise
    // we could deadlock with the event thread
    if( NULL != poller )
    {
        poller->requestStop( );
        poller->join( );
        delete poller;
        poller = NULL;

        if( UDT::ERROR == UDT::close( _udt ))
            EQWARN << UDTLASTERROR( "UDT::close" ) << std::endl;
        _udt = UDT::INVALID_SOCK;
    }
}

void UDTConnection::acceptNB( ) { /* NOP */ }

// caller: application
ConnectionPtr UDTConnection::acceptSync( )
{
    struct sockaddr address;
    int addrlen;
    UDTConnection *newConnection = NULL;

    UDTSOCKET newSocket = UDT::accept( _udt, &address, &addrlen );

    if( UDT::INVALID_SOCK == newSocket )
    {
        EQERROR << UDTLASTERROR( "UDT::accept" ) << std::endl;
        close( );
        goto err;
    }

    acknowledge( _notifier );

    newConnection = new UDTConnection( );
    newConnection->_udt = newSocket;

    // Do this after accept, otherwise accept itself becomes non-blocking
    static const bool OFF = false;
    if( !newConnection->setSockOpt( UDT_RCVSYN,
        static_cast<const void *>( &OFF ), sizeof(OFF) ))
        goto err;

    newConnection->_description->setHostname(
        ::inet_ntoa( ((struct sockaddr_in *)&address)->sin_addr ));
    newConnection->_description->port =
        ntohs( ((struct sockaddr_in *)&address)->sin_port );
    newConnection->_description->bandwidth = _description->bandwidth;
    if( newConnection->initialize( ))
    {
        newConnection->_state = STATE_CONNECTED;
        newConnection->_fireStateChanged( );
        goto out;
    }

err:
    if( NULL != newConnection )
    {
        delete newConnection;
        newConnection = NULL;
    }

out:
    // Let the event thread continue polling
    base::ScopedMutex<> mutex( _app_mutex );

    _app_block.set( true );

    return newConnection;
}

void UDTConnection::readNB( void* buffer, const uint64_t bytes ) { /* NOP */ }

// caller: application
int64_t UDTConnection::readSync( void* buffer, const uint64_t bytes,
    const bool )
{
    const int rcvd = UDT::recv( _udt, static_cast<char *>( buffer ),
        static_cast<int>( bytes ), 0 );

    if( UDT::ERROR == rcvd )
    {
        if( UDT::ERRORINFO::EASYNCRCV == UDT::getlasterror( ).getErrorCode( ))
            return 0LL;

        EQWARN << UDTLASTERROR( "UDT::recv" ) << std::endl;
        close( );
        return -1LL;
    }

    acknowledge( _notifier );

    int avail, len;
    if( UDT::ERROR == UDT::getsockopt( _udt, 0, UDT_RCVDATA, &avail, &len ))
        EQWARN << UDTLASTERROR( "UDT::getsockopt" ) << std::endl;

    if( avail > 0 )
    {
        // Keep the application going
        notify( _notifier );
    }
    else
    {
        // Let the event thread continue polling
        base::ScopedMutex<> mutex( _app_mutex );

        _app_block.set( true );
    }

    return rcvd;
}

// caller: application
int64_t UDTConnection::write( const void* buffer, const uint64_t bytes )
{
    const int sent = UDT::send( _udt, static_cast<const char *>( buffer ),
        static_cast<int>( bytes ), 0 );

    if( UDT::ERROR == sent )
    {
        EQWARN << UDTLASTERROR( "UDT::send" ) << std::endl;
        close( );
        return -1LL;
    }

    return sent;
}

// caller: application
UDTConnection::~UDTConnection( )
{
    close( );

    UDT::cleanup( );
}

// caller: application
bool UDTConnection::initialize( )
{
#ifdef _WIN32
    _notifier = CreateEvent( 0, TRUE, FALSE, 0 );
    if( NULL == _notifier )
    {
        EQERROR << "CreateEvent : " << base::sysError << std::endl;
        goto err;
    }
#else
    _notifier = ::eventfd( 0, 0 );
    if( 0 > _notifier )
    {
        EQERROR << "eventfd : " << base::sysError << std::endl;
        goto err;
    }
#endif

    _poller = new UDTConnectionThread( this );
    return _poller->start( );

err:
    return false;
}

// caller: UDTConnectionThread
void UDTConnection::wake( )
{
    bool notifyAndWait = false;

    {
        base::ScopedMutex<> mutex( _app_mutex );

        // Only block if we're not shutting down
        if(( STATE_LISTENING == _state ) || ( STATE_CONNECTED == _state ))
        {
            notifyAndWait = true;

            _app_block.set( false );
        }
    }

    if( notifyAndWait )
    {
        // Wake the application
        notify( _notifier );

        // Wait for the application to handle event
        _app_block.waitNE( false );
    }
}

// caller: application
bool UDTConnection::tuneSocket( )
{
    const int timeout = (int)Global::getTimeout( );
/* TODO: Use our own Globals?
    const int udt_snd_buf = 10240000;
    const int udt_rcv_buf = 10240000;
    const int udp_snd_buf =
        Global::getIAttribute( Global::IATTR_UDP_BUFFER_SIZE );
    const int udp_rcv_buf =
        Global::getIAttribute( Global::IATTR_UDP_BUFFER_SIZE );
*/
    const int mss = 65520;//Global::getIAttribute( Global::IATTR_UDP_MTU );

    if( !setSockOpt( UDT_SNDTIMEO,
        static_cast<const void *>( &timeout ), sizeof(timeout) ))
        goto err;
    if( !setSockOpt( UDT_RCVTIMEO,
        static_cast<const void *>( &timeout ), sizeof(timeout) ))
        goto err;
/*
    if( !setSockOpt( UDT_SNDBUF,
        static_cast<const void *>( &udt_snd_buf ), sizeof(udt_snd_buf) ))
        goto err;
    if( !setSockOpt( UDT_RCVBUF,
        static_cast<const void *>( &udt_rcv_buf ), sizeof(udt_rcv_buf) ))
        goto err;
    if( !setSockOpt( UDP_SNDBUF,
        static_cast<const void *>( &udp_snd_buf ), sizeof(udp_snd_buf) ))
        goto err;
    if( !setSockOpt( UDP_RCVBUF,
        static_cast<const void *>( &udp_rcv_buf ), sizeof(udp_rcv_buf) ))
        goto err;
*/
    if( !setSockOpt( UDT_MSS, static_cast<const void *>( &mss ), sizeof(mss) ))
        goto err;

#ifdef CONGESTION_CONTROL_CLASS
    if( !setSockOpt( UDT_CC,
        static_cast<const void *>( new CCCFactory<CONGESTION_CONTROL_CLASS> ),
            sizeof(CCCFactory<CONGESTION_CONTROL_CLASS>) ))
        goto err;
#endif

    return true;
err:
    return false;
}

// caller: application
bool UDTConnection::setSockOpt( UDT::SOCKOPT optname, const void *optval,
    int optlen )
{
    if( UDT::ERROR == UDT::setsockopt( _udt, 0, optname, optval, optlen ))
    {
        EQERROR << UDTLASTERROR( "UDT::setsockopt" ) << std::endl;
        return false;
    }

    return true;
}

// UDTConnection::UDTConnectionThread

// caller: application
UDTConnection::UDTConnectionThread::~UDTConnectionThread( )
{
    EQASSERT( !_running );

    if( UDT::ERROR != _eid )
    {
        EQASSERT( UDT::INVALID_SOCK != _connection->_udt );
        if( UDT::ERROR == UDT::epoll_remove_usock( _eid, _connection->_udt ))
            EQWARN << UDTLASTERROR( "UDT::epoll_remove_usock" ) << std::endl;

        if( UDT::ERROR == UDT::epoll_release( _eid ))
            EQWARN << UDTLASTERROR( "UDT::epoll_release" ) << std::endl;
    }
    _eid = UDT::ERROR;
}

// caller: application
bool UDTConnection::UDTConnectionThread::init( )
{
    EQASSERT( !_running );

    // Create the UDT epoll identifier
    _eid = UDT::epoll_create( );
    if( UDT::ERROR == _eid )
    {
        EQERROR << UDTLASTERROR( "UDT::epoll_create" ) << std::endl;
        goto out;
    }

    // Add the connection's UDT socket to the UDT udtsock poll
    EQASSERT( UDT::INVALID_SOCK != _connection->_udt );
    if( UDT::ERROR == UDT::epoll_add_usock( _eid, _connection->_udt ))
    {
        EQERROR << UDTLASTERROR( "UDT::epoll_add_usock" ) << std::endl;
        goto out;
    }

    _running = true;

out:
    return _running;
}

// caller: UDTConnectionThread
void UDTConnection::UDTConnectionThread::run( )
{
    while( _running )
    {
        std::set<UDTSOCKET> udtfds;

        if( UDT::ERROR ==
            UDT::epoll_wait( _eid, &udtfds, NULL, UDT_POLL_INT, NULL, NULL ))
        {
            EQERROR << UDTLASTERROR( "UDT::epoll_wait" ) << std::endl;

            _connection->wake( );
            break; // TODO: ???
        }

        // UDT indicating ready-to-read
        if( !udtfds.empty( ))
        {
            EQASSERT( 1 == udtfds.size( ));
            EQASSERT( *(udtfds.begin( )) == _connection->_udt );

            _connection->wake( );
        }
        // Otherwise, poll for status changes
        else if( _running )
        {
            Connection::State state = _connection->_state;
            UDTSTATUS status = UDT::getsockstate( _connection->_udt );

            // readSync/acceptSync will hopefully return errors
            if(( STATE_LISTENING == state ) && ( LISTENING != status ))
                _connection->wake( );
            if(( STATE_CONNECTED == state ) && ( CONNECTED != status ))
                _connection->wake( );
        }

        // TODO: UDT::perfmon?
    }
}

// caller: application
void UDTConnection::UDTConnectionThread::requestStop( )
{
    // Request thread stop
    _running = false;
}
} // namespace co
