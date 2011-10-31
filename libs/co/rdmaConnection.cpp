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

#include <sstream>
#include <limits>

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
   PARAMS = 1 << 0,
   FC     = 1 << 1,
};

/**
 * "ACK" messages sent after read, tells source about read progress
 */
struct RDMAFCPayload {
    uint32_t ringTail;
};

/**
 * "IMM" data sent with RDMA write, tells sink about send progress
 */
typedef uint32_t RDMAFCImm;

/**
 * Initial setup message used to exchange sink MR parameters
 */
struct RDMAParamsPayload
{
    uint64_t rbase;
    uint64_t rlen;
    uint64_t rkey;
    struct RDMAFCPayload fc;
};

/**
 * Payload wrapper
 */
struct RDMAMessage
{
    OpCode opcode;
    uint8_t length;
    union
    {
        struct RDMAParamsPayload params;
        struct RDMAFCPayload fc;
    };
};

RDMAConnection::RDMAConnection( )
    : _notifier( 0 )
    , _event_thread( NULL )
    , _efd( 0 )
    , _setup( SETUP_WAIT )
    , _cm( NULL )
    , _cm_id( NULL )
    , _established( false )
    , _disconnected( false )
    , _depth( 256UL )
    , _pd( NULL )
    , _cc( NULL )
    , _cq( NULL )
    , _qp( NULL )
    , _completions( 0U )
    , _msgbuf( sizeof(struct RDMAMessage) )
    , _available_wr( 0 )
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

    if( !_createEventChannel( )) {}
    else if( !_createId( )) {}
    else if( !_parseAddress( address, false )) {}
    else if( !_resolveAddress( address )) {}
    else if( !_resolveRoute( )) {}
    else if( !_initVerbs( )) {}
    else if( !_initBuffers( )) {}
    else if( !_createQP( )) {}
    else if( !_postReceives( _qpcap.max_recv_wr )) {}
    else if( !_connect( )) {}
    else if( !_startEventThread( )) {}
    else if( !_postSendSetup( )) {}
    else if( !_waitRecvSetup( )) {}
    else
    {
        setState( STATE_CONNECTED );
        return true;
    }

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

    if( !_createEventChannel( )) {}
    else if( !_createId( )) {}
    else if( !_parseAddress( address, true )) {}
    else if( !_bindAddress( address )) {}
    else if( !_listen( )) {}
    else
    {
        _notifier = _cm->fd;
        setState( STATE_LISTENING );
        return true;
    }

    close( );
    return false;
}

void RDMAConnection::close( )
{
    EQVERB << (void *)this << ".close( )" << std::endl;

    if( STATE_CLOSED == _state )
        return;

    EQASSERT( STATE_CLOSING != _state );
    setState( STATE_CLOSING );

    _disconnect( );
    _joinEventThread( );
    _cleanup( );

    _notifier = 0;
    setState( STATE_CLOSED );
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
        return NULL;
    }

    return newConnection;
}

void RDMAConnection::readNB( void* buffer, const uint64_t bytes ) { /* NOP */ }

