
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

#include "object.h"
#include "api.h"

namespace co
{

class QueueSlave : public Object
{
public:

    CO_API QueueSlave();
    ~QueueSlave() {}

    CO_API virtual void attach( const base::UUID& id, 
        const uint32_t instanceID );

    CO_API Command* pop();

protected:
    virtual ChangeType getChangeType() const { return STATIC; }

private:
    CommandQueue _queue;

    uint32_t _prefetchLow;
    uint32_t _prefetchHigh;
};

} // co

#endif // CO_QUEUESLAVE_H
