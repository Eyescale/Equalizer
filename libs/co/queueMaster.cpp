
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

#include "queueMaster.h"

#include "dataOStream.h"
#include "packets.h"

namespace co
{

QueueMaster::QueueMaster()
        : Object()
        , _queue()
        , _cache()
{
}

QueueMaster::~QueueMaster()
{
    clear();
}

void QueueMaster::attach( const base::UUID& id, const uint32_t instanceID )
{
    Object::attach(id, instanceID);

    CommandQueue* queue = getLocalNode()->getCommandThreadQueue();
    registerCommand( CMD_QUEUE_GET_ITEM, 
        CommandFunc<QueueMaster>( this, &QueueMaster::_cmdGetItem ), queue );
}

void QueueMaster::clear()
{
    while( !_queue.empty( ))
    {
        Command* cmd = _queue.front();
        _queue.pop_front();
        cmd->release();
    }

    _cache.flush();
}

void QueueMaster::getInstanceData( co::DataOStream& os )
{
    os << getInstanceID() << getLocalNode()->getNodeID();
}

void QueueMaster::push( const QueueItemPacket& packet )
{
    EQ_TS_SCOPED( _thread );

    Command& queueCommand = _cache.alloc( getLocalNode(), getLocalNode(),
                                          packet.size );
    QueueItemPacket* queuePacket =
        queueCommand.getModifiable< QueueItemPacket >();

    memcpy( queuePacket, &packet, packet.size );
    queuePacket->objectID = getID();
    queueCommand.retain();
    _queue.push_back( &queueCommand );
}

Command& QueueMaster::pop()
{
    EQ_TS_SCOPED( _thread );
    Command* cmd = _queue.front();
    _queue.pop_front();
    return *cmd;
}

bool QueueMaster::_cmdGetItem( Command& command )
{
    EQ_TS_SCOPED( _thread );
    const QueueGetItemPacket* packet = command.get< QueueGetItemPacket >();
    uint32_t itemsRequested = packet->itemsRequested;

    while( !_queue.empty() && itemsRequested )
    {
        Command* queueItem = _queue.front();
        _queue.pop_front();
        ObjectPacket* queuePacket = queueItem->getModifiable< ObjectPacket >();
        queuePacket->instanceID = packet->slaveInstanceID;
        send( command.getNode(), *queuePacket );
        --itemsRequested;
        queueItem->release();
    }

    if( itemsRequested > 0 )
    {
        QueueEmptyPacket queueEmpty;
        queueEmpty.instanceID = packet->slaveInstanceID;
        queueEmpty.objectID = getID();
        send( command.getNode(), queueEmpty );
    }

    return true;
}

} // co
