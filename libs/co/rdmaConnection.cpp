// -*- mode: c++ -*-
/* Copyright (c) 2012, Computer Integration & Programming Solutions, Corp. and
 *                     United States Naval Research Laboratory
 *               2012, Stefan Eilemann <eile@eyescale.ch>
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
#include "rdmaConnection.h"

#include "connectionType.h"
#include "connectionDescription.h"
#include "global.h"

#include <co/base/scopedMutex.h>
#include <co/base/clock.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits>
#include <sstream>
#include <stddef.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>

#include <rdma/rdma_verbs.h>

#define IPV6_DEFAULT 0

#define RDMA_PROTOCOL_MAGIC     0xC0
#define RDMA_PROTOCOL_VERSION   0x02

namespace co
{
namespace { static const uint64_t ONE = 1ULL; }

/**
 * Message types
 */
enum OpCode
{
   SETUP = 1 << 0,
   FC    = 1 << 1,
};

/**
 * Initial setup message used to exchange sink MR parameters
 */
struct RDMASetupPayload
{
    uint64_t rbase;
    uint64_t rlen;
    uint64_t rkey;
};

/**
 * "ACK" messages sent after read, tells source about read progress
 */
struct RDMAFCPayload
{
    uint32_t ringTail;
};

/**
 * Payload wrapper
 */
struct RDMAMessage
{
    enum OpCode opcode;
    uint8_t length;
    union
    {
        struct RDMASetupPayload setup;
        struct RDMAFCPayload fc;
    } payload;
};

/**
 * "IMM" data sent with RDMA write, tells sink about send progress
 */
typedef uint32_t RDMAFCImm;

/**
 * An RDMA connection implementation.
 *
 * The protocol is simple, e.g.:
 *
 *      initiator                        target
 * -----------------------------------------------------
 *                                  resolve/bind/listen
 * resolve/prepost/connect
 *                                    prepost/accept
 *     send setup         <------->    send setup
 *   wait for setup                   wait for setup
 * WR_RDMA_WRITE_WITH_IMM  -------> WC_RDMA_WRITE(DATA)
 *     WC_RECV(ACK)       <-------      WR_SEND
 *                            .
 *                            .
 *                            .
 *
 * The setup phase exchanges the MR parameters of a fixed size circular buffer
 * to which remote writes are sent.  Sender tracks available space on the
 * receiver by accepting "Flow Control" messages (aka ACKs) that update the
 * tail pointer of the local "view" of the remote sink MR.
 *
 * Once setup is complete, either side may begin operations on the other's MR
 * (the initiator doesn't have to send first, as in the above example).
 *
 * If either credits or buffer space are exhausted, sender will spin waiting
 * for flow control messages.  Receiver will also not send flow control if
 * there are no credits available.  Flow control is currently sent on every
 * read, a possible optimization might be to send it less frequently.
 *
 * One catch is that Collage will only monitor a single "notifier" for events
 * and we have two that need to be monitored: one for the receive completion
 * queue (upon incoming RDMA write), and the other for connection status events
 * (the RDMA event channel) - RDMA_CM_EVENT_DISCONNECTED in particular.
 * Collage gets the receive completion queue's file descriptor and would never
 * detect a remote hangup as that fd does not signal on that condition.  This
 * is addressed by having a singleton "ChannelEventThread" who's sole purpose
 * in life is to watch for disconnect events and trigger a local flush so that
 * the receive queue is awoken (since errors *do* wake up the selector and
 * flush is an error condition).  This thread is launched on demand and will
 * exit when there are no active RDMA connections to monitor.
 *
 * Quite interesting is the effect of RDMA_RING_BUFFER_SIZE_MB and
 * RDMA_SEND_QUEUE_DEPTH depending on the communication pattern.  Basically,
 * bigger doesn't necessarily equate to faster!  The defaults are suited for
 * low latency conditions and would need tuning otherwise.
 *
 * ib_write_bw
 * -----------
 *  #bytes     #iterations    BW peak[MB/sec]    BW average[MB/sec]
 * 1048576    10000           3248.10            3247.94
 *
 * netperf
 * -------
 * Send perf: 3240.72MB/s (3240.72pps)
 * Send perf: 3240.72MB/s (3240.72pps)
 * Send perf: 3240.95MB/s (3240.95pps)
 *
 */
RDMAConnection::RDMAConnection( )
    : _notifier( -1 )
    , _timeout( Global::getIAttribute( Global::IATTR_RDMA_RESOLVE_TIMEOUT_MS ))
    , _rai( NULL )
    , _cm( NULL )
    , _cm_id( NULL )
    , _pd( NULL )
    , _established( false )
    , _credits( 0L )
    , _completions( 0U )
    , _msgbuf( sizeof(RDMAMessage) )
    , _sourcebuf( 0 )
    , _sourceptr( 0 )
    , _sinkbuf( IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE )
    , _sinkptr( 0 )
    , _rptr( 0UL )
    , _rbase( 0ULL )
    , _rkey( 0ULL )
    , _context( this )
    , _registered( false )
{
    EQVERB << (void *)this << ".new" << std::showbase
        << std::hex << "(" << RDMA_PROTOCOL_MAGIC
        << std::dec << ":" << RDMA_PROTOCOL_VERSION << ")"
        << std::endl;

    ::memset( (void *)&_addr, 0, sizeof(_addr) );
    ::memset( (void *)&_serv, 0, sizeof(_serv) );

    _description->type = CONNECTIONTYPE_RDMA;
    _description->bandwidth = // QDR default, report "actual" 8b/10b bandwidth
        ( ::ibv_rate_to_mult( IBV_RATE_40_GBPS ) * 2.5 * 1024000 / 8 ) * 0.8;
}

