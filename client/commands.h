
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COMMANDS_H
#define EQ_COMMANDS_H

#include <eq/net/commands.h>

namespace eq
{
    enum ServerCommand
    {
        CMD_SERVER_CHOOSE_CONFIG       = eqNet::CMD_NODE_CUSTOM,
        CMD_SERVER_CHOOSE_CONFIG_REPLY,
        CMD_SERVER_INIT_CONFIG,
        REQ_SERVER_INIT_CONFIG, // REQ must follow CMD
        CMD_SERVER_RELEASE_CONFIG,
        CMD_SERVER_ALL
    };

    enum NodeCommand
    {
        CMD_NODE_CREATE_CONFIG = eqNet::CMD_NODE_CUSTOM,
        CMD_NODE_INIT,
        REQ_NODE_INIT,
        CMD_NODE_INIT_REPLY,
        CMD_NODE_EXIT,
        REQ_NODE_EXIT,
        CMD_NODE_EXIT_REPLY,
        CMD_NODE_STOP,
        REQ_NODE_STOP,
        CMD_NODE_CREATE_PIPE,
        CMD_NODE_DESTROY_PIPE,
        CMD_NODE_ALL
    };

    enum ClientCommand
    {
        CMD_CLIENT_UNUSED = CMD_SERVER_ALL,
        CMD_CLIENT_ALL
    };

    enum ConfigCommand
    {
        CMD_CONFIG_INIT,
        REQ_CONFIG_INIT, // REQ must always follow CMD
        CMD_CONFIG_INIT_REPLY,
        CMD_CONFIG_EXIT,
        REQ_CONFIG_EXIT, // REQ must always follow CMD
        CMD_CONFIG_EXIT_REPLY,
        CMD_CONFIG_FRAME_BEGIN,
        REQ_CONFIG_FRAME_BEGIN, // REQ must always follow CMD
        CMD_CONFIG_FRAME_BEGIN_REPLY,
        CMD_CONFIG_FRAME_END,
        REQ_CONFIG_FRAME_END, // REQ must always follow CMD
        CMD_CONFIG_FRAME_END_REPLY,
        CMD_CONFIG_ALL
    };

    enum PipeCommand
    {
        CMD_PIPE_INIT,
        REQ_PIPE_INIT,
        CMD_PIPE_INIT_REPLY,
        CMD_PIPE_EXIT,
        REQ_PIPE_EXIT,
        CMD_PIPE_EXIT_REPLY,
        CMD_PIPE_CREATE_WINDOW,
        CMD_PIPE_DESTROY_WINDOW,
        CMD_PIPE_ALL
    };

    enum WindowCommand
    {
        CMD_WINDOW_INIT,
        REQ_WINDOW_INIT,
        CMD_WINDOW_INIT_REPLY,
        CMD_WINDOW_EXIT,
        REQ_WINDOW_EXIT,
        CMD_WINDOW_EXIT_REPLY,
        CMD_WINDOW_CREATE_CHANNEL,
        CMD_WINDOW_DESTROY_CHANNEL,
        CMD_WINDOW_ALL
    };

    enum ChannelCommand
    {
        CMD_CHANNEL_INIT,
        REQ_CHANNEL_INIT,
        CMD_CHANNEL_INIT_REPLY,
        CMD_CHANNEL_EXIT,
        REQ_CHANNEL_EXIT,
        CMD_CHANNEL_EXIT_REPLY,
        CMD_CHANNEL_CLEAR,
        REQ_CHANNEL_CLEAR,
        CMD_CHANNEL_DRAW,
        REQ_CHANNEL_DRAW,
        CMD_CHANNEL_ALL
    };
};

#endif // EQ_COMMANDS_H