int64_t RDMAConnection::readSync( void* buffer, const uint64_t bytes,
    const bool )
{
    if( STATE_CONNECTED != _state )
        return -1LL;

    //EQWARN << (void *)this << ".read(" << bytes << ")" <<
    //   " <<<<<<<<<<---------- " << std::endl;

    uint64_t available_bytes;
    while( 0 > ::read( _notifier, (void *)&available_bytes, sizeof(uint64_t)))
    {
        if( EAGAIN == errno )
            continue; // eventfd is non-blocking
        EQINFO << "Got EOF, closing connection" << std::endl;
        close( );
        return -1LL;
    }

    if( _disconnected )
        return -1LL;

    const uint32_t bytes_taken = _drain( buffer,
        static_cast< uint32_t >( std::min( bytes, available_bytes )));

    EQASSERTINFO( bytes_taken <= available_bytes,
        bytes_taken << " > " << available_bytes );
    EQASSERTINFO( bytes_taken > 0, bytes_taken << " == 0" );

    // put back what wasn't taken
    if( available_bytes > bytes_taken )
        _notify( available_bytes - bytes_taken );

    while( _available_wr == 0 ) // TODO: timeout?
        co::base::Thread::yield( );

    // TODO : send FC less frequently?
    if( !_postSendFC( ))
    {
        close( );
        return -1LL;
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

    while( _available_wr == 0 ) // TODO: timeout?
        co::base::Thread::yield( );

    const uint32_t bytes_put = _fill( buffer, static_cast< uint32_t >( bytes ));

    if(( 0UL < bytes_put ) && ( !_postRDMAWrite( )))
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

void RDMAConnection::_disconnect( )
{
    if( _established )
    {
        EQASSERT( NULL != _cm_id );
        EQASSERT( NULL != _cm_id->verbs );

        if( 0 != ::rdma_disconnect( _cm_id ))
            EQWARN << "rdma_disconnect : " << base::sysError << std::endl;
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

    if(( NULL != _cc ) && ( setBlocking( _cc->fd, false )))
        _doCQEvents( _cc ); // drain

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

    if( !_createEventChannel( )) {}
    else if( !_doCMEvent( listen_channel, RDMA_CM_EVENT_CONNECT_REQUEST )) {}
    else if( !_migrateId( )) {}
    else if( !_initVerbs( )) {}
    else if( !_initBuffers( )) {}
    else if( !_createQP( )) {}
    else if( !_postReceives( _qpcap.max_recv_wr )) {}
    else if( !_accept( )) {}
    else if( !_startEventThread( )) {}
    else if( !_postSendSetup( )) {}
    else if( !_waitRecvSetup( )) {}
    else
    {
        setState( STATE_CONNECTED );
        return true;
    }

    close( );
    return false;
}

bool RDMAConnection::_parseAddress( struct sockaddr &address,
    const bool passive )
{
    const char *node = NULL, *service = NULL;
    struct addrinfo hints, *res;

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

    int errcode = ::getaddrinfo( node, service, &hints, &res );
    if( 0 != errcode )
        EQERROR << "getaddrinfo : " << ::gai_strerror( errcode ) << std::endl;
    else
    {
        if( NULL != res->ai_next )
            EQWARN << "multiple getaddrinfo results, using first" << std::endl;
        ::memcpy( (void *)&address, (const void *)res->ai_addr,
            res->ai_addrlen );

        ::freeaddrinfo( res );
        return true;
    }
    return false;
}

bool RDMAConnection::_createEventChannel( )
{
    EQASSERT( NULL == _cm );

    if( NULL == ( _cm = ::rdma_create_event_channel( )))
    {
        EQERROR << "rdma_create_event_channel : " << base::sysError <<
            std::endl;
        return false;
    }
    return true;
}

bool RDMAConnection::_createId( )
{
    EQASSERT( NULL != _cm );

    if( 0 != ::rdma_create_id( _cm, &_cm_id, NULL, RDMA_PS_TCP ))
    {
        EQERROR << "rdma_create_id : " << base::sysError << std::endl;
        return false;
    }
    return true;
}

bool RDMAConnection::_resolveAddress( struct sockaddr &address )
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_resolve_addr( _cm_id, NULL, &address,
        Global::getIAttribute( Global::IATTR_RDMA_RESOLVE_TIMEOUT_MS )))
    {
        EQERROR << "rdma_resolve_addr : " << base::sysError << std::endl;
        return false;
    }
    else // block for RDMA_CM_EVENT_ADDR_RESOLVED
        return _doCMEvent( _cm, RDMA_CM_EVENT_ADDR_RESOLVED );
}

bool RDMAConnection::_resolveRoute( )
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_resolve_route( _cm_id,
        Global::getIAttribute( Global::IATTR_RDMA_RESOLVE_TIMEOUT_MS )))
    {
        EQERROR << "rdma_resolve_route : " << base::sysError << std::endl;
        return false;
    }
    else // block for RDMA_CM_EVENT_ROUTE_RESOLVED
        return _doCMEvent( _cm, RDMA_CM_EVENT_ROUTE_RESOLVED );
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
    _conn_param.retry_count = 3;
    _conn_param.rnr_retry_count = 3;

    if( 0 != ::rdma_connect( _cm_id, &_conn_param ))
    {
        EQERROR << "rdma_connect : " << base::sysError << std::endl;
        return false;
    }
    else // block for RDMA_CM_EVENT_ESTABLISHED
        return _doCMEvent( _cm, RDMA_CM_EVENT_ESTABLISHED );
}

