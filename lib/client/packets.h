
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/client/channel.h>
#include <eq/client/commands.h>
#include <eq/client/frame.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/renderContext.h>
#include <eq/client/statEvent.h>
#include <eq/client/viewport.h>
#include <eq/client/window.h>

#include <eq/net/packets.h>

namespace eq
{
    class Pipe;
    class Window;

    enum DataType
    {
        DATATYPE_EQ_CLIENT = eqNet::DATATYPE_EQNET_CUSTOM, // 128
        DATATYPE_EQ_SERVER,
        DATATYPE_EQ_CUSTOM = 1<<8 // 256
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
        eqNet::NodeID appNodeID;
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
    typedef eqNet::SessionPacket ConfigPacket;
    struct ConfigCreateNodePacket : public ConfigPacket
    {
        ConfigCreateNodePacket()
            {
                command = CMD_CONFIG_CREATE_NODE;
                size    = sizeof( ConfigCreateNodePacket );
            }

        uint32_t nodeID;
    };

    struct ConfigInitPacket : public ConfigPacket
    {
        ConfigInitPacket()
            {
                command   = CMD_CONFIG_INIT;
                size      = sizeof( ConfigInitPacket );
            }

        uint32_t requestID;
        uint32_t initID;
    };

    struct ConfigInitReplyPacket : public ConfigPacket
    {
        ConfigInitReplyPacket( const ConfigInitPacket* requestPacket )
            {
                command   = CMD_CONFIG_INIT_REPLY;
                size      = sizeof( ConfigInitReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t headMatrixID;
        bool     result;
    };

    struct ConfigExitPacket : public ConfigPacket
    {
        ConfigExitPacket()
            {
                command   = CMD_CONFIG_EXIT;
                size      = sizeof( ConfigExitPacket );
            }
        uint32_t requestID;
    };

    struct ConfigExitReplyPacket : public ConfigPacket
    {
        ConfigExitReplyPacket( const ConfigExitPacket* requestPacket )
            {
                command   = CMD_CONFIG_EXIT_REPLY;
                size      = sizeof( ConfigExitReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        bool     result;
    };

    struct ConfigBeginFramePacket : public ConfigPacket
    {
        ConfigBeginFramePacket()
            {
                command   = CMD_CONFIG_BEGIN_FRAME;
                size      = sizeof( ConfigBeginFramePacket );
            }
        uint32_t requestID;
        uint32_t frameID;
    };

    struct ConfigBeginFrameReplyPacket : public ConfigPacket
    {
        ConfigBeginFrameReplyPacket(const ConfigBeginFramePacket* requestPacket)
            {
                command   = CMD_CONFIG_BEGIN_FRAME_REPLY;
                size      = sizeof( ConfigBeginFrameReplyPacket );
                sessionID = requestPacket->sessionID;
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t frameNumber;
        uint32_t nNodeIDs;
        eqNet::NodeID nodeIDs[1] EQ_ALIGN8;
    };

    struct ConfigEndFramePacket : public ConfigPacket
    {
        ConfigEndFramePacket()
            {
                command   = CMD_CONFIG_END_FRAME;
                size      = sizeof( ConfigEndFramePacket );
            }
        uint32_t requestID;
    };

    struct ConfigEndFrameReplyPacket : public ConfigPacket
    {
        ConfigEndFrameReplyPacket(const ConfigEndFramePacket* requestPacket)
            {
                command   = CMD_CONFIG_END_FRAME_REPLY;
                size      = sizeof( ConfigEndFrameReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t result;
    };

    struct ConfigFinishFramesPacket : public ConfigPacket
    {
        ConfigFinishFramesPacket()
            {
                command   = CMD_CONFIG_FINISH_FRAMES;
                size      = sizeof( ConfigFinishFramesPacket );
            }
        uint32_t requestID;
    };

    struct ConfigFinishFramesReplyPacket : public ConfigPacket
    {
        ConfigFinishFramesReplyPacket( const ConfigFinishFramesPacket* 
                                       requestPacket )
            {
                command   = CMD_CONFIG_FINISH_FRAMES_REPLY;
                size      = sizeof( ConfigFinishFramesReplyPacket );
                requestID = requestPacket->requestID;
            }
        uint32_t requestID;
        uint32_t result;
    };

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct NodeInitPacket : public eqNet::ObjectPacket
    {
        NodeInitPacket()
            {
                command        = CMD_NODE_INIT;
                size           = sizeof( NodeInitPacket );
            }

        uint32_t requestID;
        uint32_t initID;
    };

    struct NodeInitReplyPacket : public eqNet::ObjectPacket
    {
        NodeInitReplyPacket( const NodeInitPacket* requestPacket )
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
        NodeExitPacket()
            {
                command = CMD_NODE_EXIT;
                size    = sizeof( NodeExitPacket );
            }

        uint32_t requestID;
    };

    struct NodeExitReplyPacket : public eqNet::ObjectPacket
    {
        NodeExitReplyPacket( const NodeExitPacket* requestPacket )
            {
                command   = CMD_NODE_EXIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( NodeExitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
    };

    struct NodeCreatePipePacket : public eqNet::ObjectPacket
    {
        NodeCreatePipePacket()
            {
                command = CMD_NODE_CREATE_PIPE;
                size    = sizeof( NodeCreatePipePacket );
            }

        uint32_t pipeID;
    };

    struct NodeDestroyPipePacket : public eqNet::ObjectPacket
    {
        NodeDestroyPipePacket()
            {
                command = CMD_NODE_DESTROY_PIPE;
                size    = sizeof( NodeDestroyPipePacket );
            }

        uint32_t pipeID;
    };
    
    //------------------------------------------------------------
    // Pipe
    //------------------------------------------------------------
    struct PipeCreateWindowPacket : public eqNet::ObjectPacket
    {
        PipeCreateWindowPacket()
            {
                command = CMD_PIPE_CREATE_WINDOW;
                size    = sizeof( PipeCreateWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeDestroyWindowPacket : public eqNet::ObjectPacket
    {
        PipeDestroyWindowPacket()
            {
                command = CMD_PIPE_DESTROY_WINDOW;
                size    = sizeof( PipeDestroyWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeInitPacket : public eqNet::ObjectPacket
    {
        PipeInitPacket()
            {
                command = CMD_PIPE_INIT;
                size    = sizeof( PipeInitPacket );
            }

        uint32_t      requestID;
        uint32_t      initID;
        uint32_t      display;
        uint32_t      screen;
        PixelViewport pvp;
    };

    struct PipeInitReplyPacket : public eqNet::ObjectPacket
    {
        PipeInitReplyPacket( const PipeInitPacket* requestPacket )
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
        PipeExitPacket()
            {
                command = CMD_PIPE_EXIT;
                size    = sizeof( PipeExitPacket );
            }

        uint32_t requestID;
    };

    struct PipeExitReplyPacket : public eqNet::ObjectPacket
    {
        PipeExitReplyPacket( const PipeExitPacket* requestPacket )
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
        PipeUpdatePacket()
            {
                command = CMD_PIPE_UPDATE;
                size    = sizeof( PipeInitPacket );
            }

        uint32_t frameID;
    };

    struct PipeFrameSyncPacket : public eqNet::ObjectPacket
    {
        PipeFrameSyncPacket()
            {
                command = CMD_PIPE_FRAME_SYNC;
                size    = sizeof( PipeFrameSyncPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct PipeFrameSyncReplyPacket : public eqNet::ObjectPacket
    {
        PipeFrameSyncReplyPacket()
            {
                command = CMD_PIPE_FRAME_SYNC_REPLY;
                size    = sizeof( PipeFrameSyncReplyPacket );
            }

        uint32_t  frameNumber;
        uint32_t  nStatEvents;
        StatEvent statEvents[1] EQ_ALIGN8;
    };

    //------------------------------------------------------------
    // Window
    //------------------------------------------------------------
    struct WindowInitPacket : public eqNet::ObjectPacket
    {
        WindowInitPacket()
            {
                command = CMD_WINDOW_INIT;
                size    = sizeof( WindowInitPacket );
                name[0]   = '\0';
            }

        uint32_t       requestID;
        uint32_t       initID;
        int32_t        iattr[eq::Window::IATTR_ALL];
        PixelViewport  pvp;
        Viewport       vp;
        char           name[8] EQ_ALIGN8;
    };

    struct WindowInitReplyPacket : public eqNet::ObjectPacket
    {
        WindowInitReplyPacket( const WindowInitPacket* requestPacket )
            {
                command   = CMD_WINDOW_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( WindowInitReplyPacket );
            }

        uint32_t                requestID;
        bool                    result;
        PixelViewport           pvp;
        Window::DrawableConfig  drawableConfig;
    };

    struct WindowExitPacket : public eqNet::ObjectPacket
    {
        WindowExitPacket()
            {
                command = CMD_WINDOW_EXIT;
                size    = sizeof( WindowExitPacket );
            }

        uint32_t requestID;
    };

    struct WindowExitReplyPacket : public eqNet::ObjectPacket
    {
        WindowExitReplyPacket( const WindowExitPacket* requestPacket )
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
        WindowCreateChannelPacket()
            {
                command = CMD_WINDOW_CREATE_CHANNEL;
                size    = sizeof( WindowCreateChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowDestroyChannelPacket : public eqNet::ObjectPacket
    {
        WindowDestroyChannelPacket()
            {
                command = CMD_WINDOW_DESTROY_CHANNEL;
                size    = sizeof( WindowDestroyChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowSetPVPPacket : public eqNet::ObjectPacket
    {
        WindowSetPVPPacket()
            {
                command = CMD_WINDOW_SET_PVP;
                size    = sizeof( WindowSetPVPPacket );
            }

        PixelViewport pvp;
    };

    struct WindowFinishPacket : public eqNet::ObjectPacket
    {
        WindowFinishPacket()
            {
                command = CMD_WINDOW_FINISH;
                size    = sizeof( WindowFinishPacket );
            }
    };

    struct WindowBarrierPacket : public eqNet::ObjectPacket
    {
        WindowBarrierPacket()
            {
                command = CMD_WINDOW_BARRIER;
                size    = sizeof( WindowBarrierPacket );
            }
        uint32_t barrierID;
        uint32_t barrierVersion;
    };

    struct WindowSwapPacket : public eqNet::ObjectPacket
    {
        WindowSwapPacket()
            {
                command = CMD_WINDOW_SWAP;
                size    = sizeof( WindowSwapPacket );
            }
    };

    struct WindowStartFramePacket : public eqNet::ObjectPacket
    {
        WindowStartFramePacket()
            {
                command = CMD_WINDOW_STARTFRAME;
                size    = sizeof( WindowStartFramePacket );
            }
        uint32_t frameID;
        bool     makeCurrent;
    };
    struct WindowEndFramePacket : public eqNet::ObjectPacket
    {
        WindowEndFramePacket()
            {
                command = CMD_WINDOW_ENDFRAME;
                size    = sizeof( WindowEndFramePacket );
            }
        uint32_t frameID;
    };

    //------------------------------------------------------------
    // Channel
    //------------------------------------------------------------
    struct ChannelInitPacket : public eqNet::ObjectPacket
    {
        ChannelInitPacket()
            {
                command = CMD_CHANNEL_INIT;
                size    = sizeof( ChannelInitPacket );
                name[0] = '\0';
            }

        uint32_t      requestID;
        uint32_t      initID;
        int32_t       iattr[eq::Channel::IATTR_ALL];
        PixelViewport pvp;
        Viewport      vp;
        char          name[8] EQ_ALIGN8;
    };

    struct ChannelInitReplyPacket : public eqNet::ObjectPacket
    {
        ChannelInitReplyPacket( const ChannelInitPacket* requestPacket )
            {
                command   = CMD_CHANNEL_INIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( ChannelInitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
        float    near;
        float    far;
    };

    struct ChannelExitPacket : public eqNet::ObjectPacket
    {
        ChannelExitPacket()
            {
                command = CMD_CHANNEL_EXIT;
                size    = sizeof( ChannelExitPacket );
            }

        uint32_t requestID;
    };

    struct ChannelExitReplyPacket : public eqNet::ObjectPacket
    {
        ChannelExitReplyPacket( const ChannelExitPacket* requestPacket )
            {
                command   = CMD_CHANNEL_EXIT_REPLY;
                requestID = requestPacket->requestID;
                size      = sizeof( ChannelExitReplyPacket );
            }

        uint32_t requestID;
        bool     result;
    };

    struct ChannelSetNearFarPacket : public eqNet::ObjectPacket
    {
        ChannelSetNearFarPacket()
            {
                command   = CMD_CHANNEL_SET_NEARFAR;
                size      = sizeof( ChannelSetNearFarPacket );
            }

        float    near;
        float    far;
    };

    struct ChannelTaskPacket : public eqNet::ObjectPacket
    {
        RenderContext context;
    };

    struct ChannelClearPacket : public ChannelTaskPacket
    {
        ChannelClearPacket()
            {
                command       = CMD_CHANNEL_CLEAR;
                size          = sizeof( ChannelClearPacket );
            }
    };
        
    struct ChannelDrawPacket : public ChannelTaskPacket
    {
        ChannelDrawPacket()
            {
                command       = CMD_CHANNEL_DRAW;
                size          = sizeof( ChannelDrawPacket );
            }
    };
        
    struct ChannelAssemblePacket : public ChannelTaskPacket
    {
        ChannelAssemblePacket()
            {
                command       = CMD_CHANNEL_ASSEMBLE;
                size          = sizeof( ChannelAssemblePacket );
            }

        uint32_t             nFrames;
        eqNet::ObjectVersion frames[1] EQ_ALIGN8;
    };
        
    struct ChannelReadbackPacket : public ChannelTaskPacket
    {
        ChannelReadbackPacket()
            {
                command       = CMD_CHANNEL_READBACK;
                size          = sizeof( ChannelReadbackPacket );
            }

        uint32_t             nFrames;
        eqNet::ObjectVersion frames[1] EQ_ALIGN8;
    };
        
    struct ChannelTransmitPacket : public eqNet::ObjectPacket
    {
        ChannelTransmitPacket()
            {
                command       = CMD_CHANNEL_TRANSMIT;
                size          = sizeof( ChannelTransmitPacket );
            }

        
        eqNet::ObjectVersion frame;
        uint32_t             nNodes;
        eqNet::NodeID        nodes[1] EQ_ALIGN8;
    };

    //------------------------------------------------------------
    // Frame Data
    //------------------------------------------------------------
    struct FrameDataTransmitPacket : public eqNet::ObjectPacket
    {
        FrameDataTransmitPacket()
            {
                command = CMD_FRAMEDATA_TRANSMIT;
                size    = sizeof( FrameDataTransmitPacket );
            }

        uint32_t      version;
        uint32_t      buffers;
        PixelViewport pvp;
        uint8_t       data[8] EQ_ALIGN8; // size is pvp.w * pvp.h * depth
    };

    struct FrameDataReadyPacket : public eqNet::ObjectPacket
    {
        FrameDataReadyPacket()
            {
                command = CMD_FRAMEDATA_READY;
                size    = sizeof( FrameDataReadyPacket );
            }
        uint32_t version;
    };

    //------------------------------------------------------------
    // Event Thread
    //------------------------------------------------------------
    struct GLXEventThreadAddPipePacket : public eqNet::Packet
    {
        GLXEventThreadAddPipePacket()
            {
                command = CMD_GLXEVENTTHREAD_ADD_PIPE;
                size    = sizeof( GLXEventThreadAddPipePacket );
            }
        Pipe* pipe;
    };
    struct GLXEventThreadRemovePipePacket : public eqNet::Packet
    {
        GLXEventThreadRemovePipePacket()
            {
                command = CMD_GLXEVENTTHREAD_REMOVE_PIPE;
                size    = sizeof( GLXEventThreadRemovePipePacket );
            }
        uint32_t requestID;
        Pipe*    pipe;
    };
    struct GLXEventThreadAddWindowPacket : public eqNet::Packet
    {
        GLXEventThreadAddWindowPacket()
            {
                command = CMD_GLXEVENTTHREAD_ADD_WINDOW;
                size    = sizeof( GLXEventThreadAddWindowPacket );
            }
        Window* window;
    };
    struct GLXEventThreadRemoveWindowPacket : public eqNet::Packet
    {
        GLXEventThreadRemoveWindowPacket()
            {
                command = CMD_GLXEVENTTHREAD_REMOVE_WINDOW;
                size    = sizeof( GLXEventThreadRemoveWindowPacket );
            }
        uint32_t requestID;
        Window*  window;
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
                                       const ConfigInitReplyPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " requestID " << packet->requestID
           << " headMatrixID " << packet->headMatrixID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                     const ConfigBeginFrameReplyPacket* packet )
    {
        os << (ConfigPacket*)packet << " frame #" 
           << packet->frameNumber << ", " << packet->nNodeIDs << " nodes";
        for( uint32_t i=0 ; i<4 && i<packet->nNodeIDs; ++i )
            os << " " << i << ":" << packet->nodeIDs[i];

        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeInitPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " init id " << packet->initID
           << " display " << packet->display << " screen " << packet->screen;
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

    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowBarrierPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " barrier " << packet->barrierID
           << " version " << packet->barrierVersion;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowStartFramePacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " frame " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowEndFramePacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " frame " << packet->frameID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelInitReplyPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " result " << packet->result;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelTaskPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " " << packet->context;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelReadbackPacket* packet )
    {
        os << (ChannelTaskPacket*)packet << " nFrames " << packet->nFrames;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelTransmitPacket* packet )
    {
        os << (eqNet::ObjectPacket*)packet << " frame " << packet->frame
           << " nNodes " << packet->nNodes;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelAssemblePacket* packet )
    {
        os << (ChannelTaskPacket*)packet << " nFrames " << packet->nFrames;
        return os;
    }
}

#endif // EQ_PACKETS_H

