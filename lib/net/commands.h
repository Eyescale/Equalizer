
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_COMMANDS_H
#define EQNET_COMMANDS_H

#include <eq/base/base.h>

namespace eq
{
namespace net
{
    enum NodeCommand
    {
        CMD_NODE_STOP,
        CMD_NODE_MESSAGE,
        CMD_NODE_REGISTER_SESSION,
        CMD_NODE_REGISTER_SESSION_REPLY,
        CMD_NODE_MAP_SESSION,
        CMD_NODE_MAP_SESSION_REPLY,
        CMD_NODE_UNMAP_SESSION,
        CMD_NODE_UNMAP_SESSION_REPLY,
        CMD_NODE_SESSION,
        CMD_NODE_CONNECT,
        CMD_NODE_CONNECT_REPLY,
        CMD_NODE_DISCONNECT,
        CMD_NODE_GET_NODE_DATA,
        CMD_NODE_GET_NODE_DATA_REPLY,
        CMD_NODE_ACQUIRE_SEND_TOKEN,
        CMD_NODE_ACQUIRE_SEND_TOKEN_REPLY,
        CMD_NODE_RELEASE_SEND_TOKEN,
        CMD_NODE_FILL1, // some buffer for binary-compatible patches
        CMD_NODE_FILL2,
        CMD_NODE_FILL3,
        CMD_NODE_FILL4,
        CMD_NODE_FILL5,
        CMD_NODE_CUSTOM // must be last
    };

    enum SessionCommand
    {
        CMD_SESSION_ACK_REQUEST,
        CMD_SESSION_GEN_IDS,
        CMD_SESSION_GEN_IDS_REPLY,
        CMD_SESSION_SET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER_REPLY,
        CMD_SESSION_ATTACH_OBJECT,
        CMD_SESSION_DETACH_OBJECT,
        CMD_SESSION_MAP_OBJECT,
        CMD_SESSION_SUBSCRIBE_OBJECT,
        CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS,
        CMD_SESSION_SUBSCRIBE_OBJECT_REPLY,
        CMD_SESSION_UNSUBSCRIBE_OBJECT,
        CMD_SESSION_GET_OBJECT,
        CMD_SESSION_FILL1, // some buffer for binary-compatible patches
        CMD_SESSION_FILL2,
        CMD_SESSION_FILL3,
        CMD_SESSION_FILL4,
        CMD_SESSION_FILL5,
        CMD_SESSION_CUSTOM // must be last
    };

    enum ObjectCommand
    {
        CMD_OBJECT_INSTANCE_DATA,
        CMD_OBJECT_INSTANCE,
        CMD_OBJECT_DELTA_DATA,
        CMD_OBJECT_DELTA,
        CMD_OBJECT_COMMIT,
        CMD_OBJECT_NEW_MASTER,
        CMD_OBJECT_VERSION,
        CMD_OBJECT_FILL1, // some buffer for binary-compatible patches
        CMD_OBJECT_FILL2,
        CMD_OBJECT_FILL3,
        CMD_OBJECT_FILL4,
        CMD_OBJECT_FILL5,
        CMD_OBJECT_CUSTOM // must be last
    };

    enum BarrierCommand
    {
        CMD_BARRIER_ENTER = CMD_OBJECT_CUSTOM,
        CMD_BARRIER_ENTER_REPLY,
        CMD_BARRIER_FILL1, // some buffer for binary-compatible patches
        CMD_BARRIER_FILL2,
        CMD_BARRIER_FILL3,
        CMD_BARRIER_FILL4,
        CMD_BARRIER_FILL5,
        CMD_BARRIER_ALL
    };
}
}

#endif // EQNET_COMMANDS_H

