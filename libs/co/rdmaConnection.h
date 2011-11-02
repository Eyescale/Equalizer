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
#pragma once

#include <co/connection.h>

#include <co/base/thread.h>
#include <co/base/monitor.h>
#include <co/base/ring.h>

#include <netinet/in.h>
#include <rdma/rdma_cma.h>

namespace co
{
/**
 * A registered memory region (MR) broken up into a number of fixed size
 * buffers.
 */
class BufferPool
{
public:
    BufferPool( unsigned int buffer_size );
    ~BufferPool( );

    inline ibv_mr *getMR( ) const { return _mr; }
    inline unsigned int getBufferSize( ) const { return _buffer_size; }
    inline void *getBuffer( )
    {
        return (void *)( (uintptr_t)_buffer + ( _ring.get( ) * _buffer_size ));
    }
    inline void freeBuffer( void *buf )
    {
        _ring.put(( (uintptr_t)buf - (uintptr_t)_buffer ) / _buffer_size );
    }

    void clear( );
    bool resize( ibv_pd *pd, const unsigned int num_bufs );
private:
    const unsigned int _buffer_size;
    unsigned int _num_bufs;
    void *_buffer;
    struct ibv_mr *_mr;
    BufferQ<unsigned int> _ring;
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

    inline ibv_mr *getMR( ) const { return _mr; }
    inline uint32_t getSize( ) const { return _size; }
    inline void *getBase( ) const { return _map; }

    void clear( );
    bool resize( ibv_pd *pd, const unsigned long size );
private:
    const int _access;
    uint32_t _size;
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
 * RDMA write operations on the remote MR.  Since there are two "notifiers" when
 * connected via RDMA CM (in the connection manager channel and the completion
 * channel), a thread is launched that monitors both via epoll(7) and dispatches
 * events via an eventfd(2) that the application can monitor.  For listening,
 * however, the connection manager's channel is sufficient and it is simply
 * handed back to the application for monitoring.
 *
 * In order to use this connection type, at least:
 *
 * 1) The rdma_ucm kernel module must be loaded
 * 2) The application must have read/write access to the IB device nodes
 *    (typically /dev/infiniband/[rdma_cm|uverbs*])
 * 3) The IP address assigned to the connection must be an address assigned to
 *    an RDMA-capable device (e.g. IPoIB)
 * 4) Shared memory must be sufficient for all RDMA connections,
 *    2 * Global::IATTR_RDMA_RING_BUFFER_SIZE_MB for each one
 *    (i.e. /dev/shm, kernel.shm[min|max|all])
 * 5) The user must be able to lock the memory registered with verbs, such that
 *    the locked memory limit needs to be sufficient ("ulimit -l" for bash,
 *    "limit memorylocked" for csh).  Updating /etc/security/limits.conf with
 *    entries like this is usually adequate (e.g. to raise the limit to 2GB for
 *    all users):
 *    * soft memlock 2048000
 *    * hard memlock 2048000
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
    void _disconnect( );
    void _cleanup( );

    /* Setup */
    bool _finishAccept( struct rdma_event_channel *listen_channel );

    bool _parseAddress( struct sockaddr &address, const bool passive ) const;
    bool _createEventChannel( );
    bool _createId( );

    bool _resolveAddress( struct sockaddr &address );
    bool _resolveRoute( );
    bool _connect( );

    bool _bindAddress( struct sockaddr &address ) const;
    bool _listen( ) const;
    bool _accept( );
    bool _migrateId( ) const;

    bool _initVerbs( );
    bool _initBuffers( );
    bool _createQP( );

    bool _postReceives( const unsigned int count );

    /* Event thread handlers */
    void _handleSetup( RDMASetupPayload &setup );
    void _handleFC( RDMAFCPayload &fc );
    void _handleMessage( RDMAMessage &message );
    void _handleImm( const uint32_t imm );

    /* Application read/write services */
    bool _postSendWR( struct ibv_send_wr &wr );

    bool _postSendMessage( RDMAMessage &message );
    void _fillSetup( RDMASetupPayload &setup ) const;
    bool _postSendSetup( );
    void _fillFC( RDMAFCPayload &fc ) const;
    bool _postSendFC( );

    bool _postRDMAWrite( );

    bool _waitRecvSetup( ) const;

private:
    /* Connection Manager event handler */
    bool _doCMEvent( struct rdma_event_channel *channel,
        rdma_cm_event_type expected );
    /* Completion Queue event handler */
    bool _doCQEvents( struct ibv_comp_channel *channel );

    /* Event handler thread */
    class ChannelEventThread : public base::Thread
    {       
    public:
        ChannelEventThread( RDMAConnection *conn ) : _conn( conn ) { }
        virtual ~ChannelEventThread( ) { _conn = NULL; }

        virtual bool init( ) { return _conn->_initEventThread( ); }
        virtual void run( ) { _conn->_runEventThread( ); }
    private:
        RDMAConnection *_conn;
    };  

    Notifier _notifier;
    void _notify( const uint64_t val ) const;

    ChannelEventThread *_event_thread;
    int _efd; // epoll fd

    bool _startEventThread( );
    bool _initEventThread( );
    void _runEventThread( );
    void _joinEventThread( );

private:
    enum SetupWait
    {
        SETUP_NOK  = 0,
        SETUP_OK   = 1,
        SETUP_WAIT = 2
    };
    /* Blocks application connect/accept until the event thread receives the
       remote sink's MR parameters */
    base::Monitor<SetupWait> _setup;

    struct rdma_event_channel *_cm;
    struct rdma_cm_id *_cm_id;
    struct rdma_conn_param _conn_param;
    bool _established;
    bool _disconnected;
    uint32_t _depth;

    struct ibv_device_attr _dev_attr;
    struct ibv_pd *_pd;
    struct ibv_comp_channel *_cc;
    struct ibv_cq *_cq;
    struct ibv_qp *_qp;
    unsigned int _completions;
    struct ibv_qp_cap _qpcap;

    /* Send WR tracking */
    base::a_int32_t _available_wr;

    /* MR for setup and FC messages */
    BufferPool _msgbuf;

    /* source RDMA MR */
    RingBuffer _sourcebuf;
    Ring<uint32_t, 3> _sourceptr;
        //        : initialized by application during connect/accept
        // HEAD   : advanced by application after copying buffer on local write
        // MIDDLE : advanced by application before posting RDMA write
        // TAIL   : advanced by event thread after completing RDMA write

    /* sink RDMA MR */
    RingBuffer _sinkbuf;
    Ring<uint32_t, 2> _sinkptr;
        //        : initialized by application during connect/accept
        // HEAD   : advanced by event thread on receipt of remote FC
        // TAIL   : advanced by application after copying buffer on local read

    /* local "view" of remote sink MR */
    Ring<uint32_t, 2> _rptr;
        //        : initialized by event thread on receipt of setup message
        // HEAD   : advanced by application while posting RDMA write
        // TAIL   : advanced by event thread on receipt of remote FC

    /* remote sink MR parameters */
    uint64_t _rbase, _rkey;

    /* copy bytes out of the sink buffer */
    uint32_t _drain( void *buffer, const uint32_t bytes );
    /* copy bytes in to the source buffer */
    uint32_t _fill( const void *buffer, const uint32_t bytes );
}; // RDMAConnection
} // namespace co
