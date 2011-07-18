
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

#include "queueSlave.h"

#include "command.h"
#include "commands.h"
#include "dataIStream.h"
#include "global.h"
#include "packets.h"

namespace co
{

QueueSlave::QueueSlave()
        : Object()
        , _prefetchLow( Global::getIAttribute( Global::IATTR_QUEUE_MIN_SIZE ))
        , _prefetchHigh( Global::getIAttribute( Global::IATTR_QUEUE_MAX_SIZE ))
        , _masterInstanceID( EQ_INSTANCE_ALL )
{
}

QueueSlave::~QueueSlave()
{
    EQASSERT( _queue.isEmpty( ));
    while (!_queue.isEmpty())
    {
        Command* cmd = _queue.pop();
        cmd->release();
    }
}

void QueueSlave::attach( const base::UUID& id, const uint32_t instanceID )
{
    Object::attach(id, instanceID);
    registerCommand( CMD_QUEUE_ITEM, CommandFunc<Object>(0, 0), &_queue);
    registerCommand( CMD_QUEUE_EMPTY, CommandFunc<Object>(0, 0), &_queue);
}

void QueueSlave::applyInstanceData( co::DataIStream& is )
{
    uint128_t masterNodeID;
    is >> _masterInstanceID >> masterNodeID;

    EQASSERT( masterNodeID != NodeID::ZERO );
    EQASSERT( !_master );
    LocalNodePtr localNode = getLocalNode();
    _master = localNode->connect( masterNodeID );
}

Command* QueueSlave::pop()
{
    if ( _queue.getSize() <= _prefetchLow )
    {
        QueueGetItemPacket packet;
        uint32_t queueSize = static_cast<uint32_t>(_queue.getSize());
        packet.itemsRequested = _prefetchHigh - queueSize;
        packet.instanceID = _masterInstanceID;
        packet.slaveInstanceID = getInstanceID();
        send( _master, packet );
    }
    
    Command* cmd = _queue.pop();
    if ((*cmd)->command == CMD_QUEUE_ITEM)
        return cmd;
    
    cmd->release();
    return 0;
}

}
