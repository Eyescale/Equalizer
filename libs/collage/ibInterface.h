/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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
#include <eq/base/base.h>
#ifdef EQ_INFINIBAND
#ifndef CO_IBINTERFACE_H
#define CO_IBINTERFACE_H
#include "ibMemBlock.h"
#include "ibAdapter.h"
#include "ibCompletionQueue.h"
#include <iba/ib_types.h>
#include <iba/ib_al.h>
#include <eq/base/buffer.h>
#include <vector>

namespace co
{

struct IBDest 
{
    uint16_t    lid;
    uint32_t    qpn;
    uint32_t    psn;
    uint32_t    rkey;
    uint64_t    vaddr;
};

class IBAdapter;
class IBCompletionQueue;
class IBMemBlock;
//
// A class that defines a point-to-point reliable connection between 2 HCAs
class IBInterface  
{
public:
    IBInterface( );

    virtual ~IBInterface();

    // 1st stage of creating a connection
    bool create( IBAdapter* hca, 
                 IBCompletionQueue* cq,
                 IBConnection* ibConnection );
    struct IBDest getMyDest( uint32_t index );
    // record info which needed to complete connection
    void setDestInfo( struct IBDest *dest, uint32_t index )
            { _dests.getData()[index] = *dest; }
    // complete creation of connection
    void modiyQueuePairAttribute( );
    // destroy a connection
    void close();
    void readNB( void* buffer, const uint64_t bytes );
    // post a remote direct memory access write
    //  also call a one-sided transfer
    int64_t  postRdmaWrite(
               const void* buffer, uint32_t numBytes );

    int64_t _waitPollCQ( uint32_t numBuffer );
    // post cqes to record rdma writes completing on receiving host
    int64_t  readSync( void* buffer, uint32_t bytes);

    ib_qp_handle_t getQueuePair(){ return _queuePair; }
    int getSQpNum() { return _queuePairAttr.num; }

    uint32_t getNumberBlockMemory();

private:

    IBCompletionQueue*      _completionQueue;
    IBConnection*            _ibConnection;
    std::vector< IBMemBlock *>  _readBlocks;
    std::vector< IBMemBlock *>  _writeBlocks;
    uint32_t                _psn; 
    uint16_t                _dlid;
    ib_qp_handle_t          _queuePair;
    ib_qp_attr_t            _queuePairAttr;
    uint64_t                _guid;
    ib_send_wr_t            _wr;
    ib_recv_wr_t            _rwr;
    eq::base::Buffer< IBDest > _dests;
    ib_local_ds_t           _list;
    ib_local_ds_t           _recvList;
    uint32_t                _posReadInBuffer;
    bool                    _firstTimeWrite;
    eq::base::Buffer< bool >     _writePoll;
    eq::base::Buffer< int32_t >  _readPoll;
    uint32_t                _numBufRead;
    uint32_t                _numBufWrite;
    uint32_t                _countWrite;
    bool _ibPostRecv( uint32_t numBuffer );
    bool _createQueuePair( IBAdapter* adapter );
    bool _setAttributeReadyToReceive();
    bool _setAttributeReadyToSend( );

private:

    float _floatTimeReadNB;
    float _floatTimeReadSync;
    
    float _timeTotalWaitPoll;
    float _timeTotalWaitobj;

    float _timeTotalWrite;
    float _timeTotalWriteWait;

    float _timeCopyBufferRead;
    float _timeCopyBufferWrite;

};
}
#endif //CO_IBCONNECTION_H
#endif //EQ_INFINIBAND
