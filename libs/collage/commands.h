
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_COMMANDS_H
#define CO_COMMANDS_H

#include <co/base/os.h>

namespace co
{
    enum NodeCommand
    {
        CMD_NODE_STOP,
        CMD_NODE_MESSAGE,
        CMD_NODE_CONNECT,
        CMD_NODE_CONNECT_REPLY,
        CMD_NODE_CONNECT_ACK,
        CMD_NODE_ID,
        CMD_NODE_DISCONNECT,
        CMD_NODE_GET_NODE_DATA,
        CMD_NODE_GET_NODE_DATA_REPLY,
        CMD_NODE_ACQUIRE_SEND_TOKEN,
        CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY,
        CMD_NODE_RELEASE_SEND_TOKEN,
        CMD_NODE_ADD_LISTENER,
        CMD_NODE_REMOVE_LISTENER,
        CMD_NODE_ACK_REQUEST,
        CMD_NODE_FIND_MASTER_NODE_ID,
        CMD_NODE_FIND_MASTER_NODE_ID_REPLY,
        CMD_NODE_ATTACH_OBJECT,
        CMD_NODE_DETACH_OBJECT,
        CMD_NODE_REGISTER_OBJECT,
        CMD_NODE_DEREGISTER_OBJECT,
        CMD_NODE_MAP_OBJECT,
        CMD_NODE_MAP_OBJECT_SUCCESS,
        CMD_NODE_MAP_OBJECT_REPLY,
        CMD_NODE_UNMAP_OBJECT,
        CMD_NODE_UNSUBSCRIBE_OBJECT,
        CMD_NODE_OBJECT_INSTANCE,
        CMD_NODE_INSTANCE,
        CMD_NODE_CUSTOM = 40  // some buffer for binary-compatible patches
    };

    enum ObjectCommand
    {
        CMD_OBJECT_INSTANCE,
        CMD_OBJECT_DELTA,
        CMD_OBJECT_SLAVE_DELTA,
        CMD_OBJECT_COMMIT,
        CMD_OBJECT_CUSTOM = 10 // some buffer for binary-compatible patches
    };

    enum BarrierCommand
    {
        CMD_BARRIER_ENTER = CMD_OBJECT_CUSTOM,
        CMD_BARRIER_ENTER_REPLY,
        CMD_BARRIER_CUSTOM = 20 // some buffer for binary-compatible patches
    };
}

#endif // CO_COMMANDS_H

