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
#pragma once

#include <co/connection.h>

#include <co/base/thread.h>
#include <co/base/monitor.h>
#include <co/base/ring.h>

#include <rdma/rdma_cma.h>

#include <netdb.h>

namespace co
{
/**
 * A registered memory region (MR) broken up into a number of fixed size
 * buffers.
 */
class BufferPool
{
public:
    BufferPool( size_t buffer_size );
    ~BufferPool( );

    ibv_mr *getMR( ) const { return _mr; }
    size_t getBufferSize( ) const { return _buffer_size; }
    void *getBuffer( )
    {
        return (void *)( (uintptr_t)_buffer + ( _ring.get( ) * _buffer_size ));
    }
    void freeBuffer( void *buf )
    {
        ::memset( buf, 0, _buffer_size ); // Paranoid

        _ring.put(( (uintptr_t)buf - (uintptr_t)_buffer ) / _buffer_size );
    }

    void clear( );
    bool resize( ibv_pd *pd, uint32_t num_bufs );
private:
    const size_t _buffer_size;
    uint32_t _num_bufs;
    void *_buffer;
    struct ibv_mr *_mr;
    BufferQ<uint32_t> _ring;
}; // BufferPool

/**
 * A registered memory region (MR) backed by a fixed size circular buffer
 * <a href="http://en.wikipedia.org/wiki/Circular_buffer#Optimization">mapped
 * to two contiguous regions of virtual memory</a>.
 */
class RingBuffer
{
public:
    RingBuffer( int access = 0 );
    ~RingBuffer( );

    ibv_mr *getMR( ) const { return _mr; }
    size_t getSize( ) const { return _size; }
    void *getBase( ) const { return _map; }

    void clear( );
    bool resize( ibv_pd *pd, size_t size );
private:
    const int _access;
    size_t _size;
    void *_map;
    struct ibv_mr *_mr;
}; // RingBuffer

struct RDMASetupPayload;
struct RDMAFCPayload;
struct RDMAMessage;

/**
 * An RDMA connection implementation.
 *
 * This connection utilizes the OFED RDMA library to send Collage messages by
 * RDMA write operations on a remote memory region.
 *
 * In order to use this connection type, at least:
 *
 * 1) The rdma_ucm kernel module must be loaded
 * 2) The application must have read/write access to the RDMA device nodes
 *    (typically /dev/infiniband/[rdma_cm|uverbs*])
 * 3) The IP address assigned to the connection must be an address assigned to
 *    an RDMA-capable device (i.e. IPoIB)
 * 4) Shared memory must be sufficient for all RDMA connections,
 *    2 * Global::IATTR_RDMA_RING_BUFFER_SIZE_MB for each
 *    (i.e. /dev/shm, kernel.shm[min|max|all])
 * 5) The user must be able to lock the memory registered with verbs, such that
 *    the locked memory limit needs to be sufficient ("ulimit -l" for bash,
 *    "limit memorylocked" for csh).  Updating /etc/security/limits.conf with
 *    entries like this is usually adequate (e.g. to raise the limit to 2GB for
 *    all users):
 *    * soft memlock 2048000
 *    * hard memlock 2048000
 *
 * NB : Binding to "localhost" does *not* limit remote access, rdma_cm will
 * bind to all available RDMA interfaces as if bound to a wildcard address!
 *
 */
class RDMAConnection : public Connection
{
public:
    RDMAConnection( );

    virtual bool connect( );
    virtual bool listen( );
    virtual void close( );

    virtual void acceptNB( );
    virtual ConnectionPtr acceptSync( );

protected:
    virtual void    readNB  ( void* buffer, const uint64_t bytes );
    virtual int64_t readSync( void* buffer, const uint64_t bytes,
                              const bool ignored );
    virtual int64_t write   ( const void* buffer, const uint64_t bytes );

public:
    virtual Notifier getNotifier( ) const { return _notifier; };

protected:
    virtual ~RDMAConnection( );

    void setState( const State state );

private:
    /* Teardown */
    void _cleanup( );

    /* Setup */
    bool _finishAccept( struct rdma_event_channel *listen_channel );

    bool _lookupAddress( const bool passive );
    void _updateInfo( struct sockaddr_storage *sss );

    bool _createEventChannel( );
    bool _createId( );

    bool _queryDevice( );
    bool _createQP( );
    bool _initBuffers( );

    bool _resolveAddress( );
    bool _resolveRoute( );
    bool _connect( );

    bool _bindAddress( );
    bool _listen( );
    bool _migrateId( );
    bool _accept( );
    bool _reject( );

