
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

#ifndef EQ_COMMANDS_H
#define EQ_COMMANDS_H

#include <eq/net/commands.h>

namespace eq
{
/** @cond IGNORE */
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG        = net::CMD_NODE_CUSTOM,
        CMD_SERVER_USE_CONFIG,
        CMD_SERVER_CHOOSE_CONFIG_REPLY,
        CMD_SERVER_CREATE_CONFIG,
        CMD_SERVER_CREATE_CONFIG_REPLY,
        CMD_SERVER_DESTROY_CONFIG,
        CMD_SERVER_RELEASE_CONFIG,
        CMD_SERVER_RELEASE_CONFIG_REPLY,
        CMD_SERVER_INIT_CONFIG,
        CMD_SERVER_SHUTDOWN,
        CMD_SERVER_SHUTDOWN_REPLY,
        CMD_SERVER_FILL1, // some buffer for binary-compatible patches
        CMD_SERVER_FILL2,
        CMD_SERVER_FILL3,
        CMD_SERVER_FILL4,
        CMD_SERVER_FILL5,
        CMD_SERVER_CUSTOM
    };

    enum ClientCommand
    {
        CMD_CLIENT_EXIT               = net::CMD_NODE_CUSTOM,
        CMD_CLIENT_FILL1, // some buffer for binary-compatible patches
        CMD_CLIENT_FILL2,
        CMD_CLIENT_FILL3,
        CMD_CLIENT_FILL4,
        CMD_CLIENT_FILL5,
        CMD_CLIENT_CUSTOM
    };

    enum ConfigCommand
    {
        CMD_CONFIG_START_INIT                 = net::CMD_SESSION_CUSTOM,
        CMD_CONFIG_START_INIT_REPLY,
        CMD_CONFIG_INIT,
        CMD_CONFIG_INIT_REPLY,
        CMD_CONFIG_EXIT,
        CMD_CONFIG_EXIT_REPLY,
        CMD_CONFIG_CREATE_REPLY,
        CMD_CONFIG_CREATE_NODE,
        CMD_CONFIG_DESTROY_NODE,
        CMD_CONFIG_START_FRAME,
        CMD_CONFIG_RELEASE_FRAME_LOCAL,
        CMD_CONFIG_FRAME_FINISH,
        CMD_CONFIG_FINISH_ALL_FRAMES,
        CMD_CONFIG_EVENT,
        CMD_CONFIG_FREEZE_LOAD_BALANCING,
        CMD_CONFIG_SYNC_CLOCK,
        CMD_CONFIG_UNMAP,
        CMD_CONFIG_UNMAP_REPLY,
        CMD_CONFIG_FILL1, // some buffer for binary-compatible patches
        CMD_CONFIG_FILL2,
        CMD_CONFIG_FILL3,
        CMD_CONFIG_FILL4,
        CMD_CONFIG_FILL5,
        CMD_CONFIG_CUSTOM
    };

    enum NodeCommand
    {
        CMD_NODE_CONFIG_INIT = net::CMD_OBJECT_CUSTOM,
        CMD_NODE_CONFIG_INIT_REPLY,
        CMD_NODE_CONFIG_EXIT,
        CMD_NODE_CONFIG_EXIT_REPLY,
        CMD_NODE_CREATE_PIPE,
        CMD_NODE_DESTROY_PIPE,
        CMD_NODE_FRAME_START, 
        CMD_NODE_FRAME_FINISH,
        CMD_NODE_FRAME_FINISH_REPLY,
        CMD_NODE_FRAME_DRAW_FINISH,
        CMD_NODE_FRAME_TASKS_FINISH,
        CMD_NODE_FILL1, // some buffer for binary-compatible patches
        CMD_NODE_FILL2,
        CMD_NODE_FILL3,
        CMD_NODE_FILL4,
        CMD_NODE_FILL5,
        CMD_NODE_CUSTOM
    };

    enum PipeCommand
    {
        CMD_PIPE_CONFIG_INIT = net::CMD_OBJECT_CUSTOM,
        CMD_PIPE_CONFIG_INIT_REPLY,
        CMD_PIPE_CONFIG_EXIT,
        CMD_PIPE_CONFIG_EXIT_REPLY, 
        CMD_PIPE_CREATE_WINDOW,
        CMD_PIPE_DESTROY_WINDOW,
        CMD_PIPE_FRAME_START,
        CMD_PIPE_FRAME_FINISH,
        CMD_PIPE_FRAME_DRAW_FINISH,
        CMD_PIPE_FRAME_START_CLOCK,
        CMD_PIPE_FILL1, // some buffer for binary-compatible patches
        CMD_PIPE_FILL2,
        CMD_PIPE_FILL3,
        CMD_PIPE_FILL4,
        CMD_PIPE_FILL5,
        CMD_PIPE_CUSTOM
    };

    enum WindowCommand
    {
        CMD_WINDOW_CONFIG_INIT = net::CMD_OBJECT_CUSTOM,
        CMD_WINDOW_CONFIG_INIT_REPLY,
        CMD_WINDOW_CONFIG_EXIT,
        CMD_WINDOW_CONFIG_EXIT_REPLY,
        CMD_WINDOW_CREATE_CHANNEL,
        CMD_WINDOW_DESTROY_CHANNEL,
        CMD_WINDOW_SET_PVP,
        CMD_WINDOW_FRAME_START,
        CMD_WINDOW_FRAME_FINISH,
        CMD_WINDOW_FINISH,
        CMD_WINDOW_THROTTLE_FRAMERATE,
        CMD_WINDOW_BARRIER,
        CMD_WINDOW_NV_BARRIER,
        CMD_WINDOW_SWAP,
        CMD_WINDOW_FRAME_DRAW_FINISH,
        CMD_WINDOW_FILL1, // some buffer for binary-compatible patches
        CMD_WINDOW_FILL2,
        CMD_WINDOW_FILL3,
        CMD_WINDOW_FILL4,
        CMD_WINDOW_FILL5,
        CMD_WINDOW_CUSTOM
    };

    enum ChannelCommand
    {
        CMD_CHANNEL_CONFIG_INIT = net::CMD_OBJECT_CUSTOM,
        CMD_CHANNEL_CONFIG_INIT_REPLY,
        CMD_CHANNEL_CONFIG_EXIT,
        CMD_CHANNEL_CONFIG_EXIT_REPLY,
        CMD_CHANNEL_SET_NEARFAR,
        CMD_CHANNEL_FRAME_START,
        CMD_CHANNEL_FRAME_FINISH,
        CMD_CHANNEL_FRAME_FINISH_REPLY,
        CMD_CHANNEL_FRAME_CLEAR,
        CMD_CHANNEL_FRAME_DRAW,
        CMD_CHANNEL_FRAME_DRAW_FINISH,
        CMD_CHANNEL_FRAME_ASSEMBLE,
        CMD_CHANNEL_FRAME_READBACK,
        CMD_CHANNEL_FRAME_TRANSMIT,
        CMD_CHANNEL_FRAME_VIEW_START,
        CMD_CHANNEL_FRAME_VIEW_FINISH,
        CMD_CHANNEL_FILL1, // some buffer for binary-compatible patches
        CMD_CHANNEL_FILL2,
        CMD_CHANNEL_FILL3,
        CMD_CHANNEL_FILL4,
        CMD_CHANNEL_FILL5,
        CMD_CHANNEL_CUSTOM
    };

    enum FrameDataCommand
    {
        CMD_FRAMEDATA_TRANSMIT = net::CMD_OBJECT_CUSTOM,
        CMD_FRAMEDATA_READY,
        CMD_FRAMEDATA_UPDATE,
        CMD_FRAMEDATA_FILL1, // some buffer for binary-compatible patches
        CMD_FRAMEDATA_FILL2,
        CMD_FRAMEDATA_FILL3,
        CMD_FRAMEDATA_FILL4,
        CMD_FRAMEDATA_FILL5,
        CMD_FRAMEDATA_CUSTOM
    };
/** @endcond */
};

#endif // EQ_COMMANDS_H