bool RDMAConnection::_bindAddress( struct sockaddr &address )
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_bind_addr( _cm_id, &address ))
    {
        EQERROR << "rdma_bind_addr : " << base::sysError << std::endl;
        return false;
    }
    return true;
}

bool RDMAConnection::_listen( )
{
    EQASSERT( NULL != _cm_id );

    if( 0 != ::rdma_listen( _cm_id, SOMAXCONN ))
    {
        EQERROR << "rdma_listen : " << base::sysError << std::endl;
        return false;
    }
    return true;
}

bool RDMAConnection::_accept( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( !_established );

    _conn_param.responder_resources =
        std::min( static_cast< int >( _conn_param.responder_resources ),
            _dev_attr.max_qp_rd_atom );
    _conn_param.initiator_depth =
        std::min( static_cast< int >( _conn_param.initiator_depth ),
            _dev_attr.max_qp_init_rd_atom );

    if( 0 != ::rdma_accept( _cm_id, &_conn_param ))
    {
        EQERROR << "rdma_accept : " << base::sysError << std::endl;
        return false;
    }
    else // block for RDMA_CM_EVENT_ESTABLISHED
        return _doCMEvent( _cm, RDMA_CM_EVENT_ESTABLISHED );
}

bool RDMAConnection::_migrateId( )
{
    EQASSERT( NULL != _cm_id );
    EQASSERT( NULL != _cm );

    if( 0 != ::rdma_migrate_id( _cm_id, _cm ))
    {
        EQERROR << "rdma_migrate_id : " << base::sysError << std::endl;
        return false;
    }
    return true;
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
        EQERROR << "ibv_query_device" << base::sysError << std::endl;
    else if( NULL == ( _pd = ::ibv_alloc_pd( _cm_id->verbs )))
        EQERROR << "ibv_alloc_pd : " << base::sysError << std::endl;
    else if( NULL == ( _cc = ::ibv_create_comp_channel( _cm_id->verbs )))
        EQERROR << "ibv_create_comp_channel : " << base::sysError << std::endl;
    else if( NULL == ( _cq =
        ::ibv_create_cq( _cm_id->verbs, _depth * 2, NULL, _cc, 0 )))
        EQERROR << "ibv_create_cq : " << base::sysError << std::endl;
    else if( 0 != ::ibv_req_notify_cq( _cq, 0 ))
        EQERROR << "ibv_req_notify_cq : " << base::sysError << std::endl;
    else
        return true;

    return false;
}

bool RDMAConnection::_initBuffers( )
{
    EQASSERT( NULL != _pd );

    const uint32_t rbs =
        Global::getIAttribute( Global::IATTR_RDMA_RING_BUFFER_SIZE_MB );

    if( !_sourcebuf.resize( _pd, rbs * 1024 )) {}
    else if( !_sinkbuf.resize( _pd, rbs * 1024 )) {}
    else
    {
        _sourceptr.clear( _sourcebuf.getSize( ));
        _sinkptr.clear( _sinkbuf.getSize( ));
        return true;
    }
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
        return false;
    }

    _qp = _cm_id->qp;
    _qpcap = init_attr.cap;
    _depth = _qpcap.max_recv_wr;
    _available_wr = _qpcap.max_send_wr;

    EQINFO << "Infiniband QP caps : " <<
        _qpcap.max_recv_wr << " receives, " <<
        _qpcap.max_send_wr << " sends." << std::endl;

    return _msgbuf.resize( _pd, _qpcap.max_send_wr * 2 + _qpcap.max_recv_wr );
}

// caller: application before connect/accept & event thread otherwise
bool RDMAConnection::_postReceives( const unsigned int count )
{
    EQASSERT( NULL != _qp );

    if( 0U < count )
    {
        struct ibv_sge sge[count];
        ::memset( &sge, 0, count * sizeof(struct ibv_sge));
        for( unsigned int i = 0U; i != count; i++ )
        {
            sge[i].addr = (uint64_t)(uintptr_t)_msgbuf.getBuffer( );
            sge[i].length = _msgbuf.getBufferSize( );
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
            return false;
        }
    }
    return true;
}

