
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
#include <eq/base/lock.h>
#ifdef EQ_INFINIBAND

#ifndef EQNET_IBCOCOMPLETIONQUEUE_H
#define EQNET_IBCOCOMPLETIONQUEUE_H
#include <iba/ib_al.h>
#include <eq/base/atomic.h>
namespace co
{

class IBAdapter;
class IBConnection;
class IBCompletionQueue
{
public:
    /** Construct a new completion queues for read and write operations. */
    IBCompletionQueue( )
        : _cqR( 0 )
        , _cqWaitobjR( 0 )
        , _cqW( 0 )
        , _cqWaitobjW( 0 ){};


    virtual ~IBCompletionQueue();
    void close();

    /** Create and intialize Infinband completion queues*/
    bool create( IBConnection* myConnection,
                 const IBAdapter *adapter, 
                 const uint32_t size );

    /** reset read events */
    bool triggerRead() const;

    HANDLE getReadNotifier() const { return _cqWaitobjR; }
    HANDLE getWriteNotifier() const { return _cqWaitobjW; }

    // get a reference handle to the newly created completion queue read
    ib_cq_handle_t getReadHandle()  const { return _cqR; }
    // get a reference handle to the newly created completion queue write
    ib_cq_handle_t getWriteHandle() const { return _cqW; }
    
    void removeEventRead();

    ib_api_status_t pollCQRead( IN   OUT    ib_wc_t** const     pp_free_wclist,
                                OUT         ib_wc_t** const     pp_done_wclist );

    static void AL_API pp_cq_comp_cb( IN const ib_cq_handle_t h_cq,
                                      IN void  *cq_context );

    void resetEventRead();
    eq::base::Lock   _mutex;
    
private:

    ib_cq_handle_t      _cqR;
    HANDLE _cqWaitobjR;

    ib_cq_handle_t      _cqW;
    HANDLE _cqWaitobjW;

    IBConnection* _myConnection;

    ib_cq_handle_t _createNotifier( const IBAdapter* adapter,
                                          HANDLE     cqWaitobj,
                                    const uint32_t size );

    ib_cq_handle_t _createReadBack( const IBAdapter* adapter,
                                    const uint32_t size );
};
}
}
#endif //CO_IBCOMPLETIONQUEUE_H
#endif //EQ_INFINIBAND
