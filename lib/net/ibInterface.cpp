
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
#include "ibInterface.h"
#ifdef EQ_INFINIBAND



#define EQ_MAXBUFFSIZE 16777216
#define EQ_MAXBLOCKBUFFER 65535
#define EQ_NUMBUFFER 1
#define EQ_IB_MAXINLINE 600
namespace eq
{ 
namespace net
{

IBInterface::IBInterface( )
    : _completionQueue( 0 )
    , _queuePair( 0 )
    , _voidPos( 0 )
    , _firstTimeWrite( true )
{
    srand(1);
    _psn = cl_hton32( rand() & 0xffffff );
}

IBInterface::~IBInterface()
{
    close();
}

struct IBDest IBInterface::getMyDest( uint32_t index )
{
    IBDest myDest;
    myDest.lid   = _dlid;
    myDest.qpn   = getSQpNum();
    myDest.rkey  = _writeBlock.getRemoteKey();
    myDest.vaddr = ( uintptr_t ) _writeBlock.buf.getData();
    myDest.psn   = _psn;
    
    return myDest;
}

bool IBInterface::_createQueuePair( IBAdapter* adapter )
{
    // Attributes used to initialize a queue pair at creation time.
    ib_qp_create_t    queuePairCreate;
    memset( &queuePairCreate, 0, sizeof( ib_qp_create_t ));
    
    queuePairCreate.h_sq_cq     = _completionQueue->getWriteHandle();
    queuePairCreate.h_rq_cq     = _completionQueue->getReadHandle();
    // Indicates the requested maximum number of work requests that may be
    // outstanding on the queue pair's send and receive queue.  
    // This value must be less than or equal to the maximum reported 
    // by the channel adapter associated with the queue pair.
    queuePairCreate.sq_depth     = 1;
    queuePairCreate.rq_depth     = 1;
    // Indicates the maximum number scatter-gather elements that may be
    // given in a send and receive  work request.  This value must be less
    // than or equal to the maximum reported by the channel adapter associated
    // with the queue pair.
    queuePairCreate.sq_sge         = 1;
    queuePairCreate.rq_sge         = 1;
    // Send immediate data with the given request.
    queuePairCreate.sq_signaled  = IB_SEND_OPT_SIGNALED;
    
    // connection type RC
    queuePairCreate.qp_type       = IB_QPT_RELIABLE_CONN;
    queuePairCreate.sq_signaled   = true;
    queuePairCreate.sq_max_inline = EQ_IB_MAXINLINE;
    
    // Creates a queue pair
    ib_api_status_t ibStatus = ib_create_qp( adapter->getProtectionDomain(), 
                                             &queuePairCreate, 
                                             0, 0, &_queuePair );
    if ( ibStatus != IB_SUCCESS )
    {
        EQERROR << "cannot create a queue pair" << std::endl; 
        return false;
    }
    return true;
}
bool IBInterface::create( IBAdapter* adapter, 
                          IBCompletionQueue* completionQueue )
{
    
    _completionQueue = completionQueue;

    // memory block Write 
    if ( !_writeBlock.create( adapter->getProtectionDomain(), EQ_MAXBUFFSIZE ))
        return false;
    
    // memory block Read
    if (!_readBlock.create( adapter->getProtectionDomain(), EQ_MAXBUFFSIZE ))
        return false;

    _createQueuePair( adapter );

    ib_qp_mod_t    queuePairModify;
    memset( &queuePairModify, 0, sizeof( ib_qp_mod_t ));
    queuePairModify.req_state               = IB_QPS_INIT;
    queuePairModify.state.init.pkey_index   = 0;
    queuePairModify.state.init.primary_port = 1;
    //    Indicates the type of access is permitted on resources such as QPs,
    //    memory regions and memory windows.
    queuePairModify.state.init.access_ctrl  = IB_AC_RDMA_WRITE | 
                                              IB_AC_LOCAL_WRITE ;
    
    ib_api_status_t ibStatus;
    ibStatus = ib_modify_qp( _queuePair, &queuePairModify );
    if ( ibStatus != IB_SUCCESS )
    {
        EQERROR << "cannot modify a queue pair" << std::endl; 
        return false;
    }

    ibStatus = ib_query_qp( _queuePair, &_queuePairAttr );
    if ( ibStatus != IB_SUCCESS )
    {
        EQERROR << "cannot query a queue pair" << std::endl; 
        return false;
    }

    _dlid = adapter->getLid( 1 ); 

    return true;
}



bool IBInterface::_modiyQueuePairRTR()
{
    /*Information needed to change the state of a queue pair through the
      ib_modify_qp call.*/
    ib_qp_mod_t    attr;
    memset( &attr, 0, sizeof( ib_qp_mod_t ));
    attr.req_state         = IB_QPS_RTR;
    attr.state.rtr.primary_av.conn.path_mtu = IB_MTU_LEN_2048;
    attr.state.rtr.dest_qp           = _dest.qpn;
    attr.state.rtr.rq_psn            = _dest.psn;
    attr.state.rtr.resp_res        = 1;
    attr.state.rtr.rnr_nak_timeout = 12;
    
    attr.state.rtr.primary_av.grh_valid = 0;
    attr.state.rtr.primary_av.dlid      = _dest.lid;
    attr.state.rtr.primary_av.sl        = 0;
    attr.state.rtr.primary_av.path_bits = 0;
    attr.state.rtr.primary_av.port_num  = 1;
    attr.state.rtr.primary_av.static_rate = IB_PATH_RECORD_RATE_10_GBS;
    attr.state.rtr.opts = IB_MOD_QP_LOCAL_ACK_TIMEOUT |
                          IB_MOD_QP_RESP_RES          |
                          IB_MOD_QP_PRIMARY_AV;

    if( ib_modify_qp( _queuePair, &attr ) != IB_SUCCESS )
    {
        EQERROR << "Error during modification Queue pair  RTR" << std::endl; 
        return false;
    }

    return true;

}

bool IBInterface::_modiyQueuePairRTS( int myPsn )
{
    /*Information needed to change the state of a queue pair through the
      ib_modify_qp call.*/
    ib_qp_mod_t    attr;
    memset( &attr, 0, sizeof( ib_qp_mod_t ));
    attr.req_state                   = IB_QPS_RTS;
    attr.state.rts.sq_psn            = myPsn;
    attr.state.rts.resp_res          = 1;
    attr.state.rts.local_ack_timeout = 14;
    attr.state.rts.retry_cnt         = 7;
    attr.state.rts.rnr_retry_cnt     = 7;
    attr.state.rts.opts =   IB_MOD_QP_RNR_RETRY_CNT |
                            IB_MOD_QP_RETRY_CNT |
                            IB_MOD_QP_LOCAL_ACK_TIMEOUT;

    if( ib_modify_qp( _queuePair, &attr ) != IB_SUCCESS )
    {
        EQERROR << "Error during modification Queue pair RTS" << std::endl; 
        return false;
    }

    return true;

}
void IBInterface::modiyQueuePairAttribute( int myPsn )
{
    
    if ( !_modiyQueuePairRTR() ) 
        return;

    
    if ( !_modiyQueuePairRTS( myPsn ))
        return;

    ib_api_status_t     ib_status;
    
    _recvList.vaddr  = _readBlock.getVaddr();
    _recvList.length = EQ_MAXBUFFSIZE;
    _recvList.lkey   = _readBlock.getLocalKey();

    //receive
    _rwr.wr_id      = 2;
    _rwr.ds_array   = &_recvList;
    _rwr.num_ds     = 1;
    _rwr.p_next     = 0;

    _readBlock.buf.resize( 0 );
    
}

void IBInterface::close()
{
    ib_api_status_t    status;

    if( _queuePair ) 
    {
        status = ib_destroy_qp( _queuePair, 0 );
        _queuePair = 0;
    }

    _writeBlock.close();
    _readBlock.close();

    _dlid = 0;
}

void IBInterface::readNB( void* buffer, const uint64_t bytes )
{
    // the buffer has still data ( second part of a sequence )
    if( _voidPos < _readBlock.buf.getSize() )
    {
       SetEvent( _completionQueue->getReadNotifier()); 
       return;
    }

    ResetEvent( _completionQueue->getReadNotifier() );
    _completionQueue->triggerRead();
    ib_post_recv( _queuePair, &_rwr, 0);
}


int64_t IBInterface::postRdmaRecvs( void* buffer, uint32_t bytes )
{
    uint32_t sizebuf = _readBlock.buf.getSize();
    
    // if no data in buffer, we ask for a receive operation
    while ( sizebuf == 0 )
    {
        sizebuf = _waitPollCQ( bytes );
    }
    
    // if no data, it's a problem 
    if ( sizebuf == 0 )
        return -1;
    
    if ( sizebuf > _voidPos )
    {
        uint32_t nbRead = EQ_MIN( bytes, sizebuf - _voidPos);
        char* data = reinterpret_cast<char*>( _readBlock.buf.getData() );
        memcpy( buffer, data + _voidPos, nbRead );
        _voidPos += nbRead;
        // all buffer has been taken
        if (_voidPos == sizebuf)
        {
           _voidPos = 0;
           _readBlock.buf.resize(0);
        }
        return nbRead;
    }
    return -1;
}

int64_t IBInterface::_waitPollCQ( uint32_t bytes )
{
    uint32_t size;
    ib_wc_t    wc;
    
    ib_wc_t* wcDone; //  A list of work completions retrieved from 
                     //  the completion queue.
    ib_wc_t* wcFree; //  a list of work completion structures provided by
                     //     the client.  
                     //  These are used to report completed work requests through
                     //  the pp_done_wclist
    
    wcFree = &wc;
    wcFree->p_next = 0;
    wcDone = 0;
    
    // Checks a completion queue for completed work requests
    ib_api_status_t ibStatus = ib_poll_cq( _completionQueue->getReadHandle(), 
                                           &wcFree, 
                                           &wcDone);
    if ( ibStatus == IB_SUCCESS ) 
    {
        if ( wcDone->status != IB_WCS_SUCCESS ) 
        {
            close();
            size = -1;
        }
        else
        {
            size = wcDone->length;
            _readBlock.buf.resize( size );
        }
    }
    else // no job yet. we wait for new datas
    {
        ResetEvent( _completionQueue->getReadNotifier() );
        if ( !_completionQueue->triggerRead() )
        {
            return -1;
        }
        WaitForSingleObjectEx( _completionQueue->getReadNotifier(), 0, true );
        size = 0;
    }

    return size;
}


int64_t IBInterface::postRdmaWrite( const void* buffer, uint32_t numBytes )
{
    ib_api_status_t    ibStatus;

    ib_wc_t    wc;
    ib_wc_t    *wcDone,*wcFree;

    wcFree = &wc;
    wcFree->p_next = 0;
    wcDone = 0;
    
    // validation of the send job
    do {
        ibStatus = ib_poll_cq( _completionQueue->getWriteHandle(), 
                               &wcFree, 
                               &wcDone );
        if ( ibStatus == IB_SUCCESS ) 
        {
            if ( wcDone->status != IB_WCS_SUCCESS ) 
                return -1;

            _firstTimeWrite = true;
            break;
        }
        else if ( !_firstTimeWrite )
        {
            ibStatus = IB_SUCCESS;
        }
    } while ( ibStatus == IB_SUCCESS );

    uint32_t size EQ_MIN( numBytes, EQ_MAXBUFFSIZE );
    ib_local_ds_t    list;
    
    // if we can send inline data
    /*if ( numBytes < _queuePairAttr.sq_max_inline )
    {
        list.vaddr = ( uintptr_t ) buffer;
        _wr.send_opt = IB_SEND_OPT_INLINE | IB_SEND_OPT_FENCE;
    }
    else
    {
        // add new buffer to memblock 
        _writeBlock.modifyBuffer( buffer, numBytes );
        list.vaddr = ( uintptr_t ) buffer; 
        _wr.send_opt = IB_SEND_OPT_FENCE;
    }*/
    
    char* data = reinterpret_cast<char*>( _writeBlock.buf.getData() );
    memcpy( data, buffer, size );
    list.vaddr   = ( uintptr_t ) data; 
    _wr.send_opt = IB_SEND_OPT_SIGNALED ;
    list.lkey    = _writeBlock.getLocalKey();
    list.length  = size;

    // A 64-bit work request identifier that is returned to the consumer
    // as part of the work completion.
    _wr.wr_id    = 1;
    // A reference to an array of local data segments used by the send
    // operation.
    _wr.ds_array = &list;
    // Number of local data segments specified by this work request.
    _wr.num_ds   = 1;
    // The type of work request being submitted to the send queue.
    _wr.wr_type  = WR_SEND;
    // A pointer used to chain work requests together.  This permits multiple
    // work requests to be posted to a queue pair through a single function
    // call.  This value is set to NULL to mark the end of the chain.
    _wr.p_next   = 0;
    
    if ( !_completionQueue->triggerWrite() )
        return -1;

    // This routine posts a work request to the send queue of a queue pair
    ibStatus = ib_post_send( _queuePair, &_wr, 0 );
    if ( ibStatus != IB_SUCCESS )
        return -1;
    
    _firstTimeWrite = false;
    return size;
}
}
}
#endif //EQ_INFINIBAND