// caller: event thread
void RDMAConnection::_handleImm( const uint32_t imm )
{
    RDMAFCImm fc = ntohl( imm );

    _sinkptr.incrHead( fc );
    _notify( fc );
}

// caller: event thread
void RDMAConnection::_handleFC( RDMAFCPayload &fc )
{
    _rptr.moveValue( _rptr.TAIL, fc.ringTail );
}

// caller: event thread
void RDMAConnection::_handleSetup( RDMAParamsPayload &params )
{
    _rbase = params.rbase;
    _rptr.clear( params.rlen );
    _rkey = params.rkey;
    _handleFC( params.fc );

    _setup.set( SETUP_OK );
}

// caller: event thread
void RDMAConnection::_handleMsg( RDMAMessage &message )
{
    switch( message.opcode )
    {
        case FC:
            _handleFC( message.fc );
            break;
        case PARAMS:
            _handleSetup( message.params );
            break;
    }
}

// caller: application
uint32_t RDMAConnection::_makeImm( )
{
    RDMAFCImm fc = _sourceptr.available( _sourceptr.HEAD, _sourceptr.MIDDLE );
    _sourceptr.incr( _sourceptr.MIDDLE, fc );

    return htonl( fc );
}

// caller: application
void RDMAConnection::_fillFC( RDMAFCPayload &fc )
{
    fc.ringTail = _sinkptr.value( _sinkptr.MIDDLE );
    _sinkptr.moveValue( _sinkptr.TAIL, fc.ringTail );
}

// caller: application
void RDMAConnection::_fillParams( RDMAParamsPayload &params )
{
    params.rbase = (uint64_t)(uintptr_t)_sinkbuf.getBase( );
    params.rlen = _sinkbuf.getSize( );
    params.rkey = _sinkbuf.getMR( )->rkey;
    _fillFC( params.fc );
}

// caller: application
bool RDMAConnection::_postSendWR( struct ibv_send_wr &wr )
{
    EQASSERT( NULL != _qp );

    // track available
#ifdef EQ_RELEASE_ASSERT
    EQCHECK( --_available_wr >= 0 );
#else
    --_available_wr;
#endif

    struct ibv_send_wr *bad_wr;
    if( 0 != ::ibv_post_send( _qp, &wr, &bad_wr ))
    {
        EQERROR << "ibv_post_send : "  << base::sysError << std::endl;
        return false;
    }
    return true;
}

