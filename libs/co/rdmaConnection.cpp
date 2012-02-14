// -*- mode: c++ -*-
/* Copyright (c) 2011, Computer Integration & Programming Solutions, Corp. and
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
#include "rdmaConnection.h"

#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include "connectionType.h" // enum
#include "connectionDescription.h"
#include "global.h"

#include "base/scopedMutex.h"

#include <fcntl.h>
#include <sstream>
#include <limits>

#define BLOCKING_WRITE 0

namespace co
{

namespace
{
template< class T > inline std::string to_string( const T &t )
{
    std::stringstream ss; ss << t; return ss.str( );
}
}

/**
 * Set (blocking = false) or clear (blocking = true) the O_NONBLOCK flag on an
 * open file descriptor (fd)
 */
static bool setBlocking( int fd, bool blocking );

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
struct RDMAFCPayload {
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
        uint8_t offsetof_placeholder;
        struct RDMASetupPayload setup;
        struct RDMAFCPayload fc;
    };
};

/**
 * "IMM" data sent with RDMA write, tells sink about send progress
 */
typedef uint32_t RDMAFCImm;

/**
 * An RDMA connection implementation.
 */
RDMAConnection::RDMAConnection( )
    : _notifier( -1 )
    , _wfd( -1 )
    , _event_thread( NULL )
    , _efd( 0 )
    , _setup_block( SETUP_WAIT )
    , _cm( NULL )
    , _cm_id( NULL )
    , _established( false )
    , _thread_running( false )
    , _wcerr( false )
    , _depth( 256UL )
    , _pd( NULL )
    , _cc( NULL )
    , _cq( NULL )
    , _qp( NULL )
    , _completions( 0U )
    , _available_wr( 0 )
    , _msgbuf( sizeof(RDMAMessage) )
    , _sourcebuf( 0 )
    , _sourceptr( 0 )
    , _sinkbuf( IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE )
    , _sinkptr( 0 )
    , _rptr( 0UL )
    , _rbase( 0ULL )
    , _rkey( 0ULL )
{
    EQVERB << (void *)this << ".new" << std::endl;

    ::memset( (void *)&_conn_param, 0, sizeof(struct rdma_conn_param));
    ::memset( (void *)&_dev_attr, 0, sizeof(struct ibv_device_attr));

    _description->type = CONNECTIONTYPE_RDMA;
    _description->bandwidth =
        ::ibv_rate_to_mult( IBV_RATE_40_GBPS ) * 2.5 * 1000000 / 8;
}

bool RDMAConnection::connect( )
{
    struct sockaddr address;

    EQVERB << (void *)this << ".connect( )" << std::endl;

    EQASSERT( CONNECTIONTYPE_RDMA == _description->type );
    EQASSERT( STATE_CLOSED == _state );
    setState( STATE_CONNECTING );

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

    if( !_parseAddress( address, false ))
    {
        EQERROR << "Failed to parse destination address." << std::endl;
        goto err;
    }

    if( !_resolveAddress( address ))
    {
        EQERROR << "Failed to resolve destination address." << std::endl;
        goto err;
    }

    if( !_resolveRoute( ))
    {
        EQERROR << "Failed to resolve route to destination." << std::endl;
        goto err;
    }

    if( !_initVerbs( ))
    {
        EQERROR << "Failed to initialize verbs." << std::endl;
        goto err;
    }

    if( !_initBuffers( ))
    {
        EQERROR << "Failed to initialize ring buffers." << std::endl;
        goto err;
    }

    if( !_createQP( ))
    {
        EQERROR << "Failed to create queue pair." << std::endl;
        goto err;
    }

    if( !_postReceives( _qpcap.max_recv_wr ))
    {
        EQERROR << "Failed to pre-post receives." << std::endl;
        goto err;
    }

    if( !_connect( ))
    {
        EQERROR << "Failed to connect to destination." << std::endl;
        goto err;
    }

    if( !_startEventThread( ))
    {
        EQERROR << "Failed to start event thread." << std::endl;
        goto err;
    }

    if( !_postSendSetup( ))
    {
        EQERROR << "Failed to send setup message." << std::endl;
        goto err;
    }

    if( !_waitRecvSetup( ))
    {
        EQERROR << "Failed to receive setup message." << std::endl;
        goto err;
    }

    setState( STATE_CONNECTED );
    return true;

err:
    close( );
    return false;
}

bool RDMAConnection::listen( )
{
    struct sockaddr address;

    EQVERB << (void *)this << ".listen( )" << std::endl;

    EQASSERT( CONNECTIONTYPE_RDMA == _description->type );
    EQASSERT( STATE_CLOSED == _state );
    setState( STATE_CONNECTING );

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

    if( !_parseAddress( address, true ))
    {
        EQERROR << "Failed to parse local address." << std::endl;
        goto err;
    }

    if( !_bindAddress( address ))
    {
        EQERROR << "Failed to bind to local address." << std::endl;
        goto err;
    }

    if( !_listen( ))
    {
        EQERROR << "Failed to listen on bound address." << std::endl;
        goto err;
    }

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

        _drainEvents( );
        _cleanup( );

        setState( STATE_CLOSED );
    }
}

void RDMAConnection::acceptNB( ) { /* NOP */ }

