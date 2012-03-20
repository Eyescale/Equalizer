
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
 *               2011, Carsten Rohn <carsten.rohn@rtt.ag> 
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

#ifndef CO_QUEUESLAVE_H
#define CO_QUEUESLAVE_H

#include "api.h"
#include "global.h" // used inline
#include "object.h" // base class
#include "types.h"

namespace co
{
namespace detail { class QueueSlave; }

/**
 * The consumer end of a distributed queue.
 *
 * One or more instances of this class are mapped to the identifier of the
 * QueueMaster registered on another node.
 */
class QueueSlave : public Object
{
public:
    /**
     * Construct a new queue consumer.
     *
     * The implementation will prefetch items from the queue master to cache
     * thems locally. The prefetchMark determines when new items are requested,
     * and the prefetchAmount how many items are fetched. Prefetching items
     * hides the network latency by pipelining the network communication with
     * the processing but may introduce imbalance between queue slaves if used
     * aggressively.
     *
     * @param prefetchMark the low-water mark for prefetching.
     * @param prefetchAmount the refill quantity when prefetching.
     * @version 1.1.6
     */
    CO_API QueueSlave( const uint32_t prefetchMark = 
                       Global::getIAttribute( Global::IATTR_QUEUE_MIN_SIZE ),
                       const uint32_t prefetchAmount = 
                       Global::getIAttribute( Global::IATTR_QUEUE_REFILL ));

    /** Destruct this new queue consumer. @version 1.1.6 */
    virtual CO_API ~QueueSlave();

    /**
     * @return an item from the distributed queue, or 0 if the queue is empty.
     * @version 1.1.6
     */
    CO_API CommandPtr pop();

private:
    detail::QueueSlave* const _impl;

    CO_API virtual void attach(const UUID& id, const uint32_t instanceID);

    virtual ChangeType getChangeType() const { return STATIC; }
    virtual void getInstanceData( co::DataOStream& ) { LBDONTCALL }
    virtual void applyInstanceData( co::DataIStream& is );
};

} // co

#endif // CO_QUEUESLAVE_H