// caller: application
bool RDMAConnection::_postSendMessage( RDMAMessage &message )
{
    struct ibv_sge sge; 
    ::memset( (void *)&sge, 0, sizeof(struct ibv_sge));
    sge.addr = (uint64_t)&message;
    sge.length =
        (uint32_t)( sizeof(OpCode) + sizeof(uint8_t) + message.length );
    sge.lkey = _msgbuf.getMR( )->lkey;

    struct ibv_send_wr wr;
    ::memset( (void *)&wr, 0, sizeof(struct ibv_send_wr));
    wr.wr_id = sge.addr;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.opcode = IBV_WR_SEND;

    return _postSendWR( wr );
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
bool RDMAConnection::_postSendSetup( )
{
    RDMAMessage &message =
        *reinterpret_cast< RDMAMessage * >( _msgbuf.getBuffer( ));
    message.opcode = PARAMS;
    message.length = sizeof(struct RDMAParamsPayload);
    _fillParams( message.params );

    return _postSendMessage( message );
}

// caller: application
bool RDMAConnection::_postRDMAWrite( )
{
    EQASSERT( NULL != _qp );

    // TODO : break up large messages into multiple WR?

    struct ibv_sge sge; 
    ::memset( (void *)&sge, 0, sizeof(struct ibv_sge));
    sge.addr = (uint64_t)( (uintptr_t)_sourcebuf.getBase( ) +
        _sourceptr.ptr( _sourceptr.MIDDLE ));
    sge.length = _sourceptr.available( _sourceptr.HEAD, _sourceptr.MIDDLE );
    sge.lkey = _sourcebuf.getMR( )->lkey;

    struct ibv_send_wr wr;
    ::memset( (void *)&wr, 0, sizeof(struct ibv_send_wr));
    wr.wr_id = (uint64_t)_sourceptr.value( _sourceptr.MIDDLE );
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
    wr.imm_data = _makeImm( );
    wr.wr.rdma.rkey = _rkey;
    wr.wr.rdma.remote_addr = _rbase + _rptr.ptr( _rptr.HEAD );
    _rptr.incrHead( sge.length );

    return _postSendWR( wr );
}

// caller: application
bool RDMAConnection::_waitRecvSetup( )
{
    // TODO : timeout/SETUP_NOK

    return( SETUP_OK == _setup.waitNE( SETUP_WAIT ));
}

// caller: application if listener or during connect/accept,
// event thread if connected
bool RDMAConnection::_doCMEvent( struct rdma_event_channel *channel,
    rdma_cm_event_type expected )
{
    struct rdma_cm_event *event;

    if( 0 > ::rdma_get_cm_event( channel, &event ))
    {
        if( EAGAIN == errno )
            return true; // channel is non-blocking & no events are available
        EQERROR << "rdma_get_cm_event(" << errno << ") : " << base::sysError <<
            std::endl;
    }
    else
    {
        bool ok = ( event->event == expected );

        EQVERB << (void *)this << " : " << ::rdma_event_str( event->event )
            << "( " << (!ok ? "*not* " : "") << "expected )" << std::endl;

        // special case, flag that its safe to call rdma_disconnect
        _established = ( ok && ( RDMA_CM_EVENT_ESTABLISHED == event->event ));

        // special case, extract connection params from event
        if( ok && ( RDMA_CM_EVENT_CONNECT_REQUEST == event->event ))
        {
            // TODO : reject "bad" connect requests?
            // e.g. also require a magic value (i.e. password) in private_data?
            _cm_id = event->id;
            _conn_param = event->param.conn;
            // Note that the actual amount of data transferred to the
            // remote side is transport dependent and may be larger
            // than that requested.  TODO : Probably shouldn't assert
            // here, instead reject.
            EQASSERT( sizeof(uint32_t) <= _conn_param.private_data_len );
            _depth = *reinterpret_cast< const uint32_t * >(
                _conn_param.private_data );

            // Won't be valid after ack'ing the event
            _conn_param.private_data = NULL;
            _conn_param.private_data_len = 0;
        }

        if( 0 != ::rdma_ack_cm_event( event ))
            EQWARN << "rdma_ack_cm_event : "  << base::sysError << std::endl;
        return ok;
    }
    return false;
}

// caller: event thread while connected, application on cleanup
bool RDMAConnection::_doCQEvents( struct ibv_comp_channel *channel )
{
    struct ibv_cq *ev_cq;
    void *ev_ctx;

    if( 0 > ::ibv_get_cq_event( channel, &ev_cq, &ev_ctx ))
    {
        if( EAGAIN == errno )
            return true; // channel is non-blocking & no events are available
        EQERROR << "ibv_get_cq_event(" << errno << ") : " << base::sysError <<
            std::endl;
    }
    else
    {
        EQASSERT( ev_cq == _cq );

        _completions++;
        if( std::numeric_limits< unsigned >::max() <= _completions )
        {
            ::ibv_ack_cq_events( ev_cq, _completions );
            _completions = 0U;
        }

        if( 0 != ::ibv_req_notify_cq( ev_cq, 0 ))
            EQERROR << "ibv_req_notify_cq : " << base::sysError << std::endl;
        else
        {
            struct ibv_wc wcs[_depth * 2];

            int count =
                ::ibv_poll_cq( ev_cq, sizeof(wcs) / sizeof(wcs[0]), wcs );
            if( 0 > count )
                EQERROR << "ibv_poll_cq : " << base::sysError << std::endl;
            else
            {
                unsigned int num_recvs = 0U;
                bool success = true;

                for( int i = 0; ( i != count ) && success; i++ )
                {
                    struct ibv_wc &wc = wcs[i];

                    if( IBV_WC_SUCCESS != wc.status )
                    {
                        if(( IBV_WC_WR_FLUSH_ERR == wc.status ) &&
                            ( STATE_CLOSING == _state ))
                            continue; // ignore flush errors while closing
                        EQERROR << "!IBV_WC_SUCCESS : " <<
                            ::ibv_wc_status_str( wc.status ) << std::endl;
                        success = false;
                    }
                    else
                    {
                        // Send/write completions
                        if(( IBV_WC_SEND == wc.opcode ) ||
                            ( IBV_WC_RDMA_WRITE == wc.opcode ))
                        {
                            // track available
#ifdef EQ_RELEASE_ASSERT
                            EQCHECK( ++_available_wr <=int(_qpcap.max_send_wr));
#else
			    ++_available_wr;
#endif
                            if( IBV_WC_RDMA_WRITE == wc.opcode )
                            {
                                _rptr.moveValue( _rptr.MIDDLE,
                                    static_cast< uint32_t >( wc.wr_id ));
                                _sourceptr.moveValue( _sourceptr.TAIL,
                                    static_cast< uint32_t >( wc.wr_id ));
                                // not a message buffer, don't free
                                wc.wr_id = 0ULL;
                            }
                        }
                        // Receive completions
                        else if( IBV_WC_RECV == wc.opcode )
                            _handleMsg(
                                *reinterpret_cast< RDMAMessage * >( wc.wr_id ));
                        else if( IBV_WC_RECV_RDMA_WITH_IMM == wc.opcode )
                            _handleImm( wc.imm_data );

                        if( IBV_WC_RECV & wc.opcode )
                            num_recvs++;

                        if( 0ULL != wc.wr_id )
                            _msgbuf.freeBuffer( (void *)(uintptr_t)wc.wr_id );
                    }
                }

                if( success )
                    return _postReceives( num_recvs );
            }
        }
    }
    return false;
}

// caller: application & event thread
void RDMAConnection::_notify( const uint64_t val )
{
    EQASSERT( 0 < _notifier );

    if( ::write( _notifier, (const void *)&val, sizeof( val )) != sizeof( val ))
        EQWARN << "Write failed" << std::endl;
}

// caller: application
bool RDMAConnection::_startEventThread( )
{
    EQASSERT( 0 == _notifier );
    EQASSERT( NULL == _event_thread );

    if( 0 > ( _notifier = ::eventfd( 0, EFD_NONBLOCK )))
        EQERROR << "eventfd : " << base::sysError << std::endl;
    else if( !( _event_thread = new ChannelEventThread( this ))->start( ))
        EQERROR << "Event thread failed to start!" << std::endl;
    else
        return true;

    if(( 0 < _notifier ) && ( 0 != ::close( _notifier )))
        EQWARN << "close : " << base::sysError << std::endl;
    return false;
}

// caller: event thread
bool RDMAConnection::_initEventThread( )
{
    if( !setBlocking( _cm->fd, false )) {}
    else if( !setBlocking( _cc->fd, false )) {}
    else if( 0 > ( _efd = ::epoll_create( 2 )))
        EQERROR << "epoll_create : " << base::sysError << std::endl;
    else
        return true;

    _setup.set( SETUP_NOK );
    return false;
}

// caller: event thread
void RDMAConnection::_runEventThread( )
{
    enum { CM_EVENT = 0, CQ_EVENT = 1 };
    unsigned int cm_event = CM_EVENT, cq_event = CQ_EVENT;
    struct epoll_event evctl[2] =
    {
        { EPOLLIN | EPOLLET, { reinterpret_cast< void * >( &cm_event ) }},
        { EPOLLIN | EPOLLET, { reinterpret_cast< void * >( &cq_event ) }}
    };

    if( 0 != ::epoll_ctl( _efd, EPOLL_CTL_ADD, _cm->fd, &evctl[CM_EVENT] ))
        EQERROR << "epoll_ctl : " << base::sysError << std::endl;
    else if( 0 != ::epoll_ctl( _efd, EPOLL_CTL_ADD, _cc->fd, &evctl[CQ_EVENT] ))
        EQERROR << "epoll_ctl : " << base::sysError << std::endl;
    else
    {
        bool ok = true;
        struct epoll_event events[sizeof(evctl) / sizeof(evctl[0])];
        do
        {
            int nfds = ::epoll_wait( _efd, events,
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
                        *reinterpret_cast< unsigned int * >(
                            events[n].data.ptr );
                    if( cq_event == event )
                        ok = _doCQEvents( _cc );
                    else if( cm_event == event )
                        ok = !_doCMEvent( _cm, RDMA_CM_EVENT_DISCONNECTED );
                }
            }
        }
        while( ok );
    }

    _disconnected = true;
    _notify( 1ULL );

    if( 0 != ::close( _efd ))
        EQWARN << "close : " << base::sysError << std::endl;
    if( 0 != ::close( _notifier ))
        EQWARN << "close : " << base::sysError << std::endl;
}