ConnectionPtr RDMAConnection::acceptSync( )
{
    EQVERB << (void *)this << ".acceptSync( )" << std::endl;

    EQASSERT( STATE_LISTENING == _state );

    RDMAConnection *newConnection = new RDMAConnection( );

    newConnection->setDescription( _description );

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
    //EQWARN << (void *)this << ".read(" << bytes << ")" <<
    //   " <<<<<<<<<<---------- " << std::endl;

    uint64_t available_bytes;
    EQASSERT( 0 <= _notifier );
    // TODO : Timeout?
    while( 0 > ::read( _notifier, (void *)&available_bytes, sizeof(uint64_t)))
    {
        EQASSERT( EAGAIN == errno );
        if( !_thread_running )
        {
            EQINFO << "Got EOF, closing connection." << std::endl;
            close( );
            return -1LL;
        }
        co::base::Thread::yield( );
        continue;
    }

    if( !_thread_running )
    {
        // Discard extra "disconnected" byte sent by event thread.
        if( available_bytes > 1ULL )
            available_bytes--;

        // Don't close( ) here as we're not necessarily returning
        // an error right now (might be returning remaining data)
        // and don't want to change state yet.  We do want to drain
        // the completion queue, however.
        _drainEvents( );
    }

    const uint32_t bytes_taken = _drain( buffer,
        static_cast< uint32_t >( std::min( bytes, available_bytes )));

    EQASSERTINFO( bytes_taken <= available_bytes,
        bytes_taken << " > " << available_bytes );

    if(( 1ULL == available_bytes ) && ( 0UL == bytes_taken ) &&
        !_thread_running )
    {
        EQINFO << "Got EOF, closing connection." << std::endl;
        close( );
        return -1LL;
    }

    // Put back what wasn't taken
    if( available_bytes > bytes_taken )
        _notify( available_bytes - bytes_taken );

    if( bytes_taken > 0UL )
    {
        // TODO : Timeout?
        while( _thread_running && ( 0 == _available_wr ))
            co::base::Thread::yield( );

        // TODO : Send FC less frequently?
        if( !_postSendFC( ))
            EQWARN << "Failed to send flow control message." << std::endl;
    }

    //EQWARN << (void *)this << ".read(" << bytes << ")" <<
    //   " <<<<<<<<<<========== took " << bytes_taken << " bytes" << std::endl;

    return static_cast< int64_t >( bytes_taken );
}

