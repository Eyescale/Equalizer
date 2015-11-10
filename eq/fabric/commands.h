
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder  <cedric Stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_COMMANDS_H
#define EQFABRIC_COMMANDS_H

#include <co/commands.h>

namespace eq
{
namespace fabric
{
/** @cond IGNORE */
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG        = co::CMD_NODE_CUSTOM, // 50
        CMD_SERVER_CHOOSE_CONFIG_REPLY,
        CMD_SERVER_CREATE_CONFIG,
        CMD_SERVER_CREATE_CONFIG_REPLY,
        CMD_SERVER_DESTROY_CONFIG,
        CMD_SERVER_DESTROY_CONFIG_REPLY,
        CMD_SERVER_RELEASE_CONFIG,
        CMD_SERVER_RELEASE_CONFIG_REPLY,
        CMD_SERVER_INIT_CONFIG,
        CMD_SERVER_SHUTDOWN,
        CMD_SERVER_SHUTDOWN_REPLY,
        // admin
        CMD_SERVER_MAP,
        CMD_SERVER_MAP_REPLY,
        CMD_SERVER_UNMAP,
        CMD_SERVER_UNMAP_REPLY,
        CMD_SERVER_CUSTOM = co::CMD_NODE_CUSTOM + 20 // 70, buffer for changes
    };

    enum ClientCommand
    {
        CMD_CLIENT_EXIT = CMD_SERVER_CUSTOM, // 70
        CMD_CLIENT_INTERRUPT,
        CMD_CLIENT_CUSTOM = CMD_SERVER_CUSTOM + 5
    };

    enum ObjectICommand
    {
        CMD_OBJECT_SYNC = co::CMD_OBJECT_CUSTOM, // 10
        CMD_OBJECT_CUSTOM = co::CMD_OBJECT_CUSTOM + 5
    };

    enum ConfigCommand
    {
        CMD_CONFIG_NEW_LAYOUT = CMD_OBJECT_CUSTOM, // 15
        CMD_CONFIG_NEW_CANVAS,
        CMD_CONFIG_NEW_OBSERVER,
        CMD_CONFIG_NEW_ENTITY_REPLY,
        CMD_CONFIG_START_INIT,
        CMD_CONFIG_START_INIT_REPLY,
        CMD_CONFIG_INIT,
        CMD_CONFIG_INIT_REPLY,
        CMD_CONFIG_EXIT,
        CMD_CONFIG_EXIT_REPLY,
        CMD_CONFIG_UPDATE,
        CMD_CONFIG_UPDATE_VERSION,
        CMD_CONFIG_UPDATE_REPLY,
        CMD_CONFIG_CREATE_REPLY,
        CMD_CONFIG_CREATE_NODE,
        CMD_CONFIG_DESTROY_NODE,
        CMD_CONFIG_START_FRAME,
        CMD_CONFIG_RELEASE_FRAME_LOCAL,
        CMD_CONFIG_FRAME_FINISH,
        CMD_CONFIG_FINISH_ALL_FRAMES,
        CMD_CONFIG_STOP_FRAMES,
        CMD_CONFIG_EVENT,
        CMD_CONFIG_EVENT_OLD,
        CMD_CONFIG_SYNC_CLOCK,
        CMD_CONFIG_SWAP_OBJECT,
        CMD_CONFIG_CHECK_FRAME,
        CMD_CONFIG_CUSTOM = CMD_OBJECT_CUSTOM + 30
    };

    enum NodeCommand
    {
        CMD_NODE_CONFIG_INIT = CMD_OBJECT_CUSTOM, // 15
        CMD_NODE_CONFIG_INIT_REPLY,
        CMD_NODE_SET_AFFINITY,
        CMD_NODE_CONFIG_EXIT,
        CMD_NODE_CONFIG_EXIT_REPLY,
        CMD_NODE_CREATE_PIPE,
        CMD_NODE_DESTROY_PIPE,
        CMD_NODE_FRAME_START,
        CMD_NODE_FRAME_FINISH,
        CMD_NODE_FRAME_FINISH_REPLY,
        CMD_NODE_FRAME_DRAW_FINISH,
        CMD_NODE_FRAME_TASKS_FINISH,
        CMD_NODE_FRAMEDATA_TRANSMIT,
        CMD_NODE_FRAMEDATA_READY,
        CMD_NODE_CUSTOM = CMD_OBJECT_CUSTOM + 20
    };

