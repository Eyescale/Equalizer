
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

#ifndef EQ_PACKETS_H
#define EQ_PACKETS_H

#include <eq/client/channel.h>       // Channel::IATTR_ALL enum
#include <eq/client/commands.h>
#include <eq/client/node.h>          // Node::IATTR_ALL enum
#include <eq/client/renderContext.h> // member
#include <eq/client/types.h>         // member
#include <eq/client/viewport.h>      // member
#include <eq/client/window.h>        // Window::IATTR_ALL enum

#include <eq/net/packets.h>

namespace eq
{
    class Pipe;
    class Window;

/** @cond IGNORE */
    enum DataType
    {
        DATATYPE_EQ_CLIENT = net::DATATYPE_EQNET_CUSTOM, // 128
        DATATYPE_EQ_SERVER,
        DATATYPE_EQ_CUSTOM = 1<<8 // 256
    };

    //------------------------------------------------------------
    // Client
    //------------------------------------------------------------
    struct ClientPacket : public net::Packet
    {
        ClientPacket(){ datatype = DATATYPE_EQ_CLIENT; }
    };

    struct ClientExitPacket : public ClientPacket
    {
        ClientExitPacket()
            {
                command = CMD_CLIENT_EXIT;
                size    = sizeof( ClientExitPacket );
            }
    };

    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerPacket : public net::Packet
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
        EQ_ALIGN8( char rendererInfo[8] );
    };

    struct ServerUseConfigPacket : public ServerPacket
    {
        ServerUseConfigPacket()
            {
                command = CMD_SERVER_USE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
                configInfo[0] = '\0';
            }

        uint32_t requestID;
        EQ_ALIGN8( char configInfo[8] );
    };

    struct ServerCreateConfigPacket : public ServerPacket
    {
        ServerCreateConfigPacket()
            {
                command   = CMD_SERVER_CREATE_CONFIG;
                size      = sizeof( ServerCreateConfigPacket );
                requestID = EQ_ID_INVALID;
                objectID  = EQ_ID_INVALID;                
                name[0]   = '\0';
            }

        uint32_t    configID;
        uint32_t    requestID;
        uint32_t    objectID;
        net::NodeID appNodeID;
        EQ_ALIGN8( char name[8] );
    };

    struct ServerDestroyConfigPacket : public ServerPacket
    {
        ServerDestroyConfigPacket()
            {
                command = CMD_SERVER_DESTROY_CONFIG;
                size    = sizeof( ServerDestroyConfigPacket );
            }

        uint32_t      configID;
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
        ServerChooseConfigReplyPacket( const ServerUseConfigPacket*
                                       requestPacket )
            {
                command   = CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size      = sizeof( ServerChooseConfigReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
        uint32_t configID;
    };

    struct ServerReleaseConfigPacket : public ServerPacket
    {
        ServerReleaseConfigPacket()
            {
                command = CMD_SERVER_RELEASE_CONFIG;
                size    = sizeof( ServerReleaseConfigPacket );
            }

        uint32_t configID;
        uint32_t requestID;
    };

    struct ServerReleaseConfigReplyPacket : public ServerPacket
    {
        ServerReleaseConfigReplyPacket( const ServerReleaseConfigPacket*
                                        requestPacket )
            {
                command   = CMD_SERVER_RELEASE_CONFIG_REPLY;
                size      = sizeof( ServerReleaseConfigReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
    };

    struct ServerShutdownPacket : public ServerPacket
    {
        ServerShutdownPacket()
            {
                command = CMD_SERVER_SHUTDOWN;
                size    = sizeof( ServerShutdownPacket );
            }

        uint32_t requestID;
    };

    struct ServerShutdownReplyPacket : public ServerPacket
    {
        ServerShutdownReplyPacket( const ServerShutdownPacket* requestPacket )
            {
                command   = CMD_SERVER_SHUTDOWN_REPLY;
                size      = sizeof( ServerShutdownReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
        bool     result;
    };

    //------------------------------------------------------------
    // Config
    //------------------------------------------------------------
    typedef net::SessionPacket ConfigPacket;

    struct ConfigCreateReplyPacket : public ConfigPacket
    {
        ConfigCreateReplyPacket( const ServerCreateConfigPacket* request )
        {
            command   = CMD_CONFIG_CREATE_REPLY;
            size      = sizeof( ConfigCreateReplyPacket );
            sessionID = request->configID;
            requestID = request->requestID;
        }

        uint32_t requestID;
    };

    struct ConfigCreateNodePacket : public ConfigPacket
    {
        ConfigCreateNodePacket()
            {
                command = CMD_CONFIG_CREATE_NODE;
                size    = sizeof( ConfigCreateNodePacket );
            }

        uint32_t nodeID;
    };

    struct ConfigDestroyNodePacket : public ConfigPacket
    {
        ConfigDestroyNodePacket()
            {
                command = CMD_CONFIG_DESTROY_NODE;
                size    = sizeof( ConfigDestroyNodePacket );
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
                sessionID = requestPacket->sessionID;
                error[0]  = '\0';
            }

        uint32_t requestID;
        uint32_t configID;
        bool     result;
        EQ_ALIGN8( char error[8] );
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

    struct ConfigStartFramePacket : public ConfigPacket
    {
        ConfigStartFramePacket()
            {
                command   = CMD_CONFIG_START_FRAME;
                size      = sizeof( ConfigStartFramePacket );
            }
        uint32_t frameID;
        uint32_t nChanges;
        EQ_ALIGN8( net::ObjectVersion changes[1] );
    };

    struct ConfigReleaseFrameLocalPacket : public ConfigPacket
    {
        ConfigReleaseFrameLocalPacket()
            {
                command       = CMD_CONFIG_RELEASE_FRAME_LOCAL;
                size          = sizeof( ConfigReleaseFrameLocalPacket );
            }
        uint32_t frameNumber;
    };

    struct ConfigFrameFinishPacket : public ConfigPacket
    {
        ConfigFrameFinishPacket()
            {
                command     = CMD_CONFIG_FRAME_FINISH;
                size        = sizeof( ConfigFrameFinishPacket );
            }
        uint32_t frameNumber;
    };

    struct ConfigFinishAllFramesPacket : public ConfigPacket
    {
        ConfigFinishAllFramesPacket()
            {
                command   = CMD_CONFIG_FINISH_ALL_FRAMES;
                size      = sizeof( ConfigFinishAllFramesPacket );
            }
    };

    struct ConfigFreezeLoadBalancingPacket : public ConfigPacket
    {
        ConfigFreezeLoadBalancingPacket()
            {
                command = CMD_CONFIG_FREEZE_LOAD_BALANCING;
                size    = sizeof( ConfigFreezeLoadBalancingPacket );
            }
        bool freeze;
    };

    struct ConfigSyncClockPacket : public ConfigPacket
    {
        ConfigSyncClockPacket()
            {
                command       = CMD_CONFIG_SYNC_CLOCK;
                size          = sizeof( ConfigSyncClockPacket );
            }

        int64_t time;
    };

    struct ConfigUnmapPacket : public ConfigPacket
    {
        ConfigUnmapPacket()
            {
                command       = CMD_CONFIG_UNMAP;
                size          = sizeof( ConfigUnmapPacket );
            }

        uint32_t requestID;
    };

    struct ConfigUnmapReplyPacket : public ConfigPacket
    {
        ConfigUnmapReplyPacket( const ConfigUnmapPacket* request )
            {
                command       = CMD_CONFIG_UNMAP_REPLY;
                size          = sizeof( ConfigUnmapReplyPacket );
                requestID     = request->requestID;
            }

        uint32_t requestID;
    };

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct NodeConfigInitPacket : public net::ObjectPacket
    {
        NodeConfigInitPacket()
            {
                command        = CMD_NODE_CONFIG_INIT;
                size           = sizeof( NodeConfigInitPacket );
                name[0]        = '\0';
            }

        uint32_t initID;
        uint32_t frameNumber;
        int32_t  iAttributes[ eq::Node::IATTR_ALL ];
        int32_t  tasks;
        EQ_ALIGN8( char name[8] );
    };

    struct NodeConfigInitReplyPacket : public net::ObjectPacket
    {
        NodeConfigInitReplyPacket()
            {
                command   = CMD_NODE_CONFIG_INIT_REPLY;
                size      = sizeof( NodeConfigInitReplyPacket );
                error[0]  = '\0';
            }

        int32_t  iAttributes[ eq::Node::IATTR_ALL ];
        bool     result;
        EQ_ALIGN8( char error[8] );
    };

    struct NodeConfigExitPacket : public net::ObjectPacket
    {
        NodeConfigExitPacket()
            {
                command = CMD_NODE_CONFIG_EXIT;
                size    = sizeof( NodeConfigExitPacket );
            }
    };

    struct NodeConfigExitReplyPacket : public net::ObjectPacket
    {
        NodeConfigExitReplyPacket()
            {
                command   = CMD_NODE_CONFIG_EXIT_REPLY;
                size      = sizeof( NodeConfigExitReplyPacket );
            }

        bool     result;
    };

    struct NodeCreatePipePacket : public net::ObjectPacket
    {
        NodeCreatePipePacket()
            {
                command = CMD_NODE_CREATE_PIPE;
                size    = sizeof( NodeCreatePipePacket );
            }

        uint32_t pipeID;
        bool     threaded;
    };

    struct NodeDestroyPipePacket : public net::ObjectPacket
    {
        NodeDestroyPipePacket()
            {
                command = CMD_NODE_DESTROY_PIPE;
                size    = sizeof( NodeDestroyPipePacket );
            }

        uint32_t pipeID;
    };
    
    struct NodeFrameStartPacket : public net::ObjectPacket
    {
        NodeFrameStartPacket()
            {
                command        = CMD_NODE_FRAME_START;
                size           = sizeof( NodeFrameStartPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishPacket : public net::ObjectPacket
    {
        NodeFrameFinishPacket()
            {
                command          = CMD_NODE_FRAME_FINISH;
                size             = sizeof( NodeFrameFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishReplyPacket : public net::ObjectPacket
    {
        NodeFrameFinishReplyPacket()
            {
                command        = CMD_NODE_FRAME_FINISH_REPLY;
                size           = sizeof( NodeFrameFinishReplyPacket );
            }

        uint32_t frameNumber;
    };
        
    struct NodeFrameDrawFinishPacket : public net::ObjectPacket
    {
        NodeFrameDrawFinishPacket()
            {
                command       = CMD_NODE_FRAME_DRAW_FINISH;
                size          = sizeof( NodeFrameDrawFinishPacket );
            }
        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameTasksFinishPacket : public net::ObjectPacket
    {
        NodeFrameTasksFinishPacket()
            {
                command     = CMD_NODE_FRAME_TASKS_FINISH;
                size        = sizeof( NodeFrameTasksFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    //------------------------------------------------------------
    // Pipe
    //------------------------------------------------------------
    struct PipeCreateWindowPacket : public net::ObjectPacket
    {
        PipeCreateWindowPacket()
            {
                command = CMD_PIPE_CREATE_WINDOW;
                size    = sizeof( PipeCreateWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeDestroyWindowPacket : public net::ObjectPacket
    {
        PipeDestroyWindowPacket()
            {
                command = CMD_PIPE_DESTROY_WINDOW;
                size    = sizeof( PipeDestroyWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeConfigInitPacket : public net::ObjectPacket
    {
        PipeConfigInitPacket()
            {
                command = CMD_PIPE_CONFIG_INIT;
                size    = sizeof( PipeConfigInitPacket );
                name[0] = '\0';
            }

        uint32_t      initID;
        uint32_t      port;
        uint32_t      device;
        uint32_t      frameNumber;
        int32_t       tasks;
        PixelViewport pvp;
        EQ_ALIGN8( char name[8] );
    };

    struct PipeConfigInitReplyPacket : public net::ObjectPacket
    {
        PipeConfigInitReplyPacket()
            {
                command   = CMD_PIPE_CONFIG_INIT_REPLY;
                size      = sizeof( PipeConfigInitReplyPacket );
                error[0]  = '\0';
            }

        bool          result;
        PixelViewport pvp;
        EQ_ALIGN8( char error[8] );
    };

    struct PipeConfigExitPacket : public net::ObjectPacket
    {
        PipeConfigExitPacket()
            {
                command = CMD_PIPE_CONFIG_EXIT;
                size    = sizeof( PipeConfigExitPacket );
            }
        bool exitThread;
    };

    struct PipeConfigExitReplyPacket : public net::ObjectPacket
    {
        PipeConfigExitReplyPacket()
            {
                command   = CMD_PIPE_CONFIG_EXIT_REPLY;
                size      = sizeof( PipeConfigExitReplyPacket );
            }

        bool     result;
    };

    struct PipeFrameStartClockPacket : public net::ObjectPacket
    {
        PipeFrameStartClockPacket()
            {
                command       = CMD_PIPE_FRAME_START_CLOCK;
                size          = sizeof( PipeFrameStartClockPacket );
            }
    };

    struct PipeFrameStartPacket : public net::ObjectPacket
    {
        PipeFrameStartPacket()
            {
                command        = CMD_PIPE_FRAME_START;
                size           = sizeof( PipeFrameStartPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct PipeFrameFinishPacket : public net::ObjectPacket
    {
        PipeFrameFinishPacket()
            {
                command        = CMD_PIPE_FRAME_FINISH;
                size           = sizeof( PipeFrameFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct PipeFrameDrawFinishPacket : public net::ObjectPacket
    {
        PipeFrameDrawFinishPacket()
            {
                command       = CMD_PIPE_FRAME_DRAW_FINISH;
                size          = sizeof( PipeFrameDrawFinishPacket );
            }
        uint32_t frameID;
        uint32_t frameNumber;
    };

    //------------------------------------------------------------
    // Window
    //------------------------------------------------------------
    struct WindowConfigInitPacket : public net::ObjectPacket
    {
        WindowConfigInitPacket()
            {
                command = CMD_WINDOW_CONFIG_INIT;
                size    = sizeof( WindowConfigInitPacket );
                name[0] = '\0';
            }

        uint32_t       initID;
        int32_t        iAttributes[ eq::Window::IATTR_ALL ];
        int32_t        tasks;
        PixelViewport  pvp;
        Viewport       vp;
        EQ_ALIGN8( char name[8] );
    };

    struct WindowConfigInitReplyPacket : public net::ObjectPacket
    {
        WindowConfigInitReplyPacket()
            {
                command   = CMD_WINDOW_CONFIG_INIT_REPLY;
                size      = sizeof( WindowConfigInitReplyPacket );
                error[0]  = '\0';
            }

        PixelViewport           pvp;
        Window::DrawableConfig  drawableConfig;
        bool                    result;
        EQ_ALIGN8( char error[8] );
    };

    struct WindowConfigExitPacket : public net::ObjectPacket
    {
        WindowConfigExitPacket()
            {
                command = CMD_WINDOW_CONFIG_EXIT;
                size    = sizeof( WindowConfigExitPacket );
            }
    };

    struct WindowConfigExitReplyPacket : public net::ObjectPacket
    {
        WindowConfigExitReplyPacket()
            {
                command   = CMD_WINDOW_CONFIG_EXIT_REPLY;
                size      = sizeof( WindowConfigExitReplyPacket );
            }

        bool     result;
    };

    struct WindowCreateChannelPacket : public net::ObjectPacket
    {
        WindowCreateChannelPacket()
            {
                command = CMD_WINDOW_CREATE_CHANNEL;
                size    = sizeof( WindowCreateChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowDestroyChannelPacket : public net::ObjectPacket
    {
        WindowDestroyChannelPacket()
            {
                command = CMD_WINDOW_DESTROY_CHANNEL;
                size    = sizeof( WindowDestroyChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowSetPVPPacket : public net::ObjectPacket
    {
        WindowSetPVPPacket()
            {
                command = CMD_WINDOW_SET_PVP;
                size    = sizeof( WindowSetPVPPacket );
            }

        PixelViewport pvp;
    };

    struct WindowFinishPacket : public net::ObjectPacket
    {
        WindowFinishPacket()
            {
                command = CMD_WINDOW_FINISH;
                size    = sizeof( WindowFinishPacket );
            }
    };

    struct WindowThrottleFramerate : public net::ObjectPacket
    {
        WindowThrottleFramerate()
        {
            command = CMD_WINDOW_THROTTLE_FRAMERATE;
            size    = sizeof( WindowThrottleFramerate );
        }
        float    minFrameTime; // in ms
    };
    
    struct WindowBarrierPacket : public net::ObjectPacket
    {
        WindowBarrierPacket()
            {
                command = CMD_WINDOW_BARRIER;
                size    = sizeof( WindowBarrierPacket );
            }
        net::ObjectVersion barrier;
    };
    
    struct WindowNVBarrierPacket : public net::ObjectPacket
    {
        WindowNVBarrierPacket()
            {
                command = CMD_WINDOW_NV_BARRIER;
                size    = sizeof( WindowNVBarrierPacket );
            }

        uint32_t group;
        uint32_t barrier;
        net::ObjectVersion netBarrier;
    };

    struct WindowSwapPacket : public net::ObjectPacket
    {
        WindowSwapPacket()
            {
                command = CMD_WINDOW_SWAP;
                size    = sizeof( WindowSwapPacket );
            }
    };

    struct WindowFrameStartPacket : public net::ObjectPacket
    {
        WindowFrameStartPacket()
            {
                command        = CMD_WINDOW_FRAME_START;
                size           = sizeof( WindowFrameStartPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct WindowFrameFinishPacket : public net::ObjectPacket
    {
        WindowFrameFinishPacket()
            {
                command        = CMD_WINDOW_FRAME_FINISH;
                size           = sizeof( WindowFrameFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };
        
    struct WindowFrameDrawFinishPacket : public net::ObjectPacket
    {
        WindowFrameDrawFinishPacket()
            {
                command       = CMD_WINDOW_FRAME_DRAW_FINISH;
                size          = sizeof( WindowFrameDrawFinishPacket );
            }
        uint32_t frameID;
        uint32_t frameNumber;
    };

    //------------------------------------------------------------
    // Channel
    //------------------------------------------------------------
    struct ChannelConfigInitPacket : public net::ObjectPacket
    {
        ChannelConfigInitPacket()
            {
                command = CMD_CHANNEL_CONFIG_INIT;
                size    = sizeof( ChannelConfigInitPacket );
                name[0] = '\0';
            }

        uint32_t        initID;
        uint32_t        drawable;
        int32_t         iAttributes[ eq::Channel::IATTR_ALL ];
        int32_t         tasks;
        PixelViewport   pvp;
        Viewport        vp;
        Vector3ub color;
        net::ObjectVersion view;
        EQ_ALIGN8( char name[8] );
    };

    struct ChannelConfigInitReplyPacket : public net::ObjectPacket
    {
        ChannelConfigInitReplyPacket()
            {
                command   = CMD_CHANNEL_CONFIG_INIT_REPLY;
                size      = sizeof( ChannelConfigInitReplyPacket );
                error[0]  = '\0';
            }

        Vector2i maxSize;
        float    nearPlane;
        float    farPlane;
        bool     result;
        EQ_ALIGN8( char error[8] );
    };

    struct ChannelConfigExitPacket : public net::ObjectPacket
    {
        ChannelConfigExitPacket()
            {
                command = CMD_CHANNEL_CONFIG_EXIT;
                size    = sizeof( ChannelConfigExitPacket );
            }
    };

    struct ChannelConfigExitReplyPacket : public net::ObjectPacket
    {
        ChannelConfigExitReplyPacket()
            {
                command   = CMD_CHANNEL_CONFIG_EXIT_REPLY;
                size      = sizeof( ChannelConfigExitReplyPacket );
            }

        bool     result;
    };

    struct ChannelSetNearFarPacket : public net::ObjectPacket
    {
        ChannelSetNearFarPacket()
            {
                command   = CMD_CHANNEL_SET_NEARFAR;
                size      = sizeof( ChannelSetNearFarPacket );
            }

        float    nearPlane;
        float    farPlane;
    };

    struct ChannelTaskPacket : public net::ObjectPacket
    {
        RenderContext context;
    };

    struct ChannelFrameStartPacket : public ChannelTaskPacket
    {
        ChannelFrameStartPacket()
            {
                command        = CMD_CHANNEL_FRAME_START;
                size           = sizeof( ChannelFrameStartPacket );
            }

        uint32_t frameNumber;
    };

    struct ChannelFrameFinishPacket : public ChannelTaskPacket
    {
        ChannelFrameFinishPacket()
            {
                command        = CMD_CHANNEL_FRAME_FINISH;
                size           = sizeof( ChannelFrameFinishPacket );
            }

        uint32_t frameNumber;
    };

    struct ChannelFrameFinishReplyPacket : public net::ObjectPacket
    {
        ChannelFrameFinishReplyPacket( const ChannelFrameFinishPacket* request )
            {
                command     = CMD_CHANNEL_FRAME_FINISH_REPLY;
                size        = sizeof( ChannelFrameFinishReplyPacket );
                sessionID   = request->sessionID;
                objectID    = request->objectID;
                frameNumber = request->frameNumber;
            }

        uint32_t frameNumber;
        uint32_t nStatistics;
        EQ_ALIGN8( Statistic statistics[1] );
    };
        

    struct ChannelFrameDrawFinishPacket : public net::ObjectPacket
    {
        ChannelFrameDrawFinishPacket()
            {
                command       = CMD_CHANNEL_FRAME_DRAW_FINISH;
                size          = sizeof( ChannelFrameDrawFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };
        
    struct ChannelFrameClearPacket : public ChannelTaskPacket
    {
        ChannelFrameClearPacket()
            {
                command       = CMD_CHANNEL_FRAME_CLEAR;
                size          = sizeof( ChannelFrameClearPacket );
            }
    };
        
    struct ChannelFrameDrawPacket : public ChannelTaskPacket
    {
        ChannelFrameDrawPacket()
            {
                command       = CMD_CHANNEL_FRAME_DRAW;
                size          = sizeof( ChannelFrameDrawPacket );
            }
    };
        
    struct ChannelFrameAssemblePacket : public ChannelTaskPacket
    {
        ChannelFrameAssemblePacket()
            {
                command       = CMD_CHANNEL_FRAME_ASSEMBLE;
                size          = sizeof( ChannelFrameAssemblePacket );
            }

        uint32_t             nFrames;
        EQ_ALIGN8( net::ObjectVersion frames[1] );
    };
        
    struct ChannelFrameReadbackPacket : public ChannelTaskPacket
    {
        ChannelFrameReadbackPacket()
            {
                command       = CMD_CHANNEL_FRAME_READBACK;
                size          = sizeof( ChannelFrameReadbackPacket );
            }

        uint32_t             nFrames;
        EQ_ALIGN8( net::ObjectVersion frames[1] );
    };
        
    struct ChannelFrameTransmitPacket : public ChannelTaskPacket
    {
        ChannelFrameTransmitPacket()
            {
                command       = CMD_CHANNEL_FRAME_TRANSMIT;
                size          = sizeof( ChannelFrameTransmitPacket );
            }

        
        net::ObjectVersion frame;
        uint32_t           nNodes;
        EQ_ALIGN8( net::NodeID        nodes[1] );
    };

    struct ChannelFrameViewStartPacket : public ChannelTaskPacket
    {
        ChannelFrameViewStartPacket()
            {
                command       = CMD_CHANNEL_FRAME_VIEW_START;
                size          = sizeof( ChannelFrameViewStartPacket );
            }
    };
        
    struct ChannelFrameViewFinishPacket : public ChannelTaskPacket
    {
        ChannelFrameViewFinishPacket()
            {
                command       = CMD_CHANNEL_FRAME_VIEW_FINISH;
                size          = sizeof( ChannelFrameViewFinishPacket );
            }
    };
        
    //------------------------------------------------------------
    // Frame Data
    //------------------------------------------------------------
    struct FrameDataTransmitPacket : public net::ObjectPacket
    {
        FrameDataTransmitPacket()
            {
                command = CMD_FRAMEDATA_TRANSMIT;
                size    = sizeof( FrameDataTransmitPacket );
            }

        uint32_t      version;
        uint32_t      buffers;
        uint32_t      frameNumber;
        PixelViewport pvp;
        bool          ignoreAlpha;

        EQ_ALIGN8( uint8_t data[8] );
    };

    struct FrameDataReadyPacket : public net::ObjectPacket
    {
        FrameDataReadyPacket()
            {
                command = CMD_FRAMEDATA_READY;
                size    = sizeof( FrameDataReadyPacket );
            }
        uint32_t version;
    };

    struct FrameDataUpdatePacket : public net::ObjectPacket
    {
        FrameDataUpdatePacket()
            {
                command = CMD_FRAMEDATA_UPDATE;
                size    = sizeof( FrameDataUpdatePacket );
            }
        uint32_t version;
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " req " << packet->requestID
           << " renderer " << packet->rendererInfo;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerUseConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " req " << packet->requestID
           << " params " << packet->configInfo;
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
                                       const ServerCreateConfigPacket* packet )
    {
        os << (ServerPacket*)packet << " config " << packet->configID 
            << " request " << packet->requestID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ConfigFrameFinishPacket* packet )
    {
        os << (ConfigPacket*)packet << " frame " << packet->frameNumber;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeCreatePipePacket* packet )
    {
        os << (net::ObjectPacket*)packet << " id " << packet->pipeID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeFrameStartPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeFrameDrawFinishPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const NodeFrameFinishPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeCreateWindowPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " id " << packet->windowID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeConfigInitPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " init id " << packet->initID
           << " port " << packet->port << " device " << packet->device
           << " tasks " << packet->tasks << " frame " << packet->frameNumber;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeConfigInitReplyPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " result " << packet->result;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeFrameStartPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeFrameDrawFinishPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const PipeFrameFinishPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowCreateChannelPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " id " << packet->channelID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const WindowDestroyChannelPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " id " << packet->channelID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                     const WindowConfigInitReplyPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " result " << packet->result
           << " pvp " << packet->pvp;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowFrameStartPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                     const WindowFrameDrawFinishPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowBarrierPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " barrier " << packet->barrier;
        return os;
    }

    inline std::ostream& operator << ( std::ostream& os, 
                                    const ChannelConfigInitReplyPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " result " << packet->result;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelTaskPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " " << packet->context;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelFrameStartPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                    const ChannelFrameDrawFinishPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const ChannelFrameReadbackPacket* packet )
    {
        os << (ChannelTaskPacket*)packet << " nFrames " << packet->nFrames;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const ChannelFrameTransmitPacket* packet )
    {
        os << (net::ObjectPacket*)packet << " frame " << packet->frame
           << " nNodes " << packet->nNodes;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const ChannelFrameAssemblePacket* packet )
    {
        os << (ChannelTaskPacket*)packet << " nFrames " << packet->nFrames;
        return os;
    }
/** @endcond */
}

#endif // EQ_PACKETS_H