int64_t RDMAConnection::write( const void* buffer, const uint64_t bytes )
{
    if( STATE_CONNECTED != _state )
        return -1LL;

    //EQWARN << (void *)this << ".write(" << bytes << ")" <<
    //    " ---------->>>>>>>>>>" << std::endl;

    uint32_t bytes_put;
    // TODO : Timeout?
    while( _thread_running &&
        ( 0UL == ( bytes_put =
            _fill( buffer, static_cast< uint32_t >( bytes )))))
        co::base::Thread::yield( );

    // TODO : Timeout?
    while( _thread_running && ( 0 == _available_wr ))
        co::base::Thread::yield( );

    if( _thread_running && _postRDMAWrite( ))
    {
        uint64_t completed_bytes = 0ULL;
        EQASSERT( 0 <= _wfd );
        while( 0 > ::read( _wfd, (void *)&completed_bytes, sizeof(uint64_t)))
        {
            EQASSERT( EAGAIN == errno );
            if( !_thread_running || _wcerr )
            {
                close( );
                return -1LL;
            }
#if BLOCKING_WRITE
            co::base::Thread::yield( );
            continue;
#else
            break;
#endif
        }
    }
    else
    {
        close( );
        return -1LL;
    }

    //EQWARN << (void *)this << ".write(" << bytes << ")" <<
    //   " ==========>>>>>>>>>> put " << bytes_put << " bytes" << std::endl;

    return static_cast< int64_t >( bytes_put );
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

void RDMAConnection::_drainEvents( )
{
    if( _established )
    {
        bool warned = false;

        // Wait for event thread to process outstanding work requests, as long
        // as its still running.  We'll finish them later while draining if its
        // not.  TODO : Timeout?
        while( _thread_running && ( _available_wr < (int)_qpcap.max_send_wr ))
        {
            if( !warned )
            {
                EQINFO << "Still waiting for outstanding work requests ( "
                    << _available_wr << " < " << _qpcap.max_send_wr
                    << " )" << std::endl;
                warned = true;
            }
            co::base::Thread::yield( );
        }

        EQASSERT( NULL != _cm_id );
        EQASSERT( NULL != _cm_id->verbs );

        if( 0 != ::rdma_disconnect( _cm_id ))
            EQWARN << "rdma_disconnect : " << base::sysError << std::endl;
        else
            EQWARN << "rdma_disconnect!" << std::endl;
    }

    _joinEventThread( );

    EQASSERT( !_thread_running );

    if(( NULL != _cc ) && _established )
    {
        bool warned = false;

        // Drain completion queue ourselves as the event thread has exited.
        while( _doCQEvents( _cc, true ))
        {
            if( !warned )
            {
                EQWARN << "Still draining completion queue." << std::endl;
                warned = true;
            }
            co::base::Thread::yield( );
        }
    }

    _established = false;
}

void RDMAConnection::_cleanup( )
{
    EQASSERT( STATE_CLOSING == _state );
    EQASSERT( NULL == _event_thread );

    _sourcebuf.clear( );
    _sinkbuf.clear( );
    _msgbuf.clear( );

    if(( 0 <= _notifier ) && ( _cm->fd != _notifier ) &&
        ( 0 != ::close( _notifier )))
        EQWARN << "close : " << base::sysError << std::endl;
    _notifier = -1;

    if(( 0 <= _wfd ) && ( 0 != ::close( _wfd )))
        EQWARN << "close : " << base::sysError << std::endl;
    _wfd = -1;

    if(( NULL != _qp ) && ( 0 != ::ibv_destroy_qp( _qp )))
        EQWARN << "ibv_destroy_qp : " << base::sysError << std::endl;
    _qp = NULL;

    if( 0U < _completions )
    {
        ::ibv_ack_cq_events( _cq, _completions );
        _completions = 0U;
    }

    if(( NULL != _cq ) && ( 0 != ::ibv_destroy_cq( _cq )))
        EQWARN << "ibv_destroy_cq : " << base::sysError << std::endl;
    _cq = NULL;

    if(( NULL != _cc ) && ( 0 != ::ibv_destroy_comp_channel( _cc )))
        EQWARN << "ibv_destroy_comp_channel : " << base::sysError << std::endl;
    _cc = NULL;

    if(( NULL != _pd ) && ( 0 != ::ibv_dealloc_pd( _pd )))
        EQWARN << "ibv_dealloc_pd : " << base::sysError << std::endl;
    _pd = NULL;

    if(( NULL != _cm_id ) && ( 0 != ::rdma_destroy_id( _cm_id )))
        EQWARN << "rdma_destroy_id : " << base::sysError << std::endl;
    _cm_id = NULL;

    if( NULL != _cm )
        ::rdma_destroy_event_channel( _cm );
    _cm = NULL;
}

bool RDMAConnection::_finishAccept( struct rdma_event_channel *listen_channel )
{
    EQASSERT( STATE_CLOSED == _state );
    setState( STATE_CONNECTING );

    if( !_createEventChannel( ))
    {
        EQERROR << "Failed to create event channel." << std::endl;
        goto err;
    }

    if( !_doCMEvent( listen_channel, RDMA_CM_EVENT_CONNECT_REQUEST ))
    {
        EQERROR << "Failed to receive valid connect request." << std::endl;
        goto err;
    }

    if( !_migrateId( ))
    {
        EQERROR << "Failed to migrate communication identifier." << std::endl;
        goto err;
    }

    if( !_initVerbs( ))
    {
        EQERROR << "Failed to initialize verbs." << std::endl;
        goto err;
    }

    if( !_initBuffers( ))
    {
        EQERROR << "Failed to initialize ring buffers." << std::endl;
        goto err;
    }

    if( !_createQP( ))
    {
        EQERROR << "Failed to create queue pair." << std::endl;
        goto err;
    }

    if( !_postReceives( _qpcap.max_recv_wr ))
    {
        EQERROR << "Failed to pre-post receives." << std::endl;
        goto err;
    }

    if( !_accept( ))
    {
        EQERROR << "Failed to accept initiated connection." << std::endl;
        goto err;
    }

    if( !_startEventThread( ))
    {
        EQERROR << "Failed to start event thread." << std::endl;
        goto err;
    }

    if( !_postSendSetup( ))
    {
        EQERROR << "Failed to send setup message." << std::endl;
        goto err;
    }

    if( !_waitRecvSetup( ))
    {
        EQERROR << "Failed to receive setup message." << std::endl;
        goto err;
    }

    setState( STATE_CONNECTED );
    return true;

err:
    close( );
    return false;
}

bool RDMAConnection::_parseAddress( struct sockaddr &address,
    const bool passive ) const
{
    bool ok = false;
    const char *node = NULL, *service = NULL;
    struct addrinfo hints, *res = NULL;

    ::memset( (void *)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

    const std::string &hostname = _description->getHostname( );
    if( !hostname.empty( ))
        node = hostname.c_str( );
    else if( passive )
        hints.ai_flags |= AI_PASSIVE;
    const std::string port = to_string<uint16_t>( _description->port );
    if( 0u != _description->port )
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

bool RDMAConnection::_createEventChannel( )
{
    EQASSERT( NULL == _cm );

    if( NULL == ( _cm = ::rdma_create_event_channel( )))
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

    if( 0 != ::rdma_create_id( _cm, &_cm_id, NULL, RDMA_PS_TCP ))
    {
        EQERROR << "rdma_create_id : " << base::sysError << std::endl;
        goto err;
    }
    return true;

err:
    return false;
}

bool RDMAConnection::_resolveAddress( struct sockaddr &address )
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_resolve_addr( _cm_id, NULL, &address,
        Global::getIAttribute( Global::IATTR_RDMA_RESOLVE_TIMEOUT_MS )))
    {
        EQERROR << "rdma_resolve_addr : " << base::sysError << std::endl;
        goto err;
    }
    // block for RDMA_CM_EVENT_ADDR_RESOLVED
    return _doCMEvent( _cm, RDMA_CM_EVENT_ADDR_RESOLVED );

err:
    return false;
}

bool RDMAConnection::_resolveRoute( )
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_resolve_route( _cm_id,
        Global::getIAttribute( Global::IATTR_RDMA_RESOLVE_TIMEOUT_MS )))
    {
        EQERROR << "rdma_resolve_route : " << base::sysError << std::endl;
        goto err;
    }
    // block for RDMA_CM_EVENT_ROUTE_RESOLVED
    return _doCMEvent( _cm, RDMA_CM_EVENT_ROUTE_RESOLVED );

err:
    return false;
}