    enum PipeCommand
    {
        CMD_PIPE_NEW_WINDOW = CMD_OBJECT_CUSTOM, // 15
        CMD_PIPE_NEW_WINDOW_REPLY,
        CMD_PIPE_CONFIG_INIT,
        CMD_PIPE_CONFIG_INIT_REPLY,
        CMD_PIPE_CONFIG_EXIT,
        CMD_PIPE_CONFIG_EXIT_REPLY,
        CMD_PIPE_CREATE_WINDOW,
        CMD_PIPE_DESTROY_WINDOW,
        CMD_PIPE_FRAME_START,
        CMD_PIPE_FRAME_FINISH,
        CMD_PIPE_FRAME_DRAW_FINISH,
        CMD_PIPE_FRAME_START_CLOCK,
        CMD_PIPE_EXIT_THREAD,
        CMD_PIPE_EXIT_TRANSFER_THREAD,
        CMD_PIPE_DETACH_VIEW,
        CMD_PIPE_CUSTOM = CMD_OBJECT_CUSTOM + 20
    };

    enum WindowCommand
    {
        CMD_WINDOW_NEW_CHANNEL = CMD_OBJECT_CUSTOM, // 15
        CMD_WINDOW_NEW_CHANNEL_REPLY,
        CMD_WINDOW_CONFIG_INIT,
        CMD_WINDOW_CONFIG_INIT_REPLY,
        CMD_WINDOW_CONFIG_EXIT,
        CMD_WINDOW_CONFIG_EXIT_REPLY,
        CMD_WINDOW_CREATE_CHANNEL,
        CMD_WINDOW_DESTROY_CHANNEL,
        CMD_WINDOW_FRAME_START,
        CMD_WINDOW_FRAME_FINISH,
        CMD_WINDOW_FLUSH,
        CMD_WINDOW_FINISH,
        CMD_WINDOW_THROTTLE_FRAMERATE,
        CMD_WINDOW_BARRIER,
        CMD_WINDOW_NV_BARRIER,
        CMD_WINDOW_SWAP,
        CMD_WINDOW_FRAME_DRAW_FINISH,
        CMD_WINDOW_CREATE_QGL_WIDGET,
        CMD_WINDOW_DESTROY_QGL_WIDGET,
        CMD_WINDOW_CUSTOM = CMD_OBJECT_CUSTOM + 20
    };

    enum ChannelCommand
    {
        CMD_CHANNEL_CONFIG_INIT = CMD_OBJECT_CUSTOM, // 15
        CMD_CHANNEL_CONFIG_INIT_REPLY,
        CMD_CHANNEL_CONFIG_EXIT,
        CMD_CHANNEL_CONFIG_EXIT_REPLY,
        CMD_CHANNEL_FRAME_START,
        CMD_CHANNEL_FRAME_FINISH,
        CMD_CHANNEL_FRAME_FINISH_REPLY,
        CMD_CHANNEL_FRAME_CLEAR,
        CMD_CHANNEL_FRAME_DRAW,
        CMD_CHANNEL_FRAME_DRAW_FINISH,
        CMD_CHANNEL_FRAME_ASSEMBLE,
        CMD_CHANNEL_FRAME_READBACK,
        CMD_CHANNEL_FRAME_TRANSMIT_IMAGE,
        CMD_CHANNEL_FRAME_SET_READY,
        CMD_CHANNEL_FRAME_SET_READY_NODE,
        CMD_CHANNEL_FRAME_VIEW_START,
        CMD_CHANNEL_FRAME_VIEW_FINISH,
        CMD_CHANNEL_STOP_FRAME,
        CMD_CHANNEL_FRAME_TILES,
        CMD_CHANNEL_FINISH_READBACK,
        CMD_CHANNEL_DELETE_TRANSFER_WINDOW,
        CMD_CHANNEL_CUSTOM = CMD_OBJECT_CUSTOM + 30
    };

    enum CanvasCommand
    {
        CMD_CANVAS_NEW_SEGMENT = CMD_OBJECT_CUSTOM, // 15
        CMD_CANVAS_NEW_SEGMENT_REPLY,
        CMD_CANVAS_CUSTOM = CMD_OBJECT_CUSTOM + 5
    };

    enum LayoutCommand
    {
        CMD_LAYOUT_NEW_VIEW = CMD_OBJECT_CUSTOM, // 15
        CMD_LAYOUT_NEW_VIEW_REPLY,
        CMD_LAYOUT_CUSTOM = CMD_OBJECT_CUSTOM + 5
    };

    enum ViewCommand
    {
        CMD_VIEW_CUSTOM = CMD_OBJECT_CUSTOM + 5
    };
/** @endcond */
}
}
#endif // EQFABRIC_COMMANDS_H
