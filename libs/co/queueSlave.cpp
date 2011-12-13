
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
#include "queuePackets.h"

namespace co
{
namespace detail
{
class QueueSlave
{
public:
    QueueSlave()
            : prefetchLow(Global::getIAttribute( Global::IATTR_QUEUE_MIN_SIZE ))
            , prefetchHigh(Global::getIAttribute( Global::IATTR_QUEUE_MAX_SIZE))
            , masterInstanceID( EQ_INSTANCE_ALL )
        {}

    CommandQueue queue;

    uint32_t prefetchLow;
    uint32_t prefetchHigh;
    uint32_t masterInstanceID;

    NodePtr master;
};
}

QueueSlave::QueueSlave()
        : _impl( new detail::QueueSlave )
{}

QueueSlave::~QueueSlave()
{
    while( !_impl->queue.isEmpty( ))
    {
        Command* cmd = _impl->queue.pop();
        EQASSERT( (*cmd)->command == CMD_QUEUE_EMPTY );
        cmd->release();
    }
    delete _impl;
}

void QueueSlave::attach( const base::UUID& id, const uint32_t instanceID )
{
    Object::attach(id, instanceID);
    registerCommand( CMD_QUEUE_ITEM, CommandFunc<Object>(0, 0), &_impl->queue );
    registerCommand( CMD_QUEUE_EMPTY, CommandFunc<Object>(0, 0), &_impl->queue);
}

void QueueSlave::applyInstanceData( co::DataIStream& is )
{
    uint128_t masterNodeID;
    is >> _impl->masterInstanceID >> masterNodeID;

    EQASSERT( masterNodeID != NodeID::ZERO );
    EQASSERT( !_impl->master );
    LocalNodePtr localNode = getLocalNode();
    _impl->master = localNode->connect( masterNodeID );
}

Command* QueueSlave::pop()
{
    static base::a_int32_t _request;
    const int32_t request = ++_request;

    while( true )
    {
        const uint32_t queueSize( _impl->queue.getSize( ));
        if( queueSize <= _impl->prefetchLow )
        {
            QueueGetItemPacket packet;
            packet.itemsRequested = _impl->prefetchHigh - queueSize;
            packet.instanceID = _impl->masterInstanceID;
            packet.slaveInstanceID = getInstanceID();
            packet.requestID = request;
            send( _impl->master, packet );
        }

        Command* cmd = _impl->queue.pop();
        if( (*cmd)->command == CMD_QUEUE_ITEM )
            return cmd;
    
        EQASSERT( (*cmd)->command == CMD_QUEUE_EMPTY );
        const QueueEmptyPacket* packet = cmd->get< QueueEmptyPacket >();
        if( packet->requestID == request )
        {
            cmd->release();
            return 0;
        }
        // else left-over or not our empty packet, discard and retry
        cmd->release();
    }
}

}