void RDMAConnection::_joinEventThread( )
{
    if( NULL != _event_thread )
    {
        _event_thread->join( );
        delete _event_thread;
        _event_thread = NULL;
    }
}

uint32_t RDMAConnection::_drain( void *buffer, const uint32_t bytes )
{
    const uint32_t b = std::min( bytes,
        _sinkptr.available( _sinkptr.HEAD, _sinkptr.MIDDLE ));
    ::memcpy( buffer, (const void *)((uintptr_t)_sinkbuf.getBase( ) +
        _sinkptr.ptr( _sinkptr.MIDDLE )), b );
    _sinkptr.incr( _sinkptr.MIDDLE, b );
    return b;
}

uint32_t RDMAConnection::_fill( const void *buffer, const uint32_t bytes )
{
    const uint32_t b = std::min( bytes,
        std::min( _sourceptr.negAvailable( _sourceptr.HEAD, _sourceptr.TAIL ),
            _rptr.negAvailable( _rptr.HEAD, _rptr.MIDDLE )));
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
        ok = true;
    else
    {
        _num_bufs = num_bufs;
        _ring.clear( _num_bufs );

        if( NULL == ( _buffer = ::malloc( _num_bufs * _buffer_size )))
            EQERROR << "malloc : " << base::sysError << std::endl;
        else if( NULL == ( _mr = ::ibv_reg_mr( pd, _buffer,
            _num_bufs * _buffer_size, IBV_ACCESS_LOCAL_WRITE )))
            EQERROR << "ibv_reg_mr : " << base::sysError << std::endl;
        else
        {
            for( unsigned int i = 0; i != _num_bufs; i++ )
                _ring.put( i );
            ok = true;
        }
    }
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

    clear( );

    if( 0UL == size )
        ok = true;
    else
    {
        _size = size;

        char path[] = "/dev/shm/co-rdma-buffer-XXXXXX";
        int fd = ::mkstemp( path );
        if( 0 > fd )
            EQERROR << "mkstemp : " << base::sysError << std::endl;
        else if( 0 != ::unlink( path ))
            EQERROR << "unlink : " << base::sysError << std::endl;
        else if( 0 != ::ftruncate( fd, _size ))
            EQERROR << "ftruncate : " << base::sysError << std::endl;
        else if( MAP_FAILED == ( _map = ::mmap( NULL, _size << 1,
            PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 )))
            EQERROR << "mmap : " << base::sysError << std::endl;
        else if( MAP_FAILED == ( addr1 = ::mmap( _map, _size,
            PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0 )))
            EQERROR << "mmap : " << base::sysError << std::endl;
        else if( MAP_FAILED == ( addr2 =
            ::mmap( (void *)( (uintptr_t)_map + _size ), _size,
                PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0 )))
            EQERROR << "mmap : " << base::sysError << std::endl;
        else if( NULL == ( _mr = ::ibv_reg_mr( pd, _map, _size << 1, _access )))
            EQERROR << "ibv_reg_mr : " << base::sysError << std::endl;
        else
        {
            EQASSERT( addr1 == _map );
            EQASSERT( addr2 == (void *)( (uintptr_t)_map + _size ));

            ::memset( _map, 0, _size );
            *reinterpret_cast< uint8_t * >( _map ) = 0x45;
            EQASSERT( 0x45 ==
                *reinterpret_cast< uint8_t * >( (uintptr_t)_map + _size ));
            ok = true;
        }

        if(( 0 < fd ) && ( 0 != ::close( fd )))
            EQWARN << "close : " << base::sysError << std::endl;
    }
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
        return false;
    }
    return true;
}
} // namespace co
