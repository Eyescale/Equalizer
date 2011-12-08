
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef CO_QUEUEPACKETS_H
#define CO_QUEUEPACKETS_H

#include <co/packets.h> // base class

namespace co
{
    struct QueueItemPacket : public ObjectPacket
    {
        QueueItemPacket()
            : ObjectPacket()
        {
            command = CMD_QUEUE_ITEM;
            size = sizeof( QueueItemPacket );
        }
    };

    struct QueueGetItemPacket : public ObjectPacket
    {
        QueueGetItemPacket()
            : ObjectPacket()
            , itemsRequested( 0u )
            , slaveInstanceID( EQ_INSTANCE_ALL )
        {
            command = CMD_QUEUE_GET_ITEM;
            size = sizeof( QueueGetItemPacket );
        }
        uint32_t itemsRequested;
        uint32_t slaveInstanceID;
    };
    
    struct QueueEmptyPacket : public ObjectPacket
    {
        QueueEmptyPacket()
            : ObjectPacket()
        {
            command = CMD_QUEUE_EMPTY;
            size = sizeof( QueueEmptyPacket );
        }
    };
}

#endif // CO_QUEUEPACKETS_H

