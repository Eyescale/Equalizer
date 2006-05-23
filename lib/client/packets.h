
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include "commands.h"
#include "pixelViewport.h"
#include "renderContext.h"
#include "viewport.h"

#include <eq/net/packets.h>

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
                rendererInfo[0] = '\0';
            }

        uint32_t requestID;
        uint32_t compoundModes;
        char     rendererInfo[8] EQ_ALIGN8;
    };

    struct ServerCreateConfigPacket : public ServerPacket
    {
        ServerCreateConfigPacket()
            {
                command = CMD_SERVER_CREATE_CONFIG;
                size    = sizeof( ServerCreateConfigPacket );
                name[0] = '\0';
            }

        uint32_t      configID;
        eqNet::NodeID renderNodeID;
        char          name[8] EQ_ALIGN8;
    };

    struct ServerChooseConfigReplyPacket : public ServerPacket
    {
        ServerChooseConfigReplyPacket( const ServerChooseConfigPacket*
                                       requestPacket )
            {
                command   = CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size      = sizeof( ServerChooseConfigReplyPacket );
                requestID = requestPacket->requestID;
                sessionName[0] = '\0';
            }

        uint32_t requestID;
        uint32_t configID;
        char     sessionName[8] EQ_ALIGN8;
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
        uint32_t initID;
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

    struct ConfigBeginFramePacket : public eqNet::SessionPacket
    {
        ConfigBeginFramePacket( const uint32_t configID ) 
                : eqNet::SessionPacket( configID )
            {
                command   = CMD_CONFIG_FRAME_BEGIN;
                size      = sizeof( ConfigBeginFramePacket );
            }
        uint32_t requestID;
        uint32_t frameID;
    };

    struct ConfigBeginFrameReplyPacket : public eqNet::SessionPacket
    {
        ConfigBeginFrameReplyPacket(const ConfigBeginFramePacket* requestPacket)
                : eqNet::SessionPacket( requestPacket->sessionID )
            {
                command   = CMD_CONFIG_FRAME_BEGIN_REPLY;
                size      = sizeof( ConfigBeginFrameReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t frameNumber;
        uint32_t nNodeIDs;
        eqNet::NodeID nodeIDs[1];
    };

    struct ConfigEndFramePacket : public eqNet::SessionPacket
    {
        ConfigEndFramePacket( const uint32_t configID ) 
                : eqNet::SessionPacket( configID )
            {
                command   = CMD_CONFIG_FRAME_END;
                size      = sizeof( ConfigEndFramePacket );
            }
        uint32_t requestID;
    };

    struct ConfigEndFrameReplyPacket : public eqNet::SessionPacket
    {
        ConfigEndFrameReplyPacket(const ConfigEndFramePacket* requestPacket)
                : eqNet::SessionPacket( requestPacket->sessionID )
            {
                command   = CMD_CONFIG_FRAME_END_REPLY;
                size      = sizeof( ConfigEndFrameReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t result;
    };

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
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
        uint32_t initID;
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
        bool     result;
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
        uint32_t initID;
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
        bool     result;
    };

    struct PipeUpdatePacket : public eqNet::ObjectPacket
    {
        PipeUpdatePacket( const uint32_t configID, const uint32_t pipeID )
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_UPDATE;
                size    = sizeof( PipeInitPacket );
            }

        uint32_t frameID;
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
        PipeDestroyWindowPacket( const uint32_t configID, const uint32_t pipeID)
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_DESTROY_WINDOW;
                size    = sizeof( PipeDestroyWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeFrameSyncPacket : public eqNet::ObjectPacket
    {
        PipeFrameSyncPacket( const uint32_t configID, const uint32_t pipeID )
                : eqNet::ObjectPacket( configID, pipeID )
            {
                command = CMD_PIPE_FRAME_SYNC;
                size    = sizeof( PipeFrameSyncPacket );
            }
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

        uint32_t      requestID;
        uint32_t      initID;
        PixelViewport pvp;
        Viewport      vp;
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

        uint32_t      requestID;
        bool          result;

        PixelViewport pvp;
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
        bool     result;
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

    struct WindowSwapWithBarrierPacket : public eqNet::ObjectPacket
    {
        WindowSwapWithBarrierPacket( const uint32_t configID, 
                                     const uint32_t windowID )
                : eqNet::ObjectPacket( configID, windowID )
            {
                command = CMD_WINDOW_SWAP_WITH_BARRIER;
                size    = sizeof( WindowSwapWithBarrierPacket );
            }
        uint32_t barrierID;
    };

    struct WindowSwapPacket : public eqNet::ObjectPacket
    {
        WindowSwapPacket( const uint32_t configID, const uint32_t windowID )
                : eqNet::ObjectPacket( configID, windowID )
            {
                command = CMD_WINDOW_SWAP;
                size    = sizeof( WindowSwapPacket );
            }
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
        uint32_t initID;
        Viewport vp;
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

        float    _near;
        float    _far;
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
        bool     result;
    };

    struct ChannelClearPacket : public eqNet::ObjectPacket
    {
        ChannelClearPacket( const uint32_t configID, const uint32_t channelID )
                : eqNet::ObjectPacket( configID, channelID )
            {
                command       = CMD_CHANNEL_CLEAR;
                size          = sizeof( ChannelClearPacket );
                context.hints = HINT_BUFFER;
            }

        RenderContext context;
        uint32_t      frameID;
    };
        
    struct ChannelDrawPacket : public eqNet::ObjectPacket
    {
        ChannelDrawPacket( const uint32_t configID, const uint32_t channelID )
                : eqNet::ObjectPacket( configID, channelID )
            {
                command       = CMD_CHANNEL_DRAW;
                size          = sizeof( ChannelDrawPacket );
                context.hints = HINT_BUFFER | HINT_FRUSTUM;
            }

        RenderContext context;
        uint32_t      frameID;
    };
        

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " req " << packet->requestID
           << " cmp modes " << packet->compoundModes
           << " renderer " << packet->rendererInfo;
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

    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeCreatePipePacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " id " << packet->pipeID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                     const ConfigBeginFrameReplyPacket* packet )
    {
        os << (eqNet::SessionPacket*)packet << " frame #" 
           << packet->frameNumber << ", " << packet->nNodeIDs << " nodes";
        for( uint32_t i=0 ; i<4 && i<packet->nNodeIDs; ++i )
            os << " " << i << ":" << packet->nodeIDs[i];

        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeInitReplyPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " result " << packet->result;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowInitReplyPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " result " << packet->result
           << " pvp " << packet->pvp;
        return os;
    }
}

#endif // EQ_PACKETS_H

