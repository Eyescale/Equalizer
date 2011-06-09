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

#include "packets.h"

#include "queueMaster.h"

namespace co
{

QueueMaster::QueueMaster()
: Object()
, _cache()
, _queue()
{

}

void QueueMaster::attach( const base::UUID& id, const uint32_t instanceID )
{
    Object::attach(id, instanceID);

    CommandQueue* queue = getLocalNode()->getCommandThreadQueue();
    registerCommand( CMD_GET_QUEUE_ITEM, 
        CommandFunc<QueueMaster>( this, &QueueMaster::_cmdGetItem ), queue );
}

void QueueMaster::push( const QueueItemPacket& packet )
{
    Command& queueCommand = 
            _cache.alloc( getLocalNode(), getLocalNode(), packet.size );
    QueueItemPacket* queuePacket = queueCommand.get< QueueItemPacket >();

    memcpy( queuePacket, &packet, packet.size );
    queuePacket->objectID = getID();
    _queue.push_back( &queueCommand );
}

Command& QueueMaster::pop()
{
    Command* cmd = _queue.front();
    _queue.pop_front();
    return *cmd;
}

bool QueueMaster::_cmdGetItem( Command& command )
{
    GetQueueItemPacket* packet = command.get<GetQueueItemPacket>();
    uint32_t itemsRequested = packet->itemsRequested;

    while (!_queue.empty() || (--itemsRequested > 0) )
    {
        Command* queueItem = _queue.front();
        send( command.getNode(), *(queueItem->get<ObjectPacket>()) );
        _queue.pop_front();
    }

    if ( itemsRequested > 0 )
    {
        QueueEmptyPacket queueEmpty;
        send( command.getNode(), queueEmpty );
    }

    return true;
}

} // co