bool RDMAConnection::_connect( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( !_established );

    const uint32_t depth = _qpcap.max_recv_wr;
    _conn_param.private_data = reinterpret_cast< const void * >( &depth );
    _conn_param.private_data_len = sizeof(uint32_t);
    _conn_param.responder_resources = _dev_attr.max_qp_rd_atom;
    _conn_param.initiator_depth = _dev_attr.max_qp_init_rd_atom;
    _conn_param.retry_count = 7;
    _conn_param.rnr_retry_count = 7;

    EQINFO << "Connect on" << std::showbase <<
        " source lid : " <<
            std::hex << ntohs( _cm_id->route.path_rec->slid ) << " (" 
            <<
            std::dec << ntohs( _cm_id->route.path_rec->slid ) << ") "
        "to" <<
        " dest lid : " <<
            std::hex << ntohs( _cm_id->route.path_rec->dlid ) << " (" 
            <<
            std::dec << ntohs( _cm_id->route.path_rec->dlid ) << ") "
        << std::endl;

    if( 0 != ::rdma_connect( _cm_id, &_conn_param ))
    {
        EQERROR << "rdma_connect : " << base::sysError << std::endl;
        goto err;
    }
    // block for RDMA_CM_EVENT_ESTABLISHED
    return _doCMEvent( _cm, RDMA_CM_EVENT_ESTABLISHED );

err:
    return false;
}

bool RDMAConnection::_bindAddress( struct sockaddr &address ) const
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_bind_addr( _cm_id, &address ))
    {
        EQERROR << "rdma_bind_addr : " << base::sysError << std::endl;
        goto err;
    }
    return true;

err:
    return false;
}

bool RDMAConnection::_listen( ) const
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_listen( _cm_id, SOMAXCONN ))
    {
        EQERROR << "rdma_listen : " << base::sysError << std::endl;
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

    // _conn_param holds the initiator's parameters at this point, acquired
    // in _doCMEvent when event == RDMA_CM_EVENT_CONNECT_REQUEST.  We accept
    // with the minimum of the initiator's and our own values.
    _conn_param.responder_resources =
        std::min( static_cast< int >( _conn_param.responder_resources ),
            _dev_attr.max_qp_rd_atom );
    _conn_param.initiator_depth =
        std::min( static_cast< int >( _conn_param.initiator_depth ),
            _dev_attr.max_qp_init_rd_atom );

    EQINFO << "Accept on" << std::showbase <<
        " source lid : " <<
            std::hex << ntohs( _cm_id->route.path_rec->slid ) << " (" 
            <<
            std::dec << ntohs( _cm_id->route.path_rec->slid ) << ") "
        "from" <<
        " dest lid : " <<
            std::hex << ntohs( _cm_id->route.path_rec->dlid ) << " (" 
            <<
            std::dec << ntohs( _cm_id->route.path_rec->dlid ) << ") "
        << std::endl;

    if( 0 != ::rdma_accept( _cm_id, &_conn_param ))
    {
        EQERROR << "rdma_accept : " << base::sysError << std::endl;
        goto err;
    }
    // block for RDMA_CM_EVENT_ESTABLISHED
    return _doCMEvent( _cm, RDMA_CM_EVENT_ESTABLISHED );

err:
    return false;
}

bool RDMAConnection::_migrateId( ) const
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( NULL != _cm );

    if( 0 != ::rdma_migrate_id( _cm_id, _cm ))
    {
        EQERROR << "rdma_migrate_id : " << base::sysError << std::endl;
        goto err;
    }
    return true;

err:
    return false;
}

