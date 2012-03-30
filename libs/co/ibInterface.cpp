
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
#include "ibConnection.h"
#include "eq/base/clock.h"
#include "eq/base/memcpy.h"
#ifdef EQ_INFINIBAND
//#define EQ_MEASURE_TIME 0
#define EQ_MAXBLOCKBUFFER 65536
#define EQ_NUMBLOCKMEMORY 1    // maybe remove it
#define EQ_MAXBUFFSIZE EQ_NUMBUFFER*EQ_MAXBLOCKBUFFER
#define EQ_IB_MAXINLINE 600
namespace co
{


IBInterface::IBInterface( )
    : _completionQueue( 0 )
    , _queuePair( 0 )
    , _posReadInBuffer( 0 )
    , _firstTimeWrite( true )
    , _numBufRead( 0 )
    , _numBufWrite( 0 )
{
    srand(1);
    _countWrite = 0;
    _psn = cl_hton32( rand() & 0xffffff );
    _writePoll.resize( EQ_NUMBLOCKMEMORY );
    _readPoll.resize( EQ_NUMBLOCKMEMORY );
    _dests.resize( EQ_NUMBLOCKMEMORY );
    _writeBlocks.resize( EQ_NUMBLOCKMEMORY );
    _readBlocks.resize( EQ_NUMBLOCKMEMORY );
    for (int i = 0; i < EQ_NUMBLOCKMEMORY ; i++)
    {
        _writeBlocks[i] = new IBMemBlock();
        _readBlocks[i] = new IBMemBlock();
    }
    

     memset( _readPoll.getData(), 0, sizeof ( uint32_t ) 
                            * EQ_NUMBLOCKMEMORY );
     memset( _writePoll.getData(), true, sizeof ( bool ) * EQ_NUMBLOCKMEMORY );

    _floatTimeReadNB = 0.0f;
    _floatTimeReadSync  = 0.0f;
    
    _timeTotalWaitPoll = 0.0f;
    _timeTotalWaitobj  = 0.0f;

    _timeTotalWrite  = 0.0f;
    _timeTotalWriteWait  = 0.0f;

    _timeCopyBufferRead = 0.0f;
    _timeCopyBufferWrite = 0.0f;
}

IBInterface::~IBInterface()
{
    close();
}

uint32_t IBInterface::getNumberBlockMemory()
{
    return EQ_NUMBLOCKMEMORY;
}
struct IBDest IBInterface::getMyDest( uint32_t index )
{
    IBDest myDest;
    myDest.lid   = _dlid;
    myDest.qpn   = getSQpNum();
    myDest.rkey  = _writeBlocks[index]->getRemoteKey();
    myDest.vaddr = _writeBlocks[index]->getVaddr();
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
    queuePairCreate.sq_depth     = EQ_NUMBLOCKMEMORY;
    queuePairCreate.rq_depth     = EQ_NUMBLOCKMEMORY;
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
                          IBCompletionQueue* completionQueue,
                          IBConnection* ibConnection )
{
    
    _completionQueue = completionQueue;
    _ibConnection = ibConnection;
    // memory block Write 
    for ( int i = 0; i < EQ_NUMBLOCKMEMORY ; i++ )
    {
        if ( !_writeBlocks[i]->create( adapter->getProtectionDomain(), 
            EQ_MAXBLOCKBUFFER ))
            return false;
        
        // memory block Read
        if ( !_readBlocks[i]->create( adapter->getProtectionDomain(), 
            EQ_MAXBLOCKBUFFER ))
            return false;
    }

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



bool IBInterface::_setAttributeReadyToReceive()
{
    /*Information needed to change the state of a queue pair through the
      ib_modify_qp call.*/
    ib_qp_mod_t    attr;
    memset( &attr, 0, sizeof( ib_qp_mod_t ));
    attr.req_state         = IB_QPS_RTR;
    attr.state.rtr.primary_av.conn.path_mtu = IB_MTU_LEN_4096;
    attr.state.rtr.dest_qp           = _dests.getData()[0].qpn;
    attr.state.rtr.rq_psn            = _dests.getData()[0].psn;
    attr.state.rtr.resp_res        = 1;
    attr.state.rtr.rnr_nak_timeout = 12;
    
    attr.state.rtr.primary_av.grh_valid = 0;
    attr.state.rtr.primary_av.dlid      = _dests.getData()[0].lid;
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

bool IBInterface::_setAttributeReadyToSend( )
{
    /*Information needed to change the state of a queue pair through the
      ib_modify_qp call.*/
    ib_qp_mod_t    attr;
    memset( &attr, 0, sizeof( ib_qp_mod_t ));
    attr.req_state                   = IB_QPS_RTS;
    attr.state.rts.sq_psn            = _psn;
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
void IBInterface::modiyQueuePairAttribute( )
{
    
    if ( !_setAttributeReadyToReceive() ) 
        return;

    
    if ( !_setAttributeReadyToSend( ))
        return;

    _completionQueue->triggerRead();
    ResetEvent( _completionQueue->getReadNotifier());
    for ( int i = 0; i < EQ_NUMBLOCKMEMORY ; i++ )
    {
        _recvList.vaddr  = _readBlocks[ i ]->getVaddr();
        _recvList.length = EQ_MAXBLOCKBUFFER;
        _recvList.lkey   = _readBlocks[ i ]->getLocalKey();

        //receive
        _rwr.wr_id      = i;
        _rwr.ds_array   = &_recvList;
        _rwr.num_ds     = 1;
        _rwr.p_next     = 0;
    

        ib_api_status_t ibStatus = ib_post_recv( _queuePair, &_rwr, 0);
        _readPoll.getData()[ i ];
    }
}

void IBInterface::close()
{
#ifdef EQ_MEASURE_TIME
    EQINFO << " Time total to ReadNB     : " 
           << _floatTimeReadNB << std::endl;
    EQINFO << " Time total to ReadSync   : " 
           << _floatTimeReadSync << std::endl;
    EQINFO << " Time total to WaitPoll   : " 
           << _timeTotalWaitPoll << std::endl;
    EQINFO << " Time total to copyBuffer : "
           << _timeCopyBufferRead << std::endl;
    EQINFO << " Time total to WaitObj    : " 
           <<  _timeTotalWaitobj << std::endl;

    EQINFO << " Time total to Write   : " 
           << _timeTotalWrite << std::endl;
    EQINFO << " Time total to WaitWrite   : " 
           << _timeTotalWriteWait << std::endl;
    EQINFO << " Time total to copyBuffer : " 
           << _timeCopyBufferWrite << std::endl;
#endif
    ib_api_status_t    status;

    if( _queuePair ) 
    {
        status = ib_destroy_qp( _queuePair, 0 );
        _queuePair = 0;
    }
    for (int i = 0; i < EQ_NUMBLOCKMEMORY ; i++)
    {
        _writeBlocks[i]->close();
        _readBlocks[i]->close();
    }

    _dlid = 0;
}

bool IBInterface::_ibPostRecv( uint32_t numBuffer )
{
    _recvList.vaddr  = _readBlocks[ numBuffer ]->getVaddr();
    _recvList.length = EQ_MAXBLOCKBUFFER;
    _recvList.lkey   = _readBlocks[ numBuffer ]->getLocalKey();

    //receive
    _rwr.wr_id      = numBuffer;
    _rwr.ds_array   = &_recvList;
    
    _completionQueue->resetEventRead();
    eq::lunchbox::ScopedMutex mutex( _completionQueue->_mutex );
    ib_api_status_t ibStatus = ib_post_recv( _queuePair, &_rwr, 0);
    if ( ibStatus != IB_SUCCESS )
    {
        EQERROR << "Error during ib_post_recv" << std::endl; 
        return false;
    }
    
    return true;
}

void IBInterface::readNB( void* buffer, const uint64_t bytes )
{
#ifdef EQ_MEASURE_TIME
    eq::lunchbox::Clock       clock;
    clock.reset();
#endif


#ifdef EQ_MEASURE_TIME
    _floatTimeReadNB += clock.getTimef();
#endif
}

int64_t IBInterface::readSync( void* buffer, uint32_t bytes )
{

#ifdef EQ_MEASURE_TIME
    eq::lunchbox::Clock       clock;
    clock.reset();
#endif

    uint32_t comptRead = 0;
    int64_t sizebuf = _readPoll.getData()[ _numBufRead ];

    // if no data in buffer, we ask for a receive operation
    while ( sizebuf < 1 )
#ifdef EQ_MEASURE_TIME
        eq::lunchbox::Clock       clockWait;
        clockWait.reset();
#endif
        
        sizebuf = _waitPollCQ( _numBufRead );

#ifdef EQ_MEASURE_TIME
            _timeTotalWaitPoll += clockWait.getTimef();
#endif
        
    if ( sizebuf > _posReadInBuffer )
    {
#ifdef EQ_MEASURE_TIME
        eq::lunchbox::Clock       clockCopy;
        clockCopy.reset();
#endif
        // find a better memcpy or a system that we don't need to use memcpy
        // copy buffer
        uint32_t nbRead = LB_MIN( bytes, sizebuf - _posReadInBuffer);
     /*   memcpy(reinterpret_cast<char*>( buffer ) + comptRead, 
                                 reinterpret_cast<char*>
                                 ( _readBlocks[_numBufRead]->buf.getData()) + 
                                   _posReadInBuffer, nbRead );*/
        
        eq::lunchbox::fastCopy( reinterpret_cast<char*>( buffer ) + comptRead, 
                                 reinterpret_cast<char*>
                                 ( _readBlocks[_numBufRead]->buf.getData() ) + 
                                   _posReadInBuffer, nbRead );
        _posReadInBuffer += nbRead;
#ifdef EQ_MEASURE_TIME
        _timeCopyBufferRead += clockCopy.getTimef();
#endif
        // all buffer has been taken
        if ( _posReadInBuffer == sizebuf )
        {
           _completionQueue->removeEventRead();
           // init var for next read
           _posReadInBuffer = 0;
           _readPoll.getData()[ _numBufRead ] = 0;
           // next Read in on the next CQ
           if (_numBufRead == EQ_NUMBLOCKMEMORY-1 )
              _ibConnection->incReadInterface();

           // notify that read is finnish 
           _ibPostRecv( _numBufRead );
           // To do work with more buffer in a CQ 
           _numBufRead = ( _numBufRead + 1 ) % EQ_NUMBLOCKMEMORY;
        }
        return nbRead;
    }

    EQWARN << "ERROR IN READ SYNC"<< std::endl;
    return -1;
    
#ifdef EQ_MEASURE_TIME
    _floatTimeReadSync += clock.getTimef();
#endif
}


int64_t IBInterface::_waitPollCQ( uint32_t numBuffer )
{
    
    // if we have ever been accepted data 
    if ( _readPoll.getData()[ numBuffer ] > 0 )
        return _readPoll.getData()[ numBuffer ];

    uint32_t size;
    ib_wc_t    wc;

    ib_wc_t* wcDone; // A list of work completions retrieved from 
                     // the completion queue.
    ib_wc_t* wcFree; // a list of work completion structures provided by
                     // the client.
                     // These are used to report completed work requests through
                     // the pp_done_wclist

    wcFree = &wc;
    wcFree->p_next = 0;
    wcDone = 0;

    // Checks a completion queue for completed work requests
    ib_api_status_t ibStatus = _completionQueue->pollCQRead( &wcFree, &wcDone );

    if ( ibStatus == IB_SUCCESS )
    {
        if ( wcDone->status != IB_WCS_SUCCESS )
        {
            EQWARN << "ERROR IN POLL READ"<< std::endl;
            close();
            return -1;
        }
        _readPoll.getData()[ wcDone->wr_id ] = wcDone->length;
        wcFree = wcDone;
        wcFree->p_next = 0;
        wcDone = 0;

        if ( _readPoll.getData()[ numBuffer ] > 0)
            return _readPoll.getData()[ numBuffer ];

    }

    return 0;
}


int64_t IBInterface::postRdmaWrite( const void* buffer, uint32_t numBytes )
{
#ifdef EQ_MEASURE_TIME
    eq::lunchbox::Clock       clock;
    clock.reset();
#endif
    ib_api_status_t    ibStatus;

    ib_wc_t    wc;
    ib_wc_t    *wcDone,*wcFree;

    wcFree = &wc;
    wcFree->p_next = 0;
    wcDone = 0;
#ifdef EQ_MEASURE_TIME
    eq::lunchbox::Clock       clockWait;
    clockWait.reset();
#endif
    // validation of the send job
    do {
        ibStatus = ib_poll_cq( _completionQueue->getWriteHandle(), 
                               &wcFree, 
                               &wcDone );
        if ( ibStatus == IB_SUCCESS )
        {
            if ( wcDone->status != IB_WCS_SUCCESS )
            {
                EQWARN << "ERROR IN POLL WRITE"<< std::endl;
                return -1;
            }
            _writePoll.getData()[wcDone->wr_id] = true;
            wcFree = wcDone;
            wcFree->p_next = 0;
            wcDone = 0;
        }
        else if ( !_writePoll.getData()[ _numBufWrite ] )
        {
            ibStatus = IB_SUCCESS;
        }
    } while ( ibStatus == IB_SUCCESS );
#ifdef EQ_MEASURE_TIME
    _timeTotalWriteWait += clockWait.getTimef();
#endif
    uint32_t incBytes = 0;
    uint32_t compt = 0;
    uint32_t size;
    size = LB_MIN( numBytes, EQ_MAXBLOCKBUFFER );
    ib_local_ds_t    list;
#ifdef EQ_MEASURE_TIME
    eq::lunchbox::Clock       clockCopy;
    clockCopy.reset();
#endif
    
    //memcpy( _writeBlocks[ _numBufWrite ]->buf.getData(), 
    //                      buffer , size );
    eq::lunchbox::fastCopy( _writeBlocks[ _numBufWrite ]->buf.getData(), 
                             buffer , size );
    list.vaddr   = _writeBlocks[ _numBufWrite ]->getVaddr();
#ifdef EQ_MEASURE_TIME
    _timeCopyBufferWrite += clockCopy.getTimef();
#endif
    list.lkey    = _writeBlocks[_numBufWrite ]->getLocalKey();
    list.length  = size;
    
    // A 64-bit work request identifier that is returned to the consumer
    // as part of the work completion.
    _wr.wr_id    = _numBufWrite;
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

    // This routine posts a work request to the send queue of a queue pair
    ibStatus = ib_post_send( _queuePair, &_wr, 0 );
    if ( ibStatus != IB_SUCCESS )
    {
        EQWARN << "ERROR IN POST SEND DATA"<< std::endl;
        return -1;
    }
    _writePoll.getData()[ _numBufWrite ] = false;
    
    if ( _numBufWrite == EQ_NUMBLOCKMEMORY -1 )
        _ibConnection->incWriteInterface();
    
    _numBufWrite = ( _numBufWrite + 1 ) % EQ_NUMBLOCKMEMORY;
#ifdef EQ_MEASURE_TIME
    _timeTotalWrite += clock.getTimef();
#endif
    return size;
}
}
#endif //EQ_INFINIBAND