bool RDMAConnection::connect( )
{
    EQVERB << (void *)this << ".connect( )" << std::endl;

    EQASSERT( CONNECTIONTYPE_RDMA == _description->type );

    if( STATE_CLOSED != _state )
        return false;

    if( 0u == _description->port )
        return false;

    setState( STATE_CONNECTING );

    if( !_lookupAddress( false ) || ( NULL == _rai ))
    {
        EQERROR << "Failed to lookup destination address." << std::endl;
        goto err;
    }

    if( !_createEventChannel( ))
    {
        EQERROR << "Failed to create communication event channel." << std::endl;
        goto err;
    }

    if( !_createId( ))
    {
        EQERROR << "Failed to create communication identifier." << std::endl;
        goto err;
    }

    _updateInfo( _rai->ai_dst_addr );

    if( !_resolveAddress( ))
    {
        EQERROR << "Failed to resolve destination address for : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    _updateInfo( &_cm_id->route.addr.dst_addr );

    _device_name = ::ibv_get_device_name( _cm_id->verbs->device );

    EQVERB << "Initiating connection on "
        << _device_name << ":" << (int)_cm_id->port_num
        << " to "
        << _addr << ":" << _serv
        << " (" << _description->toString( ) << ")"
        << std::endl;

    _credits = Global::getIAttribute( Global::IATTR_RDMA_SEND_QUEUE_DEPTH );
    if( _credits <= 0L )
    {
        EQERROR << "Invalid queue depth." << std::endl;
        goto err;
    }

    if( !_createQP( ))
    {
        EQERROR << "Failed to create queue pair." << std::endl;
        goto err;
    }

    if( !_resolveRoute( ))
    {
        EQERROR << "Failed to resolve route to destination : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    if( !_initBuffers( ))
    {
        EQERROR << "Failed to initialize ring buffers." << std::endl;
        goto err;
    }

    if( !_postReceives( static_cast< uint32_t >( _credits )))
    {
        EQERROR << "Failed to pre-post receives." << std::endl;
        goto err;
    }

    if( !_connect( ))
    {
        EQERROR << "Failed to connect to destination : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    if(( RDMA_PROTOCOL_MAGIC != _cpd.magic ) ||
        ( RDMA_PROTOCOL_VERSION != _cpd.version ))
    {
        EQERROR << "Protocol mismatch with target : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    if( !_eventThreadRegister( ))
    {
        EQERROR << "Failed to register with event thread." << std::endl;
        goto err;
    }

    if( !_postSetup( ))
    {
        EQERROR << "Failed to post setup message." << std::endl;
        goto err;
    }

    if( !_waitRecvSetup( ))
    {
        EQERROR << "Failed to receive setup message." << std::endl;
        goto err;
    }

    EQINFO << "Connection established on "
        << _device_name << ":" << (int)_cm_id->port_num
        << " to "
        << _addr << ":" << _serv
        << " (" << _description->toString( ) << ")"
        << std::endl;

    // For a connected instance, the receive completion channel fd will indicate
    // on events such as new incoming data by waking up any polling operation.
    _notifier = _cm_id->recv_cq_channel->fd;
    setState( STATE_CONNECTED );
    return true;

err:
    EQINFO << "Connection failed to remote address : "
        << _addr << ":" << _serv << std::endl;

    close( );
    return false;
}

bool RDMAConnection::listen( )
{
    EQVERB << (void *)this << ".listen( )" << std::endl;

    EQASSERT( CONNECTIONTYPE_RDMA == _description->type );

    if( STATE_CLOSED != _state )
        return false;

    setState( STATE_CONNECTING );

    if( !_lookupAddress( true ))
    {
        EQERROR << "Failed to lookup local address." << std::endl;
        goto err;
    }

    if( !_createEventChannel( ))
    {
        EQERROR << "Failed to create communication event channel." << std::endl;
        goto err;
    }

    if( !_createId( ))
    {
        EQERROR << "Failed to create communication identifier." << std::endl;
        goto err;
    }

#if 0
    /* NOT IMPLEMENTED */

    if( ::rdma_set_option( _cm_id, RDMA_OPTION_ID, RDMA_OPTION_ID_REUSEADDR,
            (void *)&ONE, sizeof(ONE) ))
    {
        EQERROR << "rdma_set_option : " << base::sysError << std::endl;
        goto err;
    }
#endif

    if( NULL != _rai )
        _updateInfo( _rai->ai_src_addr );

    if( !_bindAddress( ))
    {
        EQERROR << "Failed to bind to local address : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    if( NULL != _rai )
        _updateInfo( &_cm_id->route.addr.src_addr );
    else
    {
        ::gethostname( _addr, NI_MAXHOST );
        _description->setHostname( _addr );
    }

    if( !_listen( ))
    {
        EQERROR << "Failed to listen on bound address : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    if( NULL != _cm_id->verbs )
        _device_name = ::ibv_get_device_name( _cm_id->verbs->device );

    EQINFO << "Listening on "
        << _device_name << ":" << (int)_cm_id->port_num
        << " at "
        << _addr << ":" << _serv
        << " (" << _description->toString( ) << ")"
        << std::endl;

    // For a listening instance, the connection manager fd will indicate
    // on events such as new incoming connections by waking up any polling
    // operation.
    _notifier = _cm->fd;
    setState( STATE_LISTENING );
    return true;

err:
    close( );
    return false;
}

void RDMAConnection::close( )
{
    EQVERB << (void *)this << ".close( )" << std::endl;

    base::ScopedMutex<> mutex( _close_mutex );

    if( STATE_CLOSED != _state )
    {
        EQASSERT( STATE_CLOSING != _state );
        setState( STATE_CLOSING );

        base::Clock clock;
        const int64_t start = clock.getTime64( );
        const uint32_t timeout = Global::getTimeout( );

        // Wait for outstanding acks.
        while( !_rptr.isEmpty( ) && _established && _pollCQ( ))
        {
            if( EQ_TIMEOUT_INDEFINITE != timeout )
            {
                if(( clock.getTime64( ) - start ) > timeout )
                {
                    EQERROR << "Timed out waiting for acks." << std::endl;
                    break;
                }
            }

            co::base::Thread::yield( );
        }

        _eventThreadUnregister( );

        // TODO : verify this method of determining if we can call disconnect
        // without getting a error (and spitting out a unnecessary warning).
        if( _cm_id && _cm_id->qp && ( _cm_id->qp->state > IBV_QPS_INIT ) &&
                ::rdma_disconnect( _cm_id ))
            EQWARN << "rdma_disconnect : " << base::sysError << std::endl;

        _cleanup( );

        setState( STATE_CLOSED );
    }
}

void RDMAConnection::acceptNB( ) { /* NOP */ }

ConnectionPtr RDMAConnection::acceptSync( )
{
    EQVERB << (void *)this << ".acceptSync( )" << std::endl;

    if( STATE_LISTENING != _state )
        return NULL;

    RDMAConnection *newConnection = new RDMAConnection( );

    if( !newConnection->_finishAccept( _cm ))
    {
        delete newConnection;
        newConnection = NULL;
    }

    return newConnection;
}

void RDMAConnection::readNB( void* buffer, const uint64_t bytes ) { /* NOP */ }

int64_t RDMAConnection::readSync( void* buffer, const uint64_t bytes,
    const bool )
{
//    EQWARN << (void *)this << std::dec << ".read(" << bytes << ")"
//       << std::endl;

    base::Clock clock;
    const int64_t start = clock.getTime64( );
    const uint32_t timeout = Global::getTimeout( );

    _stats.reads++;

retry1:
    const uint32_t bytes_taken = _drain( buffer, bytes );

retry2:
    if( !_pollCQ( ))
    {
        EQERROR << "Error while polling completion queues." << std::endl;
        goto err;
    }

    if( 0UL == bytes_taken )
    {
        if( _sinkptr.isEmpty( ) && !_established )
        {
            EQINFO << "Got EOF, closing connection." << std::endl;
            close( );
            goto err;
        }

        if( EQ_TIMEOUT_INDEFINITE != timeout )
        {
            if(( clock.getTime64( ) - start ) > timeout )
            {
                EQERROR << "Timed out trying to drain buffer." << std::endl;
                goto err;
            }
        }

        //EQWARN << "Sink buffer empty." << std::endl;
        co::base::Thread::yield( );
        _stats.buffer_empty++;
        goto retry1;
    }

    EQASSERT( _credits >= 0L );

    if(( 0L == _credits ) && _established )
    {
        if( EQ_TIMEOUT_INDEFINITE != timeout )
        {
            if(( clock.getTime64( ) - start ) > timeout )
            {
                EQERROR << "Timed out trying to acquire credit." << std::endl;
                goto err;
            }
        }

        //EQWARN << "No credit for flow control." << std::endl;
        co::base::Thread::yield( );
        _stats.no_credits_fc++;
        goto retry2;
    }

    // TODO : post FC less frequently
    if( !_postFC( ))
        EQWARN << "Error while posting flow control message." << std::endl;

    // We only want to clear the "readability" of the notifier when we know
    // we no longer have any data in the buffer and need to be notified
    // when we receive more.
    if( _sinkptr.isEmpty( ) && !_rearmCQ( ))
    {
        EQERROR << "Error while rearming receive channel." << std::endl;
        goto err;
    }

//    EQWARN << (void *)this << std::dec << ".read(" << bytes << ")"
//       << " took " << bytes_taken << " bytes"
//       << " (" << _sinkptr.available( ) << " still available)" << std::endl;

    return static_cast< int64_t >( bytes_taken );

err:
    return -1LL;
}

int64_t RDMAConnection::write( const void* buffer, const uint64_t bytes )
{
//    EQWARN << (void *)this << std::dec << ".write(" << bytes << ")"
//        << std::endl;

    if( STATE_CONNECTED != _state )
        return -1LL;

    base::Clock clock;
    const int64_t start = clock.getTime64( );
    const uint32_t timeout = Global::getTimeout( );

    _stats.writes++;

    // Can only send sizeof(struct ibv_wc.imm_data) per shot.
    const uint32_t can_put = static_cast< uint32_t >( std::min( bytes,
        static_cast< uint64_t >( std::numeric_limits< uint32_t >::max( ))));
    uint32_t bytes_put;

retry:
    if( !_pollCQ( ))
    {
        EQERROR << "Error while polling completion queues." << std::endl;
        goto err;
    }

    if( !_established )
    {
        EQINFO << "Got EOF, closing connection." << std::endl;
        close( );
        goto err;
    }

    EQASSERT( _credits >= 0L );

    if( 0L == _credits )
    {
        if( EQ_TIMEOUT_INDEFINITE != timeout )
        {
            if(( clock.getTime64( ) - start ) > timeout )
            {
                EQERROR << "Timed out trying to acquire credit." << std::endl;
                goto err;
            }
        }

        //EQWARN << "No credits for RDMA." << std::endl;
        co::base::Thread::yield( );
        _stats.no_credits_rdma++;
        goto retry;
    }

    bytes_put = _fill( buffer, can_put );

    if( 0UL == bytes_put )
    {
        if( EQ_TIMEOUT_INDEFINITE != timeout )
        {
            if(( clock.getTime64( ) - start ) > timeout )
            {
                EQERROR << "Timed out trying to fill buffer." << std::endl;
                goto err;
            }
        }

        //EQWARN << "Source buffer full." << std::endl;
        co::base::Thread::yield( );
        _stats.buffer_full++;
        goto retry;
    }

    if( !_postRDMAWrite( ))
    {
        EQERROR << "Error while posting RDMA write." << std::endl;
        goto err;
    }

//    EQWARN << (void *)this << std::dec << ".write(" << bytes << ")"
//       << " put " << bytes_put << " bytes" << std::endl;

    return static_cast< int64_t >( bytes_put );

err:
    return -1LL;
}

RDMAConnection::~RDMAConnection( )
{
    EQVERB << (void *)this << ".delete" << std::endl;

    close( );
}

void RDMAConnection::setState( const State state )
{
    if( state != _state )
    {
        _state = state;
        _fireStateChanged( );
    }
}

////////////////////////////////////////////////////////////////////////////////

void RDMAConnection::_cleanup( )
{
    EQASSERT( STATE_CLOSING == _state );

    _sourcebuf.clear( );
    _sinkbuf.clear( );
    _msgbuf.clear( );

    if( _completions > 0U )
    {
        ::ibv_ack_cq_events( _cm_id->recv_cq, _completions );
        _completions = 0U;
    }

    if( NULL != _cm_id )
        ::rdma_destroy_ep( _cm_id );
    _cm_id = NULL;

    if(( NULL != _pd ) && ::rdma_seterrno( ::ibv_dealloc_pd( _pd )))
        EQWARN << "ibv_dealloc_pd : " << base::sysError << std::endl;
    _pd = NULL;

    if( NULL != _cm )
        ::rdma_destroy_event_channel( _cm );
    _cm = NULL;

    if( NULL != _rai )
        ::rdma_freeaddrinfo( _rai );
    _rai = NULL;

    _rptr = 0UL;
    _rbase = _rkey = 0ULL;

    _notifier = -1;
}

bool RDMAConnection::_finishAccept( struct rdma_event_channel *listen_channel )
{
    EQASSERT( STATE_CLOSED == _state );
    setState( STATE_CONNECTING );

    if( !_doCMEvent( listen_channel, RDMA_CM_EVENT_CONNECT_REQUEST ))
    {
        EQERROR << "Failed to receive valid connect request." << std::endl;
        goto err;
    }

    EQASSERT( NULL != _cm_id );

    _updateInfo( &_cm_id->route.addr.dst_addr );

    _device_name = ::ibv_get_device_name( _cm_id->verbs->device );

    EQVERB << "Connection initiated on "
        << _device_name << ":" << (int)_cm_id->port_num
        << " from "
        << _addr << ":" << _serv
        << " (" << _description->toString( ) << ")"
        << std::endl;

    if(( RDMA_PROTOCOL_MAGIC != _cpd.magic ) ||
        ( RDMA_PROTOCOL_VERSION != _cpd.version ))
    {
        EQERROR << "Protocol mismatch with initiator : "
            << _addr << ":" << _serv << std::endl;
        goto err_reject;
    }

    if( !_createEventChannel( ))
    {
        EQERROR << "Failed to create event channel." << std::endl;
        goto err_reject;
    }

    if( !_migrateId( ))
    {
        EQERROR << "Failed to migrate communication identifier." << std::endl;
        goto err_reject;
    }

    _credits = _cpd.depth;
    if( _credits <= 0L )
    {
        EQERROR << "Invalid (unsent?) queue depth." << std::endl;
        goto err_reject;
    }

    if( !_createQP( ))
    {
        EQERROR << "Failed to create queue pair." << std::endl;
        goto err_reject;
    }

    if( !_initBuffers( ))
    {
        EQERROR << "Failed to initialize ring buffers." << std::endl;
        goto err_reject;
    }

    if( !_postReceives( static_cast< uint32_t >( _credits )))
    {
        EQERROR << "Failed to pre-post receives." << std::endl;
        goto err_reject;
    }

    if( !_accept( ))
    {
        EQERROR << "Failed to accept remote connection from : "
            << _addr << ":" << _serv << std::endl;
        goto err;
    }

    if( !_eventThreadRegister( ))
    {
        EQERROR << "Failed to register with event thread." << std::endl;
        goto err;
    }

    if( !_postSetup( ))
    {
        EQERROR << "Failed to post setup message." << std::endl;
        goto err;
    }

    if( !_waitRecvSetup( ))
    {
        EQERROR << "Failed to receive setup message." << std::endl;
        goto err;
    }

    EQVERB << "Connection accepted on " << _device_name << ":" 
           << (int)_cm_id->port_num << " from " << _addr << ":" << _serv << " ("
           << _description->toString( ) << ")" << std::endl;

    // For a connected instance, the receive completion channel fd will indicate
    // on events such as new incoming data by waking up any polling operation.
    _notifier = _cm_id->recv_cq_channel->fd;
    setState( STATE_CONNECTED );
    return true;

err_reject:
    EQINFO << "Rejecting connection from remote address : "
        << _addr << ":" << _serv << std::endl;

    if( !_reject( ))
        EQWARN << "Failed to issue connection reject." << std::endl;

err:
    close( );
    return false;
}

bool RDMAConnection::_lookupAddress( const bool passive )
{
    struct rdma_addrinfo hints;
    char *node = NULL, *service = NULL;
    std::string s;

    ::memset( (void *)&hints, 0, sizeof(struct rdma_addrinfo) );
    //hints.ai_flags |= RAI_NOROUTE;
    if( passive )
        hints.ai_flags |= RAI_PASSIVE;

    const std::string &hostname = _description->getHostname( );
    if( !hostname.empty( ))
        node = const_cast< char * >( hostname.c_str( ));

    if( 0u != _description->port )
    {
        std::stringstream ss;
        ss << _description->port;
        s = ss.str( );
        service = const_cast< char * >( s.c_str( ));
    }

    if(( NULL != node ) && ::rdma_getaddrinfo( node, service, &hints, &_rai ))
    {
        EQERROR << "rdma_getaddrinfo : " << base::sysError << std::endl;
        goto err;
    }

    if(( NULL != _rai ) && ( NULL != _rai->ai_next ))
        EQWARN << "Multiple getaddrinfo results, using first." << std::endl;

    if(( NULL != _rai ) && ( _rai->ai_connect_len > 0 ))
        EQWARN << "WARNING : ai_connect data specified!" << std::endl;

    return true;

err:
    return false;
}

void RDMAConnection::_updateInfo( struct sockaddr *addr )
{
    int err;
    if(( err = ::getnameinfo( addr, ( AF_INET == addr->sa_family ) ?
                sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
            _addr, sizeof(_addr), _serv, sizeof(_serv),
            NI_NUMERICHOST | NI_NUMERICSERV )))
        EQWARN << "Name info lookup failed : " << err << std::endl;

    if( _description->getHostname( ).empty( ))
        _description->setHostname( _addr );
    if( 0u == _description->port )
        _description->port = atoi( _serv );
}

bool RDMAConnection::_createEventChannel( )
{
    EQASSERT( NULL == _cm );

    _cm = ::rdma_create_event_channel( );
    if( NULL == _cm )
    {
        EQERROR << "rdma_create_event_channel : " << base::sysError <<
            std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_createId( )
{
    EQASSERT( NULL != _cm );

    if( ::rdma_create_id( _cm, &_cm_id, NULL, RDMA_PS_TCP ))
    {
        EQERROR << "rdma_create_id : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_createQP( )
{
    struct ibv_qp_init_attr init_attr;

    _pd = ::ibv_alloc_pd( _cm_id->verbs );
    if( NULL == _pd )
    {
        EQERROR << "ibv_alloc_pd : " << base::sysError << std::endl;
        goto err;
    }

    ::memset( (void *)&init_attr, 0, sizeof(struct ibv_qp_init_attr) );
    init_attr.cap.max_send_wr = static_cast< uint32_t >( _credits );
    init_attr.cap.max_recv_wr = static_cast< uint32_t >( _credits );
    init_attr.cap.max_recv_sge = 1;
    init_attr.cap.max_send_sge = 1;
    init_attr.sq_sig_all = 1; // aka always IBV_SEND_SIGNALED
    init_attr.qp_type = IBV_QPT_RC;

    if( ::rdma_create_qp( _cm_id, _pd, &init_attr ))
    {
        EQERROR << "rdma_create_qp : " << base::sysError << std::endl;
        goto err;
    }

    // Request only solicited events (i.e. don't wake up Collage on ACKs).
    if( ::rdma_seterrno( ::ibv_req_notify_cq( _cm_id->recv_cq, 1 )))
    {
        EQERROR << "ibv_req_notify_cq : " << base::sysError << std::endl;
        goto err;
    }

    EQVERB << "RDMA QP caps : " << std::dec <<
        init_attr.cap.max_recv_wr << " receives, " <<
        init_attr.cap.max_send_wr << " sends, " << std::endl;

    // Need enough space for sends and receives.
    return _msgbuf.resize( _pd, static_cast< uint32_t >( _credits * 2 ));

err:
    return false;
}

bool RDMAConnection::_initBuffers( )
{
    const size_t rbs = 1024 * 1024 *
        Global::getIAttribute( Global::IATTR_RDMA_RING_BUFFER_SIZE_MB );

    if( 0 == rbs )
    {
        EQERROR << "Invalid RDMA ring buffer size." << std::endl;
        goto err;
    }

    if( !_sourcebuf.resize( _pd, rbs ))
    {
        EQERROR << "Failed to resize source buffer." << std::endl;
        goto err;
    }

    if( !_sinkbuf.resize( _pd, rbs ))
    {
        EQERROR << "Failed to resize sink buffer." << std::endl;
        goto err;
    }

    _sourceptr.clear( _sourcebuf.getSize( ));
    _sinkptr.clear( _sinkbuf.getSize( ));
    return true;

err:
    return false;
}

bool RDMAConnection::_resolveAddress( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( NULL != _rai );

    if( ::rdma_resolve_addr( _cm_id, _rai->ai_src_addr, _rai->ai_dst_addr,
            _timeout ))
    {
        EQERROR << "rdma_resolve_addr : " << base::sysError << std::endl;
        goto err;
    }
    // Block for RDMA_CM_EVENT_ADDR_RESOLVED.
    return _doCMEvent( _cm, RDMA_CM_EVENT_ADDR_RESOLVED );

err:
    return false;
}

bool RDMAConnection::_resolveRoute( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( NULL != _rai );

    if(( IBV_TRANSPORT_IB == _cm_id->verbs->device->transport_type ) &&
            ( _rai->ai_route_len > 0 ))
    {
        if( ::rdma_set_option( _cm_id, RDMA_OPTION_IB, RDMA_OPTION_IB_PATH,
                _rai->ai_route, _rai->ai_route_len ))
        {
            EQERROR << "rdma_set_option : " << base::sysError << std::endl;
            goto err;
        }

        // rdma_resolve_route not required (TODO : is this really true?)
        return true;
    }

    if( ::rdma_resolve_route( _cm_id, _timeout ))
    {
        EQERROR << "rdma_resolve_route : " << base::sysError << std::endl;
        goto err;
    }
    // Block for RDMA_CM_EVENT_ROUTE_RESOLVED.
    return _doCMEvent( _cm, RDMA_CM_EVENT_ROUTE_RESOLVED );

err:
    return false;
}

bool RDMAConnection::_connect( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( !_established );

#if 0 // TODO
    static const uint8_t DSCP = 0;

    if( ::rdma_set_option( _cm_id, RDMA_OPTION_ID, RDMA_OPTION_ID_TOS,
            (void *)&DSCP, sizeof(DSCP) ))
    {
        EQERROR << "rdma_set_option : " << base::sysError << std::endl;
        goto err;
    }
#endif

    struct rdma_conn_param conn_param;

    ::memset( (void *)&conn_param, 0, sizeof(struct rdma_conn_param) );

    _cpd.magic = RDMA_PROTOCOL_MAGIC;
    _cpd.version = RDMA_PROTOCOL_VERSION;
    _cpd.depth = _credits;
    conn_param.private_data = reinterpret_cast< const void * >( &_cpd );
    conn_param.private_data_len = sizeof(struct RDMAConnParamData);
    conn_param.initiator_depth = RDMA_MAX_INIT_DEPTH;
    conn_param.responder_resources = RDMA_MAX_RESP_RES;
    // Magic 3-bit values.
    conn_param.retry_count = 7;
    conn_param.rnr_retry_count = 7;

    EQINFO << "Connect on source lid : " << std::showbase
        << std::hex << ntohs( _cm_id->route.path_rec->slid ) << " ("
        << std::dec << ntohs( _cm_id->route.path_rec->slid ) << ") "
        << "to dest lid : "
        << std::hex << ntohs( _cm_id->route.path_rec->dlid ) << " ("
        << std::dec << ntohs( _cm_id->route.path_rec->dlid ) << ") "
        << std::endl;

    if( ::rdma_connect( _cm_id, &conn_param ))
    {
        EQERROR << "rdma_connect : " << base::sysError << std::endl;
        goto err;
    }
    // Block for RDMA_CM_EVENT_ESTABLISHED.
    return _doCMEvent( _cm, RDMA_CM_EVENT_ESTABLISHED );

err:
    return false;
}

bool RDMAConnection::_bindAddress( )
{
    EQASSERT( NULL != _cm_id );

#if IPV6_DEFAULT
    struct sockaddr_in6 sin;
    sin.sin6_family = AF_INET6;
    sin.sin6_port = htons( _description->port );
    sin.sin6_addr = in6addr_any;
#else
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons( _description->port );
    sin.sin_addr.s_addr = INADDR_ANY;
#endif

    if( ::rdma_bind_addr( _cm_id, ( NULL != _rai ) ? _rai->ai_src_addr :
            reinterpret_cast< struct sockaddr * >( &sin )))
    {
        EQERROR << "rdma_bind_addr : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_listen( )
{
    EQASSERT( NULL != _cm_id );

    if( ::rdma_listen( _cm_id, SOMAXCONN ))
    {
        EQERROR << "rdma_listen : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_migrateId( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( NULL != _cm );

    if( ::rdma_migrate_id( _cm_id, _cm ))
    {
        EQERROR << "rdma_migrate_id : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_accept( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( !_established );

    struct rdma_conn_param accept_param;

    ::memset( (void *)&accept_param, 0, sizeof(struct rdma_conn_param) );

    _cpd.magic = RDMA_PROTOCOL_MAGIC;
    _cpd.version = RDMA_PROTOCOL_VERSION;
    _cpd.depth = _credits;
    accept_param.private_data = reinterpret_cast< const void * >( &_cpd );
    accept_param.private_data_len = sizeof(struct RDMAConnParamData);
    accept_param.initiator_depth = RDMA_MAX_INIT_DEPTH;
    accept_param.responder_resources = RDMA_MAX_RESP_RES;
    // Magic 3-bit value.
    accept_param.rnr_retry_count = 7;

    EQVERB << "Accept on source lid : "<< std::showbase
           << std::hex << ntohs( _cm_id->route.path_rec->slid ) << " ("
           << std::dec << ntohs( _cm_id->route.path_rec->slid ) << ") "
           << "from dest lid : "
           << std::hex << ntohs( _cm_id->route.path_rec->dlid ) << " ("
           << std::dec << ntohs( _cm_id->route.path_rec->dlid ) << ") "
           << std::endl;

    if( ::rdma_accept( _cm_id, &accept_param ))
    {
        EQERROR << "rdma_accept : " << base::sysError << std::endl;
        goto err;
    }
    // Block for RDMA_CM_EVENT_ESTABLISHED.
    return _doCMEvent( _cm, RDMA_CM_EVENT_ESTABLISHED );

err:
    return false;
}

bool RDMAConnection::_reject( )
{
    if( ::rdma_reject( _cm_id, NULL, 0 ))
    {
        EQERROR << "rdma_reject : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_postReceives( const uint32_t count )
{
    EQASSERT( NULL != _cm_id->qp );
    EQASSERT( count > 0UL );

    struct ibv_sge sge[count];
    struct ibv_recv_wr wrs[count];

    for( uint32_t i = 0UL; i != count; i++ )
    {
        sge[i].addr = (uint64_t)(uintptr_t)_msgbuf.getBuffer( );
        sge[i].length = (uint64_t)_msgbuf.getBufferSize( );
        sge[i].lkey = _msgbuf.getMR( )->lkey;

        wrs[i].wr_id = sge[i].addr;
        wrs[i].next = &wrs[i + 1];
        wrs[i].sg_list = &sge[i];
        wrs[i].num_sge = 1;
    }
    wrs[count - 1].next = NULL;

    struct ibv_recv_wr *bad_wr;
    if( ::rdma_seterrno( ::ibv_post_recv( _cm_id->qp, wrs, &bad_wr )))
    {
        EQERROR << "ibv_post_recv : "  << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

/* inline */ void RDMAConnection::_recvRDMAWrite( const uint32_t imm_data )
{
    // Analysis:
    //
    // Since the ring pointers are circular, a malicious (presumably overflow)
    // value here would at worst only result in us reading arbitrary regions
    // from our sink buffer, not segfaulting.  If the other side wanted us to
    // reread a previous message it should just resend it!
    _sinkptr.incrHead( ntohl( imm_data ));
}

bool RDMAConnection::_postRDMAWrite( )
{
    struct ibv_sge sge;
    struct ibv_send_wr wr;

    sge.addr = (uint64_t)( (uintptr_t)_sourcebuf.getBase( ) +
        _sourceptr.ptr( _sourceptr.MIDDLE ));
    sge.length = (uint64_t)_sourceptr.available( _sourceptr.HEAD,
        _sourceptr.MIDDLE );
    sge.lkey = _sourcebuf.getMR( )->lkey;
    _sourceptr.incr( _sourceptr.MIDDLE, (uint32_t)sge.length );

    wr.wr_id = (uint64_t)_sourceptr.value( _sourceptr.MIDDLE );
    wr.next = NULL;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
    wr.send_flags = IBV_SEND_SOLICITED; // Important!
    wr.imm_data = htonl( (uint32_t)sge.length );
    wr.wr.rdma.rkey = _rkey;
    wr.wr.rdma.remote_addr = (uint64_t)( (uintptr_t)_rbase +
        _rptr.ptr( _rptr.HEAD ));
    _rptr.incrHead( (uint32_t)sge.length );

    struct ibv_send_wr *bad_wr;
    if( ::rdma_seterrno( ::ibv_post_send( _cm_id->qp, &wr, &bad_wr )))
    {
        EQERROR << "ibv_post_send : "  << base::sysError << std::endl;
        goto err;
    }

    _credits--;

    EQASSERT( _credits >= 0L );

    return true;

err:
    return false;
}

void RDMAConnection::_recvMessage( const RDMAMessage &message )
{
    switch( message.opcode )
    {
        case FC:
            if( sizeof(struct RDMAFCPayload) == (size_t)message.length )
                _recvFC( message.payload.fc );
            else
                EQWARN << "Invalid flow control message received!" << std::endl;
            break;
        case SETUP:
            if( sizeof(struct RDMASetupPayload) == (size_t)message.length )
                _recvSetup( message.payload.setup );
            else
                EQWARN << "Invalid setup message received!" << std::endl;
            break;
        default:
            EQWARN << "Invalid message received!" << std::endl;
    }
}

/* inline */ void RDMAConnection::_recvFC( const RDMAFCPayload &fc )
{
    // Analysis:
    //
    // Since we will only write a maximum of _sourceptr.available( ) bytes
    // to our source buffer, a malicious (presumably overflow) value here would
    // have no chance of causing us to write beyond our buffer as we have local
    // control over those ring pointers.  Worst case, we'd and up writing to
    // arbitrary regions of the remote buffer, since this ring pointer is
    // circular as well.
    _rptr.moveValue( _rptr.TAIL, fc.ringTail );
}

bool RDMAConnection::_postMessage( const RDMAMessage &message )
{
    if( ::rdma_post_send( _cm_id, (void *)&message, (void *)&message,
            offsetof( RDMAMessage, payload ) + message.length, _msgbuf.getMR( ),
            0 ))
    {
        EQERROR << "rdma_post_send : "  << base::sysError << std::endl;
        goto err;
    }

    _credits--;

    EQASSERT( _credits >= 0L );

    return true;

err:
    return false;
}

bool RDMAConnection::_postFC( )
{
    RDMAMessage &message =
        *reinterpret_cast< RDMAMessage * >( _msgbuf.getBuffer( ));

    message.opcode = FC;
    message.length = (uint8_t)sizeof(struct RDMAFCPayload);

    message.payload.fc.ringTail = _sinkptr.value( _sinkptr.TAIL );

    return _postMessage( message );
}

void RDMAConnection::_recvSetup( const RDMASetupPayload &setup )
{
    // Analysis:
    //
    // Malicious values here would only affect the receiver, we're willing
    // to RDMA write to anywhere specified!
    _rbase = setup.rbase;
    _rptr.clear( setup.rlen );
    _rkey = setup.rkey;

    EQVERB << "RDMA MR: " << std::showbase
        << std::dec << setup.rlen << " @ "
        << std::hex << setup.rbase << std::endl;
}

bool RDMAConnection::_postSetup( )
{
    RDMAMessage &message =
        *reinterpret_cast< RDMAMessage * >( _msgbuf.getBuffer( ));

    message.opcode = SETUP;
    message.length = (uint8_t)sizeof(struct RDMASetupPayload);

    message.payload.setup.rbase = (uint64_t)(uintptr_t)_sinkbuf.getBase( );
    message.payload.setup.rlen = (uint64_t)_sinkbuf.getSize( );
    message.payload.setup.rkey = _sinkbuf.getMR( )->rkey;

    return _postMessage( message );
}

bool RDMAConnection::_waitRecvSetup( )
{
    base::Clock clock;
    const int64_t start = clock.getTime64( );
    const uint32_t timeout = Global::getTimeout( );

retry:
    if( !_pollCQ( ))
    {
        EQERROR << "Error while polling completion queue." << std::endl;
        goto err;
    }

    if(( 0ULL == _rkey ) && _established )
    {
        if( EQ_TIMEOUT_INDEFINITE != timeout )
        {
            if(( clock.getTime64( ) - start ) > timeout )
            {
                EQERROR << "Timed out waiting for setup message." << std::endl;
                goto err;
            }
        }

        co::base::Thread::yield( );
        goto retry;
    }

    return true;

err:
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool RDMAConnection::_doCMEvent( struct rdma_event_channel *channel,
    enum rdma_cm_event_type expected )
{
    bool ok = false;
    struct rdma_cm_event *event;

    if( ::rdma_get_cm_event( channel, &event ))
    {
        EQERROR << "rdma_get_cm_event : " << base::sysError << std::endl;
        goto out;
    }

    ok = ( event->event == expected );

#ifndef NDEBUG
    if( ok )
        EQVERB << (void *)this << " event : " << ::rdma_event_str( event->event )
               << std::endl;
    else
        EQINFO << (void *)this << " event : " << ::rdma_event_str( event->event )
               << " expected: " << ::rdma_event_str( expected ) << std::endl;
#endif

    if( ok && ( RDMA_CM_EVENT_DISCONNECTED == event->event ))
        _established = false;

    if( ok && ( RDMA_CM_EVENT_ESTABLISHED == event->event ))
    {
        _established = true;

        struct rdma_conn_param *cp = &event->param.conn;

        ::memset( (void *)&_cpd, 0, sizeof(RDMAConnParamData) );
        // Note that the actual amount of data transferred to the remote side
        // is transport dependent and may be larger than that requested.
        if( cp->private_data_len >= sizeof(RDMAConnParamData) )
            _cpd = *reinterpret_cast< const RDMAConnParamData * >(
                cp->private_data );
    }

    if( ok && ( RDMA_CM_EVENT_CONNECT_REQUEST == event->event ))
    {
        _cm_id = event->id;

        struct rdma_conn_param *cp = &event->param.conn;

        ::memset( (void *)&_cpd, 0, sizeof(RDMAConnParamData) );
        // TODO : Not sure what happens when initiator sent ai_connect data
        // (assuming the underlying transport doesn't strip it)?
        if( cp->private_data_len >= sizeof(RDMAConnParamData) )
            _cpd = *reinterpret_cast< const RDMAConnParamData * >(
                cp->private_data );
    }

    if( RDMA_CM_EVENT_REJECTED == event->event )
        EQINFO << "Connection reject status : " << event->status << std::endl;

    if( ::rdma_ack_cm_event( event ))
        EQWARN << "rdma_ack_cm_event : "  << base::sysError << std::endl;

out:
    return ok;
}

bool RDMAConnection::_pollCQ( )
{
    struct ibv_wc wcs[static_cast< uint32_t >( _credits )];
    uint32_t num_recvs = 0UL;
    int count;

    base::ScopedMutex<> mutex( _poll_mutex );

    /* CHECK RECEIVE COMPLETIONS */
    count = ::ibv_poll_cq( _cm_id->recv_cq, sizeof(wcs) / sizeof(wcs[0]), wcs );
    if( count < 0 )
    {
        EQERROR << "ibv_poll_cq : " << base::sysError << std::endl;
        goto err;
    }

    for( int i = 0; i != count ; i++ )
    {
        struct ibv_wc &wc = wcs[i];

        if( IBV_WC_SUCCESS != wc.status )
        {
            // Non-fatal.
            if( IBV_WC_WR_FLUSH_ERR == wc.status )
                continue;

            EQWARN << (void *)this << " !IBV_WC_SUCCESS : " << std::showbase
                << std::hex << "wr_id = " << wc.wr_id
                << ", status = \"" << ::ibv_wc_status_str( wc.status ) << "\""
                << std::dec << " (" << (unsigned int)wc.status << ")"
                << std::hex << ", vendor_err = " << wc.vendor_err
                << std::dec << std::endl;

            // All others are fatal.
            goto err;
        }

        EQASSERT( IBV_WC_SUCCESS == wc.status );
        EQASSERT( IBV_WC_RECV & wc.opcode );

        // All receive completions need to be reposted.
        num_recvs++;

        if( IBV_WC_RECV_RDMA_WITH_IMM == wc.opcode )
            _recvRDMAWrite( wc.imm_data );
        else if( IBV_WC_RECV == wc.opcode )
            _recvMessage( *reinterpret_cast< RDMAMessage * >( wc.wr_id ));
        else
            EQUNREACHABLE;

        _msgbuf.freeBuffer( (void *)(uintptr_t)wc.wr_id );
    }

    if(( num_recvs > 0UL ) && !_postReceives( num_recvs ))
        goto err;

    /* CHECK SEND COMPLETIONS */
    count = ::ibv_poll_cq( _cm_id->send_cq, sizeof(wcs) / sizeof(wcs[0]), wcs );
    if( count < 0 )
    {
        EQERROR << "ibv_poll_cq : " << base::sysError << std::endl;
        goto err;
    }

    for( int i = 0; i != count ; i++ )
    {
        struct ibv_wc &wc = wcs[i];

        if( IBV_WC_SUCCESS != wc.status )
        {
            // Non-fatal.
            if( IBV_WC_WR_FLUSH_ERR == wc.status )
                continue;

            EQWARN << (void *)this << " !IBV_WC_SUCCESS : " << std::showbase
                << std::hex << "wr_id = " << wc.wr_id
                << ", status = \"" << ::ibv_wc_status_str( wc.status ) << "\""
                << std::dec << " (" << (unsigned int)wc.status << ")"
                << std::hex << ", vendor_err = " << wc.vendor_err
                << std::dec << std::endl;

            // Warning only as we just might be trying to ack a dead sender.
            if(( IBV_WC_RETRY_EXC_ERR == wc.status )/* ||
                ( IBV_WC_RNR_RETRY_EXC_ERR == wc.status )*/)
                continue;

            // All others are fatal.
            goto err;
        }

        EQASSERT( IBV_WC_SUCCESS == wc.status );

        // All send completions replenish credit.
        _credits++;

        if( IBV_WC_SEND == wc.opcode )
            _msgbuf.freeBuffer( (void *)(uintptr_t)wc.wr_id );
        else if( IBV_WC_RDMA_WRITE == wc.opcode )
            _sourceptr.moveValue( _sourceptr.TAIL, (uint32_t)wc.wr_id );
        else
            EQUNREACHABLE;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_rearmCQ( )
{
    struct ibv_cq *ev_cq;
    void *ev_ctx;

    if( ::ibv_get_cq_event( _cm_id->recv_cq_channel, &ev_cq, &ev_ctx ) == -1 )
    {
        EQERROR << "ibv_get_cq_event : " << base::sysError << std::endl;
        goto err;
    }

    // http://lists.openfabrics.org/pipermail/general/2008-November/055237.html
    _completions++;
    if( std::numeric_limits< unsigned int >::max( ) == _completions )
    {
        ::ibv_ack_cq_events( _cm_id->recv_cq, _completions );
        _completions = 0U;
    }

    if( ::rdma_seterrno( ::ibv_req_notify_cq( _cm_id->recv_cq, 1 )))
    {
        EQERROR << "ibv_req_notify_cq : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

uint32_t RDMAConnection::_drain( void *buffer, const uint32_t bytes )
{
    const uint32_t b = std::min( bytes, _sinkptr.available( ));
    ::memcpy( buffer, (const void *)((uintptr_t)_sinkbuf.getBase( ) +
        _sinkptr.tail( )), b );
    _sinkptr.incrTail( b );
    return b;
}

uint32_t RDMAConnection::_fill( const void *buffer, const uint32_t bytes )
{
    const uint32_t b = std::min( bytes,
        std::min( _sourceptr.negAvailable( _sourceptr.HEAD, _sourceptr.TAIL ),
            _rptr.negAvailable( _rptr.HEAD, _rptr.TAIL )));
    ::memcpy( (void *)((uintptr_t)_sourcebuf.getBase( ) +
        _sourceptr.ptr( _sourceptr.HEAD )), buffer, b );
    _sourceptr.incrHead( b );
    return b;
}

//////////////////////////////////////////////////////////////////////////////

RDMAConnection::ChannelEventThread *RDMAConnection::_event_thread = NULL;
base::Lock RDMAConnection::_thread_mutex;

class RDMAConnection::ChannelEventThread : public base::Thread
{
public:
    ChannelEventThread( );
    virtual ~ChannelEventThread( );

    virtual bool init( );
    virtual void run( );

    bool add( RDMAConnection *conn );
    bool remove( RDMAConnection *conn );
private:
    bool _wake( );

    struct epoll_context _context;
    int _event_fd;
    int _epoll_fd;
    RDMAConnection *_to_add, *_to_remove; // Depth-one "queues"
};

RDMAConnection::ChannelEventThread::ChannelEventThread( )
    : _context( this )
    , _event_fd( -1 )
    , _epoll_fd( -1 )
    , _to_add( NULL )
    , _to_remove( NULL )
{
}

RDMAConnection::ChannelEventThread::~ChannelEventThread( )
{
    if(( _epoll_fd >= 0 ) && ( _event_fd >= 0 ))
    {
        struct epoll_event evctl;

        ::memset( (void *)&evctl, 0, sizeof(struct epoll_event) );
        if( ::epoll_ctl( _epoll_fd, EPOLL_CTL_DEL, _event_fd, &evctl ))
            EQWARN << "epoll_ctl : " << base::sysError << std::endl;
    }

    if(( _epoll_fd >= 0 ) && TEMP_FAILURE_RETRY( ::close( _epoll_fd )))
        EQWARN << "close : " << base::sysError << std::endl;
    _epoll_fd = -1;

    if(( _event_fd >= 0 ) && TEMP_FAILURE_RETRY( ::close( _event_fd )))
        EQWARN << "close : " << base::sysError << std::endl;
    _event_fd = -1;
}

bool RDMAConnection::ChannelEventThread::init( )
{
    struct epoll_event evctl;

    _event_fd = ::eventfd( 0, 0 );
    if( _event_fd < 0 )
    {
        EQERROR << "eventfd : " << base::sysError << std::endl;
        goto err;
    }

    _epoll_fd = ::epoll_create1( 0 );
    if( _epoll_fd < 0 )
    {
        EQERROR << "epoll_create1 : " << base::sysError << std::endl;
        goto err;
    }

    ::memset( (void *)&evctl, 0, sizeof(struct epoll_event) );
    evctl.events = EPOLLIN;
    evctl.data.ptr = reinterpret_cast< void * >( &_context );
    if( ::epoll_ctl( _epoll_fd, EPOLL_CTL_ADD, _event_fd, &evctl ))
    {
        EQERROR << "epoll_ctl : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

void RDMAConnection::ChannelEventThread::run( )
{
    bool running = true;
    struct epoll_event event;
    int active = 0;

    do
    {
        struct epoll_context *context;

        int n = TEMP_FAILURE_RETRY( ::epoll_wait( _epoll_fd, &event, 1, -1 ));
        if( n < 0 )
        {
            EQERROR << "epoll_wait : " << base::sysError << std::endl;
            break;
        }

        EQASSERT( 1 == n );

        context = reinterpret_cast< struct epoll_context * >( event.data.ptr );
        if( EVENT_FD == context->type )
        {
            uint64_t one;
            struct epoll_event evctl;

            EQASSERT( context->ctx.thread == this );

            if( ::read( _event_fd, (void *)&one, sizeof(one) ) != sizeof(one) )
            {
                EQERROR << "read : " << base::sysError << std::endl;
                break;
            }

            EQASSERT( ONE == one );

            ::memset( (void *)&evctl, 0, sizeof(struct epoll_event) );
            if( NULL != _to_add )
            {
                EQASSERT( NULL == _to_remove );

                RDMAConnection *to_add = _to_add;
                _to_add = NULL;

                evctl.events = EPOLLIN /*| EPOLLONESHOT*/;
                evctl.data.ptr =
                    reinterpret_cast< void * >( &to_add->_context );
                if( ::epoll_ctl( _epoll_fd, EPOLL_CTL_ADD, to_add->_cm->fd,
                        &evctl ))
                {
                    EQERROR << "epoll_ctl : " << base::sysError << std::endl;
                    to_add->_cmd_block.set( CMD_FAIL );
                }
                else
                {
                    ++active;
                    EQVERB << "active connections : " << active << std::endl;
                    to_add->_cmd_block.set( CMD_DONE );
                }
            }
            else if( NULL != _to_remove )
            {
                EQASSERT( NULL == _to_add );

                RDMAConnection *to_remove = _to_remove;
                _to_remove = NULL;

                if( ::epoll_ctl( _epoll_fd, EPOLL_CTL_DEL, to_remove->_cm->fd,
                        &evctl ))
                {
                    EQWARN << "epoll_ctl : " << base::sysError << std::endl;
                    to_remove->_cmd_block.set( CMD_FAIL );
                }
                else
                {
                    --active;
                    EQVERB << "active connections : " << active << std::endl;
                    if( active == 0 )
                    {
                        to_remove->_cmd_block.set( CMD_DONE_LAST );
                        running = false;
                    }
                    else
                        to_remove->_cmd_block.set( CMD_DONE );
                }
            }
            else
                EQUNREACHABLE;
        }
        else if( CONNECTION_FD == context->type )
        {
            RDMAConnection *conn = context->ctx.connection;

            if( !conn->_doCMEvent( conn->_cm, RDMA_CM_EVENT_DISCONNECTED ))
                EQWARN << "Unexpected event on connection." << std::endl;
            // TODO : should we rdma_disconnect on *any* event?
            else if( ::rdma_disconnect( conn->_cm_id ))
                EQWARN << "rdma_disconnect : " << base::sysError << std::endl;
        }
        else
            EQUNREACHABLE;
    }
    while( running );
}

bool RDMAConnection::ChannelEventThread::_wake( )
{
    if( ::write( _event_fd, (const void *)&ONE, sizeof(ONE) ) != sizeof(ONE) )
    {
        EQERROR << "write : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::ChannelEventThread::add( RDMAConnection *conn )
{
    conn->_cmd_block.set( CMD_WAIT );

    EQASSERT( NULL == _to_add );
    _to_add = conn;

    if( !_wake( ) || ( CMD_DONE != conn->_cmd_block.waitNE( CMD_WAIT )))
    {
        EQERROR << "Event thread failed to add connection fd." << std::endl;
        goto err;
    }

    conn->_registered = true;

    return true;

err:
    return false;
}

bool RDMAConnection::ChannelEventThread::remove( RDMAConnection *conn )
{
    bool last = false;

    conn->_cmd_block.set( CMD_WAIT );

    EQASSERT( NULL == _to_remove );
    _to_remove = conn;

    if( _wake( ) && ( CMD_DONE & conn->_cmd_block.waitNE( CMD_WAIT )))
        last = ( conn->_cmd_block == CMD_DONE_LAST );
    else
        EQWARN << "Event thread failed to remove connection fd." << std::endl;

    conn->_registered = false;

    return last;
}

bool RDMAConnection::_eventThreadRegister( )
{
    base::ScopedMutex<> mutex( RDMAConnection::_thread_mutex );

    if( NULL == RDMAConnection::_event_thread )
    {
        RDMAConnection::_event_thread =
            new RDMAConnection::ChannelEventThread( );
        if( !RDMAConnection::_event_thread->start( ))
        {
            EQERROR << "Event thread failed to start." << std::endl;
            delete RDMAConnection::_event_thread;
            RDMAConnection::_event_thread = NULL;
            goto err;
        }
    }

    return RDMAConnection::_event_thread->add( this );

err:
    return false;
}

void RDMAConnection::_eventThreadUnregister( )
{
    base::ScopedMutex<> mutex( RDMAConnection::_thread_mutex );

    if( _registered )
    {
        EQASSERT( NULL != RDMAConnection::_event_thread );

        if( RDMAConnection::_event_thread->remove( this ))
        {
            RDMAConnection::_event_thread->join( );
            delete RDMAConnection::_event_thread;
            RDMAConnection::_event_thread = NULL;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void RDMAConnection::_showStats( )
{
    EQVERB << std::dec
        << "reads = " << _stats.reads
        << ", buffer_empty = " << _stats.buffer_empty
        << ", no_credits_fc = " << _stats.no_credits_fc
        << ", writes = " << _stats.writes
        << ", buffer_full = " << _stats.buffer_full
        << ", no_credits_rdma = " << _stats.no_credits_rdma
        << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

BufferPool::BufferPool( size_t buffer_size )
    : _buffer_size( buffer_size )
    , _num_bufs( 0 )
    , _buffer( NULL )
    , _mr( NULL )
    , _ring( 0 )
{
}

BufferPool::~BufferPool( )
{
    clear( );
}

void BufferPool::clear( )
{
    _num_bufs = 0;
    _ring.clear( _num_bufs );

    if(( NULL != _mr ) && ::rdma_seterrno( ::ibv_dereg_mr( _mr )))
        EQWARN << "ibv_dereg_mr : " << base::sysError << std::endl;
    _mr = NULL;

    if( NULL != _buffer )
        ::free( _buffer );
    _buffer = NULL;
}

bool BufferPool::resize( ibv_pd *pd, uint32_t num_bufs )
{
    clear( );

    if( num_bufs )
    {
        _num_bufs = num_bufs;
        _ring.clear( _num_bufs );

        if( ::posix_memalign( &_buffer, (size_t)::getpagesize( ),
                (size_t)( _num_bufs * _buffer_size )))
        {
            EQERROR << "posix_memalign : " << base::sysError << std::endl;
            goto err;
        }

        ::memset( _buffer, 0, (size_t)( _num_bufs * _buffer_size ));
        _mr = ::ibv_reg_mr( pd, _buffer, (size_t)( _num_bufs * _buffer_size ),
            IBV_ACCESS_LOCAL_WRITE );
        if( NULL == _mr )
        {
            EQERROR << "ibv_reg_mr : " << base::sysError << std::endl;
            goto err;
        }

        for( uint32_t i = 0; i != _num_bufs; i++ )
            _ring.put( i );
    }

    return true;

err:
    return false;
}

//////////////////////////////////////////////////////////////////////////////

RingBuffer::RingBuffer( int access )
    : _access( access )
    , _size( 0 )
    , _map( MAP_FAILED )
    , _mr( NULL )
{
}

RingBuffer::~RingBuffer( )
{
    clear( );
}

void RingBuffer::clear( )
{
    if(( NULL != _mr ) && ::rdma_seterrno( ::ibv_dereg_mr( _mr )))
        EQWARN << "ibv_dereg_mr : " << base::sysError << std::endl;
    _mr = NULL;

    if(( MAP_FAILED != _map ) && ::munmap( _map, _size << 1 ))
        EQWARN << "munmap : " << base::sysError << std::endl;
    _map = MAP_FAILED;

    _size = 0;
}

bool RingBuffer::resize( ibv_pd *pd, size_t size )
{
    bool ok = false;
    int fd = -1;

    clear( );

    if( size )
    {
        void *addr1, *addr2;
        char path[] = "/dev/shm/co-rdma-buffer-XXXXXX";

        _size = size;

        fd = ::mkstemp( path );
        if( fd < 0 )
        {
            EQERROR << "mkstemp : " << base::sysError << std::endl;
            goto out;
        }

        if( ::unlink( path ))
        {
            EQERROR << "unlink : " << base::sysError << std::endl;
            goto out;
        }

        if( ::ftruncate( fd, _size ))
        {
            EQERROR << "ftruncate : " << base::sysError << std::endl;
            goto out;
        }

        _map = ::mmap( NULL, _size << 1,
            PROT_NONE,
            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 );
        if( MAP_FAILED == _map )
        {
            EQERROR << "mmap : " << base::sysError << std::endl;
            goto out;
        }

        addr1 = ::mmap( _map, _size,
            PROT_READ | PROT_WRITE,
            MAP_FIXED | MAP_SHARED, fd, 0 );
        if( MAP_FAILED == addr1 )
        {
            EQERROR << "mmap : " << base::sysError << std::endl;
            goto out;
        }

        addr2 = ::mmap( (void *)( (uintptr_t)_map + _size ), _size,
            PROT_READ | PROT_WRITE,
            MAP_FIXED | MAP_SHARED, fd, 0 );
        if( MAP_FAILED == addr2 )
        {
            EQERROR << "mmap : " << base::sysError << std::endl;
            goto out;
        }

        _mr = ::ibv_reg_mr( pd, _map, _size << 1, _access );
        if( NULL == _mr )
        {
            EQERROR << "ibv_reg_mr : " << base::sysError << std::endl;
            goto out;
        }

        EQASSERT( addr1 == _map );
        EQASSERT( addr2 == (void *)( (uintptr_t)_map + _size ));

        ::memset( _map, 0, _size );
        *reinterpret_cast< uint8_t * >( _map ) = 0x45;
        EQASSERT( 0x45 ==
            *reinterpret_cast< uint8_t * >( (uintptr_t)_map + _size ));
    }

    ok = true;

out:
    if(( fd >= 0 ) && TEMP_FAILURE_RETRY( ::close( fd )))
        EQWARN << "close : " << base::sysError << std::endl;

    return ok;
}
} // namespace co