    /* Protocol */
    bool _postReceives( const int32_t count );
    inline void _recvRDMAWrite( const uint32_t imm_data );
    bool _postRDMAWrite( );
    void _recvMessage( const RDMAMessage &message );
    bool _postMessage( const RDMAMessage &message );
    inline void _recvFC( const RDMAFCPayload &fc );
    bool _postFC( );
    void _recvSetup( const RDMASetupPayload &setup );
    bool _postSetup( );
    bool _waitRecvSetup( );

private:
    /* Connection Manager event handler */
    bool _doCMEvent( struct rdma_event_channel *channel,
        enum rdma_cm_event_type expected );

    /* Completion Queue event handler */
    bool _pollCQ( );
    bool _rearmCQ( );

private:
    /* _cm->fd (listener) or _cm_id->recv_cq_channel->fd (connection) */
    Notifier _notifier;

    /* Protect close( ) from multiple threads */
    base::Lock _close_mutex;

    /* Protect _pollCQ( ) from multiple threads */
    base::Lock _poll_mutex;

    const int32_t _timeout;

    /* Final connection info */
    char _addr[NI_MAXHOST], _serv[NI_MAXSERV];

    /* RDMA/Verbs vars */
    struct rdma_addrinfo *_rai;
    struct rdma_event_channel *_cm;
    struct rdma_cm_id *_cm_id;
    struct rdma_conn_param _conn_param;
    struct ibv_device_attr _dev_attr;
    struct ibv_pd *_pd;

    bool _established;
    base::a_int32_t _credits;
    unsigned int _completions;

    /* MR for setup and FC messages */
    BufferPool _msgbuf;

    /* source RDMA MR */
    RingBuffer _sourcebuf;
    Ring<uint32_t, 3> _sourceptr;
        //        : initialized during connect/accept
        // HEAD   : advanced after copying buffer (fill) on local write
        // MIDDLE : advanced before posting RDMA write
        // TAIL   : advanced after completing RDMA write

    /* sink RDMA MR */
    RingBuffer _sinkbuf;
    Ring<uint32_t, 2> _sinkptr;
        //        : initialized during connect/accept
        // HEAD   : advanced on receipt of RDMA WRITE
        // TAIL   : advanced after copying buffer (drain) on local read

    /* local "view" of remote sink MR */
    Ring<uint32_t, 2> _rptr;
        //        : initialized on receipt of setup message
        // HEAD   : advanced before posting RDMA write
        // TAIL   : advanced on receipt of FC

    /* remote sink MR parameters */
    uint64_t _rbase, _rkey;

    /* copy bytes out of the sink buffer */
    uint32_t _drain( void *buffer, const uint32_t bytes );
    /* copy bytes in to the source buffer */
    uint32_t _fill( const void *buffer, const uint32_t bytes );

private:
    /* Singleton channel event monitor thread */
    class ChannelEventThread;
    static ChannelEventThread *_event_thread;
    static base::Lock _thread_mutex;

    bool _eventThreadRegister( );
    void _eventThreadUnregister( );

    /* Simple command protocol between application and event thread */
    enum CmdStatus
    {
        CMD_WAIT,
        CMD_FAIL,
   /*
    * Set value of CMD_DONE so consumers can test if a command is done by
    * testing (result & CMD_DONE).
    */
        CMD_DONE = 1 << 7,
        CMD_DONE_LAST,
    };
    base::Monitor<CmdStatus> _cmd_block;

    /* Types of file descriptors that the event thread will monitor */
    enum epoll_type { EVENT_FD, CONNECTION_FD };
    struct epoll_context
    {
        epoll_context( ChannelEventThread *thread )
            : type( EVENT_FD ) { ctx.thread = thread; }
        epoll_context( RDMAConnection *connection )
            : type( CONNECTION_FD ) { ctx.connection = connection; }

        const epoll_type type;
        union
        {
            ChannelEventThread *thread;
            RDMAConnection *connection;
        } ctx;
    };
    struct epoll_context _context;
    bool _registered;

private:
    struct stats
    {
        stats( )
            : reads( 0ULL )
            , buffer_empty( 0ULL )
            , no_credits_fc( 0ULL )
            , writes( 0ULL )
            , buffer_full( 0ULL )
            , no_credits_rdma( 0ULL )
        { }

        uint64_t reads;
        uint64_t buffer_empty;
        uint64_t no_credits_fc;
        uint64_t writes;
        uint64_t buffer_full;
        uint64_t no_credits_rdma;
    } _stats;

    void _showStats( );
}; // RDMAConnection
} // namespace co