bool RDMAConnection::_initVerbs( )
{
    EQASSERT( STATE_CONNECTING == _state );
    EQASSERT( NULL != _cm_id );
    EQASSERT( NULL != _cm_id->verbs );
    EQASSERT( NULL != _cm_id->verbs->device );

    EQINFO << "Infiniband device : " << _cm_id->verbs->device->name <<
        std::endl;

    if( 0 != ::ibv_query_device( _cm_id->verbs, &_dev_attr ))
    {
        EQERROR << "ibv_query_device : " << base::sysError << std::endl;
        goto err;
    }

    _pd = ::ibv_alloc_pd( _cm_id->verbs );
    if( NULL == _pd )
    {
        EQERROR << "ibv_alloc_pd : " << base::sysError << std::endl;
        goto err;
    }

    _cc = ::ibv_create_comp_channel( _cm_id->verbs );
    if( NULL == _cc )
    {
        EQERROR << "ibv_create_comp_channel : " << base::sysError << std::endl;
        goto err;
    }

    _cq = ::ibv_create_cq( _cm_id->verbs, _depth * 2, NULL, _cc, 0 );
    if( NULL == _cq )
    {
        EQERROR << "ibv_create_cq : " << base::sysError << std::endl;
        goto err;
    }

    if( 0 != ::ibv_req_notify_cq( _cq, 0 ))
    {
        EQERROR << "ibv_req_notify_cq : " << base::sysError << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

bool RDMAConnection::_initBuffers( )
{
    EQASSERT( NULL != _pd );

    const uint32_t rbs = 1024UL * 1024UL *
        Global::getIAttribute( Global::IATTR_RDMA_RING_BUFFER_SIZE_MB );

    if( 0UL == rbs )
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

bool RDMAConnection::_createQP( )
{
    EQASSERT( NULL != _pd );
    EQASSERT( NULL != _cq );

    struct ibv_qp_init_attr init_attr;
    ::memset( &init_attr, 0, sizeof(struct ibv_qp_init_attr));

    init_attr.cap.max_send_wr = _depth;
    init_attr.cap.max_recv_wr = _depth;
    init_attr.cap.max_recv_sge = 1;
    init_attr.cap.max_send_sge = 1;
    init_attr.qp_type = IBV_QPT_RC;
    init_attr.send_cq = _cq;
    init_attr.recv_cq = _cq; 

    if( 0 != ::rdma_create_qp( _cm_id, _pd, &init_attr ))
    {
        EQERROR << "rdma_create_qp : " << base::sysError << std::endl;
        goto err;
    }

    _qp = _cm_id->qp;
    _qpcap = init_attr.cap;
    _depth = _qpcap.max_recv_wr;
    _available_wr = _qpcap.max_send_wr;

    EQINFO << "Infiniband QP caps : " <<
        _qpcap.max_recv_wr << " receives, " <<
        _qpcap.max_send_wr << " sends." << std::endl;

    return _msgbuf.resize( _pd, _qpcap.max_send_wr * 2 + _qpcap.max_recv_wr );

err:
    return false;
}

// caller: application before connect/accept (AKA pre-posting receives),
// event thread otherwise
bool RDMAConnection::_postReceives( const unsigned int count )
{
    bool ok = false;

    EQASSERT( NULL != _qp );
    EQASSERT( count <= _qpcap.max_recv_wr );

    if( 0U == count )
    {
        ok = true;
        goto out;
    }

    struct ibv_sge sge[count];
    ::memset( &sge, 0, count * sizeof(struct ibv_sge));
    for( unsigned int i = 0U; i != count; i++ )
    {
        sge[i].addr = (uint64_t)(uintptr_t)_msgbuf.getBuffer( );
        sge[i].length = (uint64_t)_msgbuf.getBufferSize( );
        sge[i].lkey = _msgbuf.getMR( )->lkey;
    }

    struct ibv_recv_wr wrs[count];
    ::memset( &wrs, 0, count * sizeof(struct ibv_recv_wr));
    for( unsigned int i = 0U; i != count; i++ )
    {
        wrs[i].wr_id = sge[i].addr;
        wrs[i].next = &wrs[i + 1];
        wrs[i].sg_list = &sge[i];
        wrs[i].num_sge = 1;
    }
    wrs[count - 1].next = NULL;

    struct ibv_recv_wr *bad_wr;
    if( 0 != ::ibv_post_recv( _qp, wrs, &bad_wr ))
    {
        EQERROR << "ibv_post_recv : "  << base::sysError << std::endl;
        goto out;
    }

    ok = true;

out:
    return ok;
}

// caller: event thread
void RDMAConnection::_handleSetup( RDMASetupPayload &setup )
{
    _rbase = setup.rbase;
    _rptr.clear( setup.rlen );
    _rkey = setup.rkey;

    _setup_block.set( SETUP_OK );
}

// caller: event thread
void RDMAConnection::_handleFC( RDMAFCPayload &fc )
{
    _rptr.moveValue( _rptr.TAIL, ntohl( fc.ringTail ));
}

// caller: event thread
void RDMAConnection::_handleMessage( RDMAMessage &message )
{
    switch( message.opcode )
    {
        case SETUP:
            _handleSetup( message.setup );
            break;
        case FC:
            _handleFC( message.fc );
            break;
    }
}

// caller: event thread
void RDMAConnection::_handleImm( const uint32_t imm )
{
    RDMAFCImm fc = ntohl( imm );

    _sinkptr.incrHead( fc );
    _notify( fc );
}

// caller: application
bool RDMAConnection::_postSendWR( struct ibv_send_wr &wr )
{
    EQASSERT( NULL != _qp );

    struct ibv_send_wr *bad_wr;
    if( 0 != ::ibv_post_send( _qp, &wr, &bad_wr ))
    {
        EQERROR << "ibv_post_send : "  << base::sysError << std::endl;
        goto err;
    }

    // Track available work requests
#ifdef EQ_RELEASE_ASSERT
    EQCHECK( --_available_wr >= 0 );
#else
    --_available_wr;
#endif

    return true;

err:
    return false;
}

// caller: application
bool RDMAConnection::_postSendMessage( RDMAMessage &message )
{
    struct ibv_sge sge; 
    ::memset( (void *)&sge, 0, sizeof(struct ibv_sge));
    sge.addr = (uint64_t)&message;
    sge.length = (uint64_t)( offsetof( RDMAMessage, offsetof_placeholder ) +
        message.length );
    sge.lkey = _msgbuf.getMR( )->lkey;

    struct ibv_send_wr wr;
    ::memset( (void *)&wr, 0, sizeof(struct ibv_send_wr));
    wr.wr_id = sge.addr; // Carry the &message so we can free it on completion
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.opcode = IBV_WR_SEND;

    return _postSendWR( wr );
}

// caller: application
void RDMAConnection::_fillSetup( RDMASetupPayload &setup ) const
{
    setup.rbase = (uint64_t)(uintptr_t)_sinkbuf.getBase( );
    setup.rlen = (uint64_t)_sinkbuf.getSize( );
    setup.rkey = _sinkbuf.getMR( )->rkey;
}

// caller: application
bool RDMAConnection::_postSendSetup( )
{
    RDMAMessage &message =
        *reinterpret_cast< RDMAMessage * >( _msgbuf.getBuffer( ));
    message.opcode = SETUP;
    message.length = sizeof(struct RDMASetupPayload);
    _fillSetup( message.setup );

    return _postSendMessage( message );
}

// caller: application
void RDMAConnection::_fillFC( RDMAFCPayload &fc ) const
{
    fc.ringTail = htonl( _sinkptr.value( _sinkptr.TAIL ));
}

// caller: application
bool RDMAConnection::_postSendFC( )
{
    RDMAMessage &message =
        *reinterpret_cast< RDMAMessage * >( _msgbuf.getBuffer( ));
    message.opcode = FC;
    message.length = sizeof(struct RDMAFCPayload);
    _fillFC( message.fc );

    return _postSendMessage( message );
}

// caller: application
bool RDMAConnection::_postRDMAWrite( )
{
    EQASSERT( NULL != _qp );

    // TODO : Break up large messages into multiple WR?

    struct ibv_sge sge; 
    ::memset( (void *)&sge, 0, sizeof(struct ibv_sge));
    sge.addr = (uint64_t)( (uintptr_t)_sourcebuf.getBase( ) +
        _sourceptr.ptr( _sourceptr.MIDDLE ));
    sge.length =
        (uint64_t)_sourceptr.available( _sourceptr.HEAD, _sourceptr.MIDDLE );
    sge.lkey = _sourcebuf.getMR( )->lkey;
    _sourceptr.incr( _sourceptr.MIDDLE, (uint32_t)sge.length );

    struct ibv_send_wr wr;
    ::memset( (void *)&wr, 0, sizeof(struct ibv_send_wr));
    wr.wr_id = (uint64_t)_sourceptr.value( _sourceptr.MIDDLE );
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
    wr.imm_data = htonl( static_cast< uint32_t >( sge.length ));
    wr.wr.rdma.rkey = _rkey;
    wr.wr.rdma.remote_addr = (uint64_t)( (uintptr_t)_rbase +
        _rptr.ptr( _rptr.HEAD ));
    _rptr.incrHead( (uint32_t)sge.length );

    return _postSendWR( wr );
}

// caller: application
bool RDMAConnection::_waitRecvSetup( ) const
{
    return( _setup_block.timedWaitEQ( SETUP_OK, Global::getTimeout( )));
}

// caller: application if listener or during connect/accept,
// event thread if connected
bool RDMAConnection::_doCMEvent( struct rdma_event_channel *channel,
    rdma_cm_event_type expected )
{
    bool ok = false;
    struct rdma_cm_event *event;

    if( 0 > ::rdma_get_cm_event( channel, &event ))
    {
        // Channel is non-blocking & no events are available
        if( EAGAIN == errno )
        {
            ok = true;
            goto out;
        }
        EQERROR << "rdma_get_cm_event(" << errno << ") : " << base::sysError <<
            std::endl;
        goto out;
    }

    ok = ( event->event == expected );

    EQINFO << (void *)this << " : " << ::rdma_event_str( event->event )
        << "( " << ( !ok ? "*not* " : "" ) << "expected )" << std::endl;

    // Special case, flag that its safe to call rdma_disconnect
    if( ok && ( RDMA_CM_EVENT_ESTABLISHED == event->event ))
        _established = true;

    // Special case, extract connection params from event
    if( ok && ( RDMA_CM_EVENT_CONNECT_REQUEST == event->event ))
    {
        // TODO : Reject "bad" connect requests?
        // e.g. also require a magic value (i.e. password) in private_data?
        _cm_id = event->id;
        _conn_param = event->param.conn;
        // Note that the actual amount of data transferred to the
        // remote side is transport dependent and may be larger
        // than that requested.  TODO : probably shouldn't assert
        // here, instead reject.
        EQASSERT( sizeof(uint32_t) <= _conn_param.private_data_len );
        _depth =
            *reinterpret_cast< const uint32_t * >( _conn_param.private_data );

        // Won't be valid after ack'ing the event
        _conn_param.private_data = NULL;
        _conn_param.private_data_len = 0;
    }

    // Special case, log reject reason
    if( RDMA_CM_EVENT_REJECTED == event->event )
        EQWARN << "Connection rejected, status " << event->status <<
            std::endl;

    if( 0 != ::rdma_ack_cm_event( event ))
        EQWARN << "rdma_ack_cm_event : "  << base::sysError << std::endl;

out:
    return ok;
}

// caller: event thread while connected, application on cleanup
bool RDMAConnection::_doCQEvents( struct ibv_comp_channel *channel, bool drain )
{
    bool ok = false;
    struct ibv_cq *ev_cq;
    void *ev_ctx;
    struct ibv_wc wcs[_depth * 2];
    unsigned int num_recvs = 0U;
    bool success = true;
    int count;

    if( 0 > ::ibv_get_cq_event( channel, &ev_cq, &ev_ctx ))
    {
        // Channel is non-blocking & no events are available
        if( EAGAIN == errno )
        {
            ok = !drain;
            goto out;
        }
        EQERROR << "ibv_get_cq_event(" << errno << ") : " << base::sysError <<
            std::endl;
        goto out;
    }

    EQASSERT( ev_cq == _cq );

    // We just keep track of completions and periodically ack to avoid
    // overflow of the counter which we will use in _cleanup so we don't
    // hang on ibv_destroy_cq since "destroying a CQ will wait for all
    // completion events to be acknowledged" (per IBV_GET_CQ_EVENT(3)).
    //
    // Also see: http://tinyurl.com/3rvuxjh
    _completions++;
    if( std::numeric_limits< unsigned int >::max( ) <= _completions )
    {
        ::ibv_ack_cq_events( ev_cq, _completions );
        _completions = 0U;
    }

    if( 0 != ::ibv_req_notify_cq( ev_cq, 0 ))
    {
        EQERROR << "ibv_req_notify_cq : " << base::sysError << std::endl;
        goto out;
    }

    count = ::ibv_poll_cq( ev_cq, sizeof(wcs) /sizeof(wcs[0]), wcs );
    if( 0 > count )
    {
        EQERROR << "ibv_poll_cq : " << base::sysError << std::endl;
        goto out;
    }

    for( int i = 0; ( i != count ) && success; i++ )
    {
        struct ibv_wc &wc = wcs[i];

        if( IBV_WC_SUCCESS != wc.status )
        {
            _wcerr = true;

            // Wake up any blocking write
            if( IBV_WC_RDMA_WRITE == wc.opcode )
                _complete( 1UL );

            // Ignore flush errors while closing
            if(( IBV_WC_WR_FLUSH_ERR == wc.status ) &&
                (( STATE_CLOSING == _state ) || drain ))
                continue;

            EQERROR << (void *)this << " !IBV_WC_SUCCESS : " <<
                "wr_id = " << std::showbase << std::hex <<
                (unsigned int)wc.wr_id << std::dec <<
                ", opcode = " << (unsigned int)wc.opcode <<
                ", status = " << (unsigned int)wc.status <<
                " (" << ::ibv_wc_status_str( wc.status ) << ")" <<
                ", vendor_err = " << std::hex << wc.vendor_err <<
                std::dec << std::endl;

            success = false;
        }
        else
        {
            // Send/write completions
            if(( IBV_WC_SEND == wc.opcode ) ||
                ( IBV_WC_RDMA_WRITE == wc.opcode ))
            {
                // Track available work requests
#ifdef EQ_RELEASE_ASSERT
                EQCHECK( ++_available_wr <= int(_qpcap.max_send_wr));
#else
                ++_available_wr;
#endif
                if( IBV_WC_RDMA_WRITE == wc.opcode )
                {
                    const uint32_t bytes_written =
                        _sourceptr.available( _sourceptr.MIDDLE,
                            _sourceptr.TAIL );
                      
                    _sourceptr.moveValue( _sourceptr.TAIL,
                        static_cast< uint32_t >( wc.wr_id ));

                    // Not a message buffer, don't free
                    wc.wr_id = 0ULL;

                    // Wake up any blocking write
                    _complete( bytes_written );
                }
            }
            // Receive completions
            if( IBV_WC_RECV == wc.opcode )
                _handleMessage( *reinterpret_cast< RDMAMessage * >( wc.wr_id ));
            if( IBV_WC_RECV_RDMA_WITH_IMM == wc.opcode )
                _handleImm( wc.imm_data );

            // All receives need to be re-posted
            if( IBV_WC_RECV & wc.opcode )
                num_recvs++;

            // Release message buffers back to the pool
            if( 0ULL != wc.wr_id )
                _msgbuf.freeBuffer( (void *)(uintptr_t)wc.wr_id );
        }
    }

    if( success && _thread_running )
        ok = _postReceives( num_recvs );

out:
    return ok;
}

#ifdef EQ_GCC_4_5_OR_LATER
#  pragma GCC diagnostic ignored "-Wunused-result"
#endif

void RDMAConnection::_eventFDWrite( int fd, const uint64_t val ) const
{
    EQASSERT( 0 <= fd );

#ifdef EQ_RELEASE_ASSERT
    EQCHECK( ::write( fd, (const void *)&val, sizeof( val )) == sizeof( val ));
#else
    ::write( fd, (const void *)&val, sizeof( val ));
#endif
}

// caller: application & event thread
void RDMAConnection::_notify( const uint64_t val ) const
{
    EQASSERT( 0 <= _notifier );
    _eventFDWrite( _notifier, val );
}

// caller: event thread
void RDMAConnection::_complete( const uint64_t val ) const
{
    EQASSERT( 0 <= _wfd );
    _eventFDWrite( _wfd, val );
}

// caller: application
bool RDMAConnection::_startEventThread( )
{
    EQASSERT( -1 == _notifier );
    EQASSERT( -1 == _wfd );
    EQASSERT( NULL == _event_thread );

    // For a connected instance we need to multiplex both the connection
    // manager fd and the completion channel fd to any polling operation,
    // we do that by epoll'ing those fds in our own thread and passing along
    // "events" via an eventfd.
    _notifier = ::eventfd( 0, EFD_NONBLOCK );
    if( 0 > _notifier )
    {
        EQERROR << "eventfd : " << base::sysError << std::endl;
        goto err;
    }

    _wfd = ::eventfd( 0, EFD_NONBLOCK );
    if( 0 > _wfd )
    {
        EQERROR << "eventfd : " << base::sysError << std::endl;
        goto err;
    }

    _event_thread = new ChannelEventThread( this );
    if( !_event_thread->start( ))
    {
        EQERROR << "Event thread failed to start!" << std::endl;
        goto err;
    }

    return true;

err:
    return false;
}

// caller: event thread
bool RDMAConnection::_initEventThread( )
{
    if( !setBlocking( _cm->fd, false ))
    {
        EQERROR << "Failed to unblock connection manager fd." << std::endl;
        goto err;
    }

    if( !setBlocking( _cc->fd, false ))
    {
        EQERROR << "Failed to unblock completion queue fd." << std::endl;
        goto err;
    }

    _efd = ::epoll_create( 2 );
    if( 0 > _efd )
    {
        EQERROR << "epoll_create : " << base::sysError << std::endl;
        goto err;
    }

    _thread_running = true;
    return true;

err:
    _setup_block.set( SETUP_NOK );
    return false;
}

// caller: event thread
void RDMAConnection::_runEventThread( )
{
    bool ok = false;
    enum { CM_EVENT = 0, CQ_EVENT = 1 };
    unsigned int cm_event = CM_EVENT, cq_event = CQ_EVENT;
    struct epoll_event evctl[2] =
    {
        { EPOLLIN | EPOLLET, { reinterpret_cast< void * >( &cm_event ) }},
        { EPOLLIN | EPOLLET, { reinterpret_cast< void * >( &cq_event ) }}
    };
    struct epoll_event events[sizeof(evctl) / sizeof(evctl[0])];

    if( 0 != ::epoll_ctl( _efd, EPOLL_CTL_ADD, _cm->fd, &evctl[CM_EVENT] ))
    {
        EQERROR << "epoll_ctl : " << base::sysError << std::endl;
        goto out;
    }
    if( 0 != ::epoll_ctl( _efd, EPOLL_CTL_ADD, _cc->fd, &evctl[CQ_EVENT] ))
    {
        EQERROR << "epoll_ctl : " << base::sysError << std::endl;
        goto out;
    }

    ok = true;
    do
    {
        const int nfds = ::epoll_wait( _efd, events,
            sizeof(evctl) / sizeof(evctl[0]), -1 );
        if( 0 > nfds )
        {
            if( EINTR == errno )
                continue;
            EQERROR << "epoll_wait : " << base::sysError << std::endl;
            ok = false;
        }
        else
        {
            for( int n = 0 ; ( n != nfds ) && ok; n++ )
            {
                const unsigned int event =
                    *reinterpret_cast< unsigned int * >( events[n].data.ptr );

                if( cq_event == event )
                    ok = _doCQEvents( _cc );
                else if( cm_event == event )
                    ok = !_doCMEvent( _cm, RDMA_CM_EVENT_DISCONNECTED );
            }
        }
    }
    while( ok );

out:
    _thread_running = false;
    _notify( 1ULL );

    if( 0 != ::close( _efd ))
        EQWARN << "close : " << base::sysError << std::endl;
}

// caller: application
void RDMAConnection::_joinEventThread( )
{
    if( NULL != _event_thread )
    {
        _event_thread->join( );
        delete _event_thread;
        _event_thread = NULL;
    }
}

// caller: application
uint32_t RDMAConnection::_drain( void *buffer, const uint32_t bytes )
{
    const uint32_t b = std::min( bytes, _sinkptr.available( ));
    ::memcpy( buffer, (const void *)((uintptr_t)_sinkbuf.getBase( ) +
        _sinkptr.tail( )), b );
    _sinkptr.incrTail( b );
    return b;
}

// caller: application
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

BufferPool::BufferPool( unsigned int buffer_size )
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

    if(( NULL != _mr ) && ( 0 != ::ibv_dereg_mr( _mr )))
        EQWARN << "ibv_dereg_mr : " << base::sysError << std::endl;
    _mr = NULL;

    if( NULL != _buffer )
        ::free( _buffer );
    _buffer = NULL;
}

bool BufferPool::resize( ibv_pd *pd, const unsigned int num_bufs )
{
    bool ok = false;

    clear( );

    if( 0U == num_bufs )
    {
        ok = true;
        goto out;
    }

    _num_bufs = num_bufs;
    _ring.clear( _num_bufs );

    _buffer = ::calloc( _num_bufs, _buffer_size );
    if( NULL == _buffer )
    {
        EQERROR << "calloc : " << base::sysError << std::endl;
        goto out;
    }

    _mr = ::ibv_reg_mr( pd, _buffer, _num_bufs * _buffer_size,
        IBV_ACCESS_LOCAL_WRITE );
    if( NULL == _mr )
    {
        EQERROR << "ibv_reg_mr : " << base::sysError << std::endl;
        goto out;
    }

    for( unsigned int i = 0; i != _num_bufs; i++ )
        _ring.put( i );
    ok = true;

out:
    return ok;
}

//////////////////////////////////////////////////////////////////////////////

RingBuffer::RingBuffer( int access )
    : _access( access )
    , _size( 0 )
    , _map( NULL )
    , _mr( NULL )
{
}

RingBuffer::~RingBuffer( )
{
    clear( );
}

void RingBuffer::clear( )
{
    if(( NULL != _mr ) && ( 0 != ::ibv_dereg_mr( _mr )))
        EQWARN << "ibv_dereg_mr : " << base::sysError << std::endl;
    _mr = NULL;

    if(( NULL != _map ) && ( MAP_FAILED != _map ) &&
        ( 0 != ::munmap( _map, _size << 1 )))
        EQWARN << "munmap @ " << _map << " : " << base::sysError << std::endl;
    _map = NULL;

    _size = 0;
}

bool RingBuffer::resize( ibv_pd *pd, const unsigned long size )
{
    bool ok = false;
    void *addr1, *addr2;
    char path[] = "/dev/shm/co-rdma-buffer-XXXXXX";
    int fd = -1;

    clear( );

    if( 0UL == size )
    {
        ok = true;
        goto out;
    }

    _size = size;

    fd = ::mkstemp( path );
    if( 0 > fd )
    {
        EQERROR << "mkstemp : " << base::sysError << std::endl;
        goto out;
    }

    if( 0 != ::unlink( path ))
    {
        EQERROR << "unlink : " << base::sysError << std::endl;
        goto out;
    }

    if( 0 != ::ftruncate( fd, _size ))
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

    ok = true;

out:
    if(( 0 <= fd ) && ( 0 != ::close( fd )))
        EQWARN << "close : " << base::sysError << std::endl;

    return ok;
}

//////////////////////////////////////////////////////////////////////////////

static bool setBlocking( int fd, bool blocking )
{
    int flags = ::fcntl( fd, F_GETFL );
    flags = blocking ? ( flags & ~O_NONBLOCK ) : ( flags | O_NONBLOCK );
    if( 0 != ::fcntl( fd, F_SETFL, flags ))
    {
        EQERROR << "fcntl : " << base::sysError << std::endl;
        goto err;
    }
    return true;

err:
    return false;
}
} // namespace co
