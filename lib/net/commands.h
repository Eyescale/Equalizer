
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_COMMANDS_H
#define EQNET_COMMANDS_H

namespace eqNet
{
    enum ObjectCommand
    {
        CMD_OBJECT_SYNC,
        REQ_OBJECT_SYNC,
        CMD_OBJECT_CUSTOM // must be last
    };

    enum NodeCommand
    {
        CMD_NODE_STOP = CMD_OBJECT_CUSTOM,
        REQ_NODE_STOP,
        CMD_NODE_MESSAGE,
        CMD_NODE_MAP_SESSION,
        CMD_NODE_MAP_SESSION_REPLY,
        CMD_NODE_UNMAP_SESSION,
        CMD_NODE_UNMAP_SESSION_REPLY,
        CMD_NODE_SESSION,
        CMD_NODE_CONNECT,
        CMD_NODE_GET_CONNECTION_DESCRIPTION,
        CMD_NODE_GET_CONNECTION_DESCRIPTION_REPLY,
        CMD_NODE_CUSTOM = 1<<6 // must be last
    };

    enum SessionCommand
    {
        CMD_SESSION_GEN_IDS = CMD_OBJECT_CUSTOM,
        CMD_SESSION_GEN_IDS_REPLY,
        CMD_SESSION_SET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER,
        CMD_SESSION_GET_ID_MASTER_REPLY,
        CMD_SESSION_GET_OBJECT_MASTER,
        CMD_SESSION_GET_OBJECT_MASTER_REPLY,
        CMD_SESSION_REGISTER_OBJECT,
        CMD_SESSION_UNREGISTER_OBJECT,
        CMD_SESSION_GET_OBJECT,
        CMD_SESSION_INIT_OBJECT,
        CMD_SESSION_INSTANCIATE_OBJECT,
        CMD_SESSION_INIT_OBJECT_REPLY,
        CMD_SESSION_CUSTOM // must be last
    };

    enum BarrierCommand
    {
        CMD_BARRIER_ENTER = CMD_OBJECT_CUSTOM,
        CMD_BARRIER_ENTER_REPLY,
        CMD_BARRIER_ALL
    };

    enum UserCommand
    {
        CMD_USER_CUSTOM // must be last
    };
};

#endif // EQNET_COMMANDS_H

