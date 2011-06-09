/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Carsten Rohn <carsten.rohn@rtt.ag> 
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

#include "command.h"
#include "object.h"
#include "api.h"


namespace co
{

class QueueMaster : public Object
{
public:
    QueueMaster();
    ~QueueMaster() {};

    Command& pop();
    CO_API void push( const QueueItemPacket& packet );

    CO_API virtual void attach( const base::UUID& id, 
        const uint32_t instanceID );

protected:
    virtual ChangeType getChangeType() const { return STATIC; }

private:
    typedef std::deque< Command* > PacketQueue;

    PacketQueue _queue;
    CommandCache _cache;

    /** The command handler functions. */
    bool _cmdGetItem( Command& command );
};

} // co

#endif // CO_QUEUEMASTER_H