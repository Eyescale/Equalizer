
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/net/packets.h>

#include "commands.h"
#include "config.h"

namespace eq
{
    enum DataType
    {
        DATATYPE_EQ_CLIENT = eqNet::DATATYPE_CUSTOM,
        DATATYPE_EQ_SERVER
    };

    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerPacket : public eqNet::Packet
    {
        ServerPacket(){ datatype = DATATYPE_EQ_SERVER; }
    };

    struct ServerChooseConfigPacket : public ServerPacket
    {
        ServerChooseConfigPacket()
            {
                command = CMD_SERVER_CHOOSE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
            }

        uint32_t requestID;
        uint32_t appNameLength;
        uint32_t renderClientLength;
        uint32_t compoundModes;
    };

    struct ServerChooseConfigReplyPacket : public ServerPacket
    {
        ServerChooseConfigReplyPacket( const ServerChooseConfigPacket*
                                       requestPacket )
            {
                command   = CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size      = sizeof( ServerChooseConfigReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
        uint32_t configID;
        uint32_t sessionNameLength;
    };

    struct ServerReleaseConfigPacket : public ServerPacket
    {
        ServerReleaseConfigPacket()
            {
                command = CMD_SERVER_RELEASE_CONFIG;
                size    = sizeof( ServerReleaseConfigPacket );
            }

        uint32_t configID;
    };

    //------------------------------------------------------------
    // Config
    //------------------------------------------------------------
    struct ConfigInitPacket : public eqNet::SessionPacket
    {
        ConfigInitPacket( const uint32_t configID ) 
                : eqNet::SessionPacket( configID )
            {
                command   = CMD_CONFIG_INIT;
                size      = sizeof( ConfigInitPacket );
            }
        uint32_t requestID;
    };

    struct ConfigInitReplyPacket : public eqNet::SessionPacket
    {
        ConfigInitReplyPacket( const ConfigInitPacket* requestPacket )
                : eqNet::SessionPacket( requestPacket->sessionID )
            {
                command   = CMD_CONFIG_INIT_REPLY;
                size      = sizeof( ConfigInitReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        bool     result;
    };

    struct ConfigExitPacket : public eqNet::SessionPacket
    {
        ConfigExitPacket( const uint32_t configID )
                : eqNet::SessionPacket( configID )
            {
                command   = CMD_CONFIG_EXIT;
                size      = sizeof( ConfigExitPacket );
            }
        uint32_t requestID;
    };

    struct ConfigExitReplyPacket : public eqNet::SessionPacket
    {
        ConfigExitReplyPacket( const ConfigExitPacket* requestPacket )
                : eqNet::SessionPacket( requestPacket->sessionID )
            {
                command   = CMD_CONFIG_EXIT_REPLY;
                size      = sizeof( ConfigExitReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        bool     result;
    };

    struct ConfigFrameBeginPacket : public eqNet::SessionPacket
    {
        ConfigFrameBeginPacket( const uint32_t configID ) 
                : eqNet::SessionPacket( configID )
            {
                command   = CMD_CONFIG_FRAME_BEGIN;
                size      = sizeof( ConfigFrameBeginPacket );
            }
        uint32_t requestID;
    };

    struct ConfigFrameBeginReplyPacket : public eqNet::SessionPacket
    {
        ConfigFrameBeginReplyPacket(const ConfigFrameBeginPacket* requestPacket)
                : eqNet::SessionPacket( requestPacket->sessionID )
            {
                command   = CMD_CONFIG_FRAME_BEGIN_REPLY;
                size      = sizeof( ConfigFrameBeginReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t result;
    };

    struct ConfigFrameEndPacket : public eqNet::SessionPacket
    {
        ConfigFrameEndPacket( const uint32_t configID ) 
                : eqNet::SessionPacket( configID )
            {
                command   = CMD_CONFIG_FRAME_END;
                size      = sizeof( ConfigFrameEndPacket );
            }
        uint32_t requestID;
    };

    struct ConfigFrameEndReplyPacket : public eqNet::SessionPacket
    {
        ConfigFrameEndReplyPacket(const ConfigFrameEndPacket* requestPacket)
                : eqNet::SessionPacket( requestPacket->sessionID )
            {
                command   = CMD_CONFIG_FRAME_END_REPLY;
                size      = sizeof( ConfigFrameEndReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t result;
    };

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct NodeCreateConfigPacket : public eqNet::NodePacket
    {
        NodeCreateConfigPacket()
            {
                command = CMD_NODE_CREATE_CONFIG;
                size    = sizeof( NodeCreateConfigPacket );
            }

        uint32_t configID;
        uint32_t nameLength;
    };

    // This packet is similar to the Create packets, except that the node
    // already exists and is registered and initialized with this one packet.
    struct NodeInitPacket : public eqNet::NodePacket
    {
        NodeInitPacket( const uint32_t configID, const uint32_t nodeID )
            {
                command        = CMD_NODE_INIT;
                size           = sizeof( NodeInitPacket );
                this->configID = configID;
                this->nodeID   = nodeID;
            }

        uint32_t requestID;
        uint32_t configID;
        uint32_t nodeID;
    };

    struct NodeInitReplyPacket : public eqNet::ObjectPacket
    {
        NodeInitReplyPacket( NodeInitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->configID, 
                                       requestPacket->nodeID )
            {
                command   = CMD_NODE_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( NodeInitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
    };

    struct NodeExitPacket : public eqNet::ObjectPacket
    {
        NodeExitPacket( const uint32_t configID, const uint32_t nodeID )
                : eqNet::ObjectPacket( configID, nodeID )
            {
                command = CMD_NODE_EXIT;
                size    = sizeof( NodeExitPacket );
            }

        uint32_t requestID;
    };

    struct NodeExitReplyPacket : public eqNet::ObjectPacket
    {
        NodeExitReplyPacket( NodeExitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID, 
                                       requestPacket->objectID )
            {
                command   = CMD_NODE_EXIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( NodeExitReplyPacket );
            }

        uint32_t requestID;
    };

    struct NodeStopPacket : public eqNet::ObjectPacket
    {
        NodeStopPacket( const uint32_t configID, const uint32_t nodeID )
                : eqNet::ObjectPacket( configID, nodeID )
            {
                command = CMD_NODE_STOP;
                size    = sizeof( NodeStopPacket );
            }
    };

    struct NodeCreatePipePacket : public eqNet::ObjectPacket
    {
        NodeCreatePipePacket( const uint32_t configID, const uint32_t nodeID )
                : eqNet::ObjectPacket( configID, nodeID )
            {
                command = CMD_NODE_CREATE_PIPE;
                size    = sizeof( NodeCreatePipePacket );
            }

        uint32_t pipeID;
    };

    struct NodeDestroyPipePacket : public eqNet::ObjectPacket
    {
        NodeDestroyPipePacket( const uint32_t configID, const uint32_t nodeID )
                : eqNet::ObjectPacket( configID, nodeID )
            {
                command = CMD_NODE_DESTROY_PIPE;
                size    = sizeof( NodeDestroyPipePacket );
            }

        uint32_t pipeID;
    };
    
    //------------------------------------------------------------
    // Pipe
    //------------------------------------------------------------
    struct PipeInitPacket : public eqNet::ObjectPacket
    {
        PipeInitPacket( const uint32_t configID, const uint32_t pipeID )
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_INIT;
                size    = sizeof( PipeInitPacket );
            }

        uint32_t requestID;
        uint32_t display;
        uint32_t screen;
    };

    struct PipeInitReplyPacket : public eqNet::ObjectPacket
    {
        PipeInitReplyPacket( PipeInitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID, 
                                       requestPacket->objectID )
            {
                command   = CMD_PIPE_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( PipeInitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
    };

    struct PipeExitPacket : public eqNet::ObjectPacket
    {
        PipeExitPacket( const uint32_t configID, const uint32_t pipeID )
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_EXIT;
                size    = sizeof( PipeExitPacket );
            }

        uint32_t requestID;
    };

    struct PipeExitReplyPacket : public eqNet::ObjectPacket
    {
        PipeExitReplyPacket( PipeExitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID, 
                                       requestPacket->objectID )
            {
                command   = CMD_PIPE_EXIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( PipeExitReplyPacket );
            }

        uint32_t requestID;
    };

    struct PipeCreateWindowPacket : public eqNet::ObjectPacket
    {
        PipeCreateWindowPacket( const uint32_t configID, const uint32_t pipeID )
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_CREATE_WINDOW;
                size    = sizeof( PipeCreateWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeDestroyWindowPacket : public eqNet::ObjectPacket
    {
        PipeDestroyWindowPacket( const uint32_t configID, const uint32_t pipeID )
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_DESTROY_WINDOW;
                size    = sizeof( PipeDestroyWindowPacket );
            }

        uint32_t windowID;
    };

    //------------------------------------------------------------
    // Window
    //------------------------------------------------------------
    struct WindowInitPacket : public eqNet::ObjectPacket
    {
        WindowInitPacket( const uint32_t configID, const uint32_t windowID )
                : eqNet::ObjectPacket( configID, windowID )
            {
                command = CMD_WINDOW_INIT;
                size    = sizeof( WindowInitPacket );
            }

        uint32_t requestID;
    };

    struct WindowInitReplyPacket : public eqNet::ObjectPacket
    {
        WindowInitReplyPacket( WindowInitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID, 
                                       requestPacket->objectID )
            {
                command   = CMD_WINDOW_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( WindowInitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
    };

    struct WindowExitPacket : public eqNet::ObjectPacket
    {
        WindowExitPacket( const uint32_t configID, const uint32_t windowID )
                : eqNet::ObjectPacket( configID, windowID )
            {
                command = CMD_WINDOW_EXIT;
                size    = sizeof( WindowExitPacket );
            }

        uint32_t requestID;
    };

    struct WindowExitReplyPacket : public eqNet::ObjectPacket
    {
        WindowExitReplyPacket( WindowExitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID, 
                                       requestPacket->objectID )
            {
                command   = CMD_WINDOW_EXIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( WindowExitReplyPacket );
            }

        uint32_t requestID;
    };

    struct WindowCreateChannelPacket : public eqNet::ObjectPacket
    {
        WindowCreateChannelPacket( const uint32_t configID,
                                   const uint32_t windowID )
                : eqNet::ObjectPacket( configID, windowID )
            {
                command = CMD_WINDOW_CREATE_CHANNEL;
                size    = sizeof( WindowCreateChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowDestroyChannelPacket : public eqNet::ObjectPacket
    {
        WindowDestroyChannelPacket( const uint32_t configID,
                                    const uint32_t windowID )
                : eqNet::ObjectPacket( configID, windowID )
            {
                command = CMD_WINDOW_DESTROY_CHANNEL;
                size    = sizeof( WindowDestroyChannelPacket );
            }

        uint32_t channelID;
    };

    //------------------------------------------------------------
    // Channel
    //------------------------------------------------------------
    struct ChannelInitPacket : public eqNet::ObjectPacket
    {
        ChannelInitPacket( const uint32_t configID, const uint32_t channelID )
                : eqNet::ObjectPacket( configID, channelID )
            {
                command = CMD_CHANNEL_INIT;
                size    = sizeof( ChannelInitPacket );
            }

        uint32_t requestID;
    };

    struct ChannelInitReplyPacket : public eqNet::ObjectPacket
    {
        ChannelInitReplyPacket( ChannelInitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID,
                                       requestPacket->objectID )
            {
                command   = CMD_CHANNEL_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( ChannelInitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
    };

    struct ChannelExitPacket : public eqNet::ObjectPacket
    {
        ChannelExitPacket( const uint32_t configID, const uint32_t channelID )
                : eqNet::ObjectPacket( configID, channelID )
            {
                command = CMD_CHANNEL_EXIT;
                size    = sizeof( ChannelExitPacket );
            }

        uint32_t requestID;
    };

    struct ChannelExitReplyPacket : public eqNet::ObjectPacket
    {
        ChannelExitReplyPacket( ChannelExitPacket* requestPacket )
                : eqNet::ObjectPacket( requestPacket->sessionID,
                                       requestPacket->objectID )
            {
                command   = CMD_CHANNEL_EXIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( ChannelExitReplyPacket );
            }

        uint32_t requestID;
    };


    //------------------------------------------------------------


    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes << " appName " 
           << packet->appNameLength << " renderClient "
           << packet->renderClientLength;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                   const ServerChooseConfigReplyPacket* packet )
    {
        os << (ServerPacket*)packet << " req " << packet->requestID << " id " 
           << packet->configID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerReleaseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " config " << packet->configID;
        return os;
    }
}

#endif // EQ_PACKETS_H

