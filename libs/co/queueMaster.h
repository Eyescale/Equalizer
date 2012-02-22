
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

#ifndef CO_QUEUEMASTER_H
#define CO_QUEUEMASTER_H

#include "object.h" // base class
#include "api.h"

namespace co
{
namespace detail { class QueueMaster; }

/**
 * The producer end of a distributed queue.
 *
 * One instance of this class is registered with a LocalNode to for the producer
 * end of a distributed queue. One or more QueueSlave instances are mapped to
 * this master instance and consume the data.
 */
class QueueMaster : public Object
{
public:
    /** Construct a new queue master. @version 1.1.6 */
    CO_API QueueMaster();

    /** Destruct this queue master. @version 1.1.6 */
    virtual CO_API ~QueueMaster();

    /**
     * Enqueue a new item.
     *
     * The enqueued item has to inherit from QueueItemPacket and be a flat
     * structure, that is, it should only contain POD data and no pointers. The
     * packet is copied as is over the network, using the packet's size
     * parameter. The packet is copied by this method.
     *
     * @param packet the item to enqueue.
     * @version 1.1.6
     */
    CO_API void push( const QueueItemPacket& packet );

    /** Remove all enqueued items. @version 1.1.6 */
    CO_API void clear();

private:
    detail::QueueMaster* const _impl;

    CO_API virtual void attach(const base::UUID& id, const uint32_t instanceID);

    virtual ChangeType getChangeType() const { return STATIC; }
    virtual void getInstanceData( co::DataOStream& os );
    virtual void applyInstanceData( co::DataIStream& ) { EQDONTCALL }
};

} // co

#endif // CO_QUEUEMASTER_H
