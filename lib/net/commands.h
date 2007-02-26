
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDS_H
#define EQNET_COMMANDS_H

#include <eq/base/base.h>

namespace eqNet
{
    enum NodeCommand
    {
        CMD_NODE_STOP,
        REQ_NODE_STOP,
        CMD_NODE_MESSAGE,
        CMD_NODE_MAP_SESSION,
        CMD_NODE_MAP_SESSION_REPLY,
        CMD_NODE_UNMAP_SESSION,
        CMD_NODE_UNMAP_SESSION_REPLY,
        CMD_NODE_SESSION,
        CMD_NODE_LAUNCHED,
        CMD_NODE_CONNECT,
        CMD_NODE_CONNECT_REPLY,
        CMD_NODE_GET_CONNECTION_DESCRIPTION,
        CMD_NODE_GET_CONNECTION_DESCRIPTION_REPLY,
        CMD_NODE_CUSTOM // must be last
    };

    enum SessionCommand
    {
        CMD_SESSION_GEN_IDS,
        CMD_SESSION_GEN_IDS_REPLY,
        CMD_SESSION_SET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER_REPLY,
        CMD_SESSION_ATTACH_OBJECT,
        CMD_SESSION_DETACH_OBJECT,
        CMD_SESSION_MAP_OBJECT,
        CMD_SESSION_UNMAP_OBJECT,
        CMD_SESSION_SUBSCRIBE_OBJECT,
        CMD_SESSION_SUBSCRIBE_OBJECT_SUCCESS,
        CMD_SESSION_SUBSCRIBE_OBJECT_REPLY,
        CMD_SESSION_UNSUBSCRIBE_OBJECT,
        CMD_SESSION_UNSUBSCRIBE_OBJECT_REPLY,
        CMD_SESSION_GET_OBJECT,
        CMD_SESSION_CUSTOM // must be last
    };

    enum ObjectCommand
    {
        CMD_OBJECT_INSTANCE_DATA,
        REQ_OBJECT_INSTANCE_DATA,
        CMD_OBJECT_DELTA_DATA,
        REQ_OBJECT_DELTA_DATA,
        CMD_OBJECT_COMMIT,
        CMD_OBJECT_CUSTOM // must be last
    };

    enum BarrierCommand
    {
        CMD_BARRIER_ENTER = CMD_OBJECT_CUSTOM,
        CMD_BARRIER_ENTER_REPLY,
        CMD_BARRIER_ALL
    };
};

#endif // EQNET_COMMANDS_H

