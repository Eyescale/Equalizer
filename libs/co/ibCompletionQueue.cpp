
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

#include "ibCompletionQueue.h"
#ifdef EQ_INFINIBAND

#include <eq/base/log.h>
#include <eq/base/debug.h>
#include <eq/base/scopedMutex.h>
#include "ibAdapter.h"
#include "ibConnection.h"



namespace co
{

IBCompletionQueue::~IBCompletionQueue()
{
    close();
}

void IBCompletionQueue::close()
{
    if( _cqR )
    {
        ib_destroy_cq( _cqR, 0 );
        _cqR = 0;
    }
    
    if( _cqW )
    {
        ib_destroy_cq( _cqW, 0 );
        _cqW = 0;
    }

    if ( _cqWaitobjR )
    {
        CloseHandle( _cqWaitobjR );
        _cqWaitobjR = 0;
    }

    if ( _cqWaitobjR )
    {
        CloseHandle( _cqWaitobjW );
        _cqWaitobjW = 0;
    }
}
bool IBCompletionQueue::create( IBConnection* myConnection,
                                const IBAdapter* adapter, 
                                const uint32_t size )
{
    _myConnection = myConnection;

    // build a completion queue for receive part
    _cqWaitobjR = CreateEvent( 0, true, false, 0 );
    _cqR = _createReadBack( adapter, size );
    if ( !_cqR )
        return false;
    
    // build a completion queue for send part
    _cqWaitobjW = CreateEvent( 0, false, false, 0 );
    _cqW = _createNotifier( adapter, _cqWaitobjW, size );
    if ( !_cqW )
        return false;
    return true;
}


ib_cq_handle_t IBCompletionQueue::_createNotifier( 
                                    const IBAdapter*     adapter,
                                          HANDLE         cqWaitobj,
                                          const uint32_t size)
{
    // build a completion queue for send part
    ib_cq_create_t    cqCreate;
    cqCreate.h_wait_obj  = cqWaitobj;
    cqCreate.pfn_comp_cb = 0;
    cqCreate.size        = size;

    ib_cq_handle_t _cq;
    // Creates a completion queue
    ib_api_status_t ibStatus = ib_create_cq( adapter->getHandle(), 
                                             &cqCreate, 0, 0, &_cq );
    if( ibStatus )
    {
        EQERROR << "Can't create CQ" << std::endl;
        ib_destroy_cq( _cq, 0 );
        return 0;
    }

    return _cq;
}

ib_cq_handle_t IBCompletionQueue::_createReadBack( 
                                    const IBAdapter*     adapter, 
                                    const uint32_t size)
{
    // build a completion queue for send part
    ib_cq_create_t    cqCreate;
    cqCreate.h_wait_obj  = 0;
    cqCreate.pfn_comp_cb = pp_cq_comp_cb;
    cqCreate.size        = size;

    ib_cq_handle_t _cq;
    // Creates a completion queue
    ib_api_status_t ibStatus = ib_create_cq( adapter->getHandle(), 
                                             &cqCreate, this, 0, &_cq);
    if( ibStatus )
    {
        EQERROR << "Can't create CQ" << std::endl;
        ib_destroy_cq( _cq, 0 );
        return 0;
    }
    return _cq;
}

ib_api_status_t IBCompletionQueue::pollCQRead( 
                            IN   OUT    ib_wc_t** const freeWclist,
                            OUT         ib_wc_t** const doneWclist)
{
    eq::base::ScopedMutex mutex( _mutex );
    return ib_poll_cq( getReadHandle(), freeWclist, doneWclist);
}


bool IBCompletionQueue::triggerRead() const
{
    const ib_api_status_t ibStatus  = ib_rearm_cq( _cqR, false );
    if( ibStatus )
    {
        EQERROR << "Could not rearm handle read Event" << std::endl; 
        return false;
    }
    return true;
}


void AL_API IBCompletionQueue::pp_cq_comp_cb( 
                           IN const ib_cq_handle_t h_cq,
                           IN       void  *cq_context )
{
    
    IBCompletionQueue* cq = reinterpret_cast<IBCompletionQueue*>
                            ( cq_context );
    eq::base::ScopedMutex mutex( cq->_mutex );
    
    cq->_myConnection->addEvent();

    return ;
}
void IBCompletionQueue::removeEventRead()
{
    eq::base::ScopedMutex mutex( _mutex );
    _myConnection->removeEvent();
}

void IBCompletionQueue::resetEventRead()
{
    eq::base::ScopedMutex mutex( _mutex );
    triggerRead();
    ResetEvent( getReadNotifier() );
}
}
#endif //EQ_INFINIBAND
