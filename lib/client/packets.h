
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric Stalder@gmail.com> 
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

#include <eq/client/statistic.h>     // member

#include <eq/fabric/commands.h>      // enum 
#include <eq/fabric/packetType.h>    // member
#include <eq/fabric/packets.h>
#include <eq/fabric/renderContext.h> // member
#include <eq/fabric/viewport.h>      // member

namespace eq
{
    class Pipe;
    class Window;

/** @cond IGNORE */
    //------------------------------------------------------------
    // Client
    //------------------------------------------------------------
    struct ClientPacket : public net::Packet
    {
        ClientPacket(){ type = fabric::PACKETTYPE_EQ_CLIENT; }
    };

    struct ClientExitPacket : public ClientPacket
    {
        ClientExitPacket()
            {
                command = fabric::CMD_CLIENT_EXIT;
                size    = sizeof( ClientExitPacket );
            }
    };

    //------------------------------------------------------------
    // Server
    //------------------------------------------------------------
    struct ServerChooseConfigPacket : public fabric::ServerPacket
    {
        ServerChooseConfigPacket()
                : fill ( 0 )
            {
                command = fabric::CMD_SERVER_CHOOSE_CONFIG;
                size    = sizeof( ServerChooseConfigPacket );
                rendererInfo[0] = '\0';
            }

        uint32_t requestID;
        uint32_t fill;
        EQ_ALIGN8( char rendererInfo[8] );
    };

    struct ServerChooseConfigReplyPacket : public fabric::ServerPacket
    {
        ServerChooseConfigReplyPacket( const ServerChooseConfigPacket*
                                       requestPacket )
            {
                command   = fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY;
                size      = sizeof( ServerChooseConfigReplyPacket );
                requestID = requestPacket->requestID;
            }

        net::SessionID configID;
        uint32_t requestID;
    };

    struct ServerReleaseConfigPacket : public fabric::ServerPacket
    {
        ServerReleaseConfigPacket()
            {
                command = fabric::CMD_SERVER_RELEASE_CONFIG;
                size    = sizeof( ServerReleaseConfigPacket );
            }

        net::SessionID configID;
        uint32_t requestID;
    };

    struct ServerReleaseConfigReplyPacket : public fabric::ServerPacket
    {
        ServerReleaseConfigReplyPacket( const ServerReleaseConfigPacket*
                                        requestPacket )
            {
                command   = fabric::CMD_SERVER_RELEASE_CONFIG_REPLY;
                size      = sizeof( ServerReleaseConfigReplyPacket );
                requestID = requestPacket->requestID;
            }

        uint32_t requestID;
    };

    struct ServerShutdownPacket : public fabric::ServerPacket
    {
        ServerShutdownPacket()
            {
                command = fabric::CMD_SERVER_SHUTDOWN;
                size    = sizeof( ServerShutdownPacket );
            }

        uint32_t requestID;
    };

    struct ServerShutdownReplyPacket : public fabric::ServerPacket
    {
        ServerShutdownReplyPacket( const ServerShutdownPacket* requestPacket )
            {
                command   = fabric::CMD_SERVER_SHUTDOWN_REPLY;
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

    struct ConfigCreateNodePacket : public ConfigPacket
    {
        ConfigCreateNodePacket()
            {
                command = fabric::CMD_CONFIG_CREATE_NODE;
                size    = sizeof( ConfigCreateNodePacket );
            }

        uint32_t nodeID;
    };

    struct ConfigDestroyNodePacket : public ConfigPacket
    {
        ConfigDestroyNodePacket()
            {
                command = fabric::CMD_CONFIG_DESTROY_NODE;
                size    = sizeof( ConfigDestroyNodePacket );
            }

        uint32_t nodeID;
    };

    struct ConfigInitPacket : public ConfigPacket
    {
        ConfigInitPacket()
            {
                command   = fabric::CMD_CONFIG_INIT;
                size      = sizeof( ConfigInitPacket );
            }

        uint32_t requestID;
        uint32_t initID;
    };

    struct ConfigInitReplyPacket : public ConfigPacket
    {
        ConfigInitReplyPacket( const ConfigInitPacket* requestPacket )
            {
                command   = fabric::CMD_CONFIG_INIT_REPLY;
                size      = sizeof( ConfigInitReplyPacket );
                requestID = requestPacket->requestID;
                sessionID = requestPacket->sessionID;
            }

        uint32_t requestID;
        uint32_t version;
        bool     result;
    };

    struct ConfigExitPacket : public ConfigPacket
    {
        ConfigExitPacket()
            {
                command   = fabric::CMD_CONFIG_EXIT;
                size      = sizeof( ConfigExitPacket );
            }
        uint32_t requestID;
    };

    struct ConfigExitReplyPacket : public ConfigPacket
    {
        ConfigExitReplyPacket( const ConfigExitPacket* requestPacket )
            {
                command   = fabric::CMD_CONFIG_EXIT_REPLY;
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
                command   = fabric::CMD_CONFIG_START_FRAME;
                size      = sizeof( ConfigStartFramePacket );
            }
        uint32_t frameID;
        uint32_t syncID;
        uint32_t startID;
    };

    struct ConfigSyncPacket : public ConfigPacket
    {
        ConfigSyncPacket( const ConfigStartFramePacket* request,
                          const uint32_t version_ )
                : requestID( request->syncID )
                , version( version_ )
            {
                command   = fabric::CMD_CONFIG_SYNC;
                size      = sizeof( ConfigSyncPacket );
            }
        const uint32_t requestID;
        const uint32_t version;
    };

    struct ConfigStartFrameReplyPacket : public ConfigPacket
    {
        ConfigStartFrameReplyPacket( const ConfigStartFramePacket* request,
                                     const bool finish_ )
                : requestID( request->startID )
                , finish( finish_ )
            {
                command   = fabric::CMD_CONFIG_START_FRAME_REPLY;
                size      = sizeof( ConfigStartFrameReplyPacket );
            }
        const uint32_t requestID;
        const bool finish;
    };

    struct ConfigReleaseFrameLocalPacket : public ConfigPacket
    {
        ConfigReleaseFrameLocalPacket()
            {
                command       = fabric::CMD_CONFIG_RELEASE_FRAME_LOCAL;
                size          = sizeof( ConfigReleaseFrameLocalPacket );
            }
        uint32_t frameNumber;
    };

    struct ConfigFrameFinishPacket : public ConfigPacket
    {
        ConfigFrameFinishPacket()
            {
                command     = fabric::CMD_CONFIG_FRAME_FINISH;
                size        = sizeof( ConfigFrameFinishPacket );
            }
        uint32_t frameNumber;
    };

    struct ConfigFinishAllFramesPacket : public ConfigPacket
    {
        ConfigFinishAllFramesPacket()
            {
                command   = fabric::CMD_CONFIG_FINISH_ALL_FRAMES;
                size      = sizeof( ConfigFinishAllFramesPacket );
            }
    };

    struct ConfigFreezeLoadBalancingPacket : public ConfigPacket
    {
        ConfigFreezeLoadBalancingPacket()
            {
                command = fabric::CMD_CONFIG_FREEZE_LOAD_BALANCING;
                size    = sizeof( ConfigFreezeLoadBalancingPacket );
            }
        bool freeze;
    };

    struct ConfigSyncClockPacket : public ConfigPacket
    {
        ConfigSyncClockPacket()
            {
                command       = fabric::CMD_CONFIG_SYNC_CLOCK;
                size          = sizeof( ConfigSyncClockPacket );
            }

        int64_t time;
    };

    //------------------------------------------------------------
    // Node
    //------------------------------------------------------------
    struct NodeConfigInitPacket : public net::ObjectPacket
    {
        NodeConfigInitPacket()
            {
                command        = fabric::CMD_NODE_CONFIG_INIT;
                size           = sizeof( NodeConfigInitPacket );
            }

        uint32_t initID;
        uint32_t frameNumber;
    };

    struct NodeConfigInitReplyPacket : public net::ObjectPacket
    {
        NodeConfigInitReplyPacket()
            {
                command   = fabric::CMD_NODE_CONFIG_INIT_REPLY;
                size      = sizeof( NodeConfigInitReplyPacket );
            }

        bool     result;
    };

    struct NodeConfigExitPacket : public net::ObjectPacket
    {
        NodeConfigExitPacket()
            {
                command = fabric::CMD_NODE_CONFIG_EXIT;
                size    = sizeof( NodeConfigExitPacket );
            }
    };

    struct NodeConfigExitReplyPacket : public net::ObjectPacket
    {
        NodeConfigExitReplyPacket()
            {
                command   = fabric::CMD_NODE_CONFIG_EXIT_REPLY;
                size      = sizeof( NodeConfigExitReplyPacket );
            }

        bool     result;
    };

    struct NodeCreatePipePacket : public net::ObjectPacket
    {
        NodeCreatePipePacket()
            {
                command = fabric::CMD_NODE_CREATE_PIPE;
                size    = sizeof( NodeCreatePipePacket );
            }

        uint32_t pipeID;
        bool     threaded;
    };

    struct NodeDestroyPipePacket : public net::ObjectPacket
    {
        NodeDestroyPipePacket()
            {
                command = fabric::CMD_NODE_DESTROY_PIPE;
                size    = sizeof( NodeDestroyPipePacket );
            }

        uint32_t pipeID;
    };
    
    struct NodeFrameStartPacket : public net::ObjectPacket
    {
        NodeFrameStartPacket()
                : configVersion( net::VERSION_INVALID )
            {
                command        = fabric::CMD_NODE_FRAME_START;
                size           = sizeof( NodeFrameStartPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
        uint32_t version;
        uint32_t configVersion;
    };

    struct NodeFrameFinishPacket : public net::ObjectPacket
    {
        NodeFrameFinishPacket()
            {
                command          = fabric::CMD_NODE_FRAME_FINISH;
                size             = sizeof( NodeFrameFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishReplyPacket : public net::ObjectPacket
    {
        NodeFrameFinishReplyPacket()
            {
                command        = fabric::CMD_NODE_FRAME_FINISH_REPLY;
                size           = sizeof( NodeFrameFinishReplyPacket );
            }

        uint32_t frameNumber;
    };
        
    struct NodeFrameDrawFinishPacket : public net::ObjectPacket
    {
        NodeFrameDrawFinishPacket()
            {
                command       = fabric::CMD_NODE_FRAME_DRAW_FINISH;
                size          = sizeof( NodeFrameDrawFinishPacket );
            }
        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameTasksFinishPacket : public net::ObjectPacket
    {
        NodeFrameTasksFinishPacket()
            {
                command     = fabric::CMD_NODE_FRAME_TASKS_FINISH;
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
                command = fabric::CMD_PIPE_CREATE_WINDOW;
                size    = sizeof( PipeCreateWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeDestroyWindowPacket : public net::ObjectPacket
    {
        PipeDestroyWindowPacket()
            {
                command = fabric::CMD_PIPE_DESTROY_WINDOW;
                size    = sizeof( PipeDestroyWindowPacket );
            }

        uint32_t windowID;
    };

    struct PipeConfigInitPacket : public net::ObjectPacket
    {
        PipeConfigInitPacket()
            {
                command = fabric::CMD_PIPE_CONFIG_INIT;
                size    = sizeof( PipeConfigInitPacket );
            }

        uint32_t      initID;
        uint32_t      frameNumber;
    };

    struct PipeConfigInitReplyPacket : public net::ObjectPacket
    {
        PipeConfigInitReplyPacket()
            {
                command   = fabric::CMD_PIPE_CONFIG_INIT_REPLY;
                size      = sizeof( PipeConfigInitReplyPacket );
            }

        bool          result;
    };

    struct PipeConfigExitPacket : public net::ObjectPacket
    {
        PipeConfigExitPacket()
            {
                command = fabric::CMD_PIPE_CONFIG_EXIT;
                size    = sizeof( PipeConfigExitPacket );
            }
    };

    struct PipeConfigExitReplyPacket : public net::ObjectPacket
    {
        PipeConfigExitReplyPacket()
            {
                command   = fabric::CMD_PIPE_CONFIG_EXIT_REPLY;
                size      = sizeof( PipeConfigExitReplyPacket );
            }

        bool     result;
    };

    struct PipeFrameStartClockPacket : public net::ObjectPacket
    {
        PipeFrameStartClockPacket()
            {
                command       = fabric::CMD_PIPE_FRAME_START_CLOCK;
                size          = sizeof( PipeFrameStartClockPacket );
            }
    };

    struct PipeFrameStartPacket : public net::ObjectPacket
    {
        PipeFrameStartPacket()
            {
                command        = fabric::CMD_PIPE_FRAME_START;
                size           = sizeof( PipeFrameStartPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
        uint32_t version;
    };

    struct PipeFrameFinishPacket : public net::ObjectPacket
    {
        PipeFrameFinishPacket()
            {
                command        = fabric::CMD_PIPE_FRAME_FINISH;
                size           = sizeof( PipeFrameFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct PipeFrameDrawFinishPacket : public net::ObjectPacket
    {
        PipeFrameDrawFinishPacket()
            {
                command       = fabric::CMD_PIPE_FRAME_DRAW_FINISH;
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
                command = fabric::CMD_WINDOW_CONFIG_INIT;
                size    = sizeof( WindowConfigInitPacket );
            }

        uint32_t       initID;
    };

    struct WindowConfigInitReplyPacket : public net::ObjectPacket
    {
        WindowConfigInitReplyPacket()
            {
                command   = fabric::CMD_WINDOW_CONFIG_INIT_REPLY;
                size      = sizeof( WindowConfigInitReplyPacket );
            }

        bool            result;
    };

    struct WindowConfigExitPacket : public net::ObjectPacket
    {
        WindowConfigExitPacket()
            {
                command = fabric::CMD_WINDOW_CONFIG_EXIT;
                size    = sizeof( WindowConfigExitPacket );
            }
    };

    struct WindowConfigExitReplyPacket : public net::ObjectPacket
    {
        WindowConfigExitReplyPacket()
            {
                command   = fabric::CMD_WINDOW_CONFIG_EXIT_REPLY;
                size      = sizeof( WindowConfigExitReplyPacket );
            }

        bool     result;
    };

    struct WindowCreateChannelPacket : public net::ObjectPacket
    {
        WindowCreateChannelPacket()
            {
                command = fabric::CMD_WINDOW_CREATE_CHANNEL;
                size    = sizeof( WindowCreateChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowDestroyChannelPacket : public net::ObjectPacket
    {
        WindowDestroyChannelPacket()
            {
                command = fabric::CMD_WINDOW_DESTROY_CHANNEL;
                size    = sizeof( WindowDestroyChannelPacket );
            }

        uint32_t channelID;
    };

    struct WindowFinishPacket : public net::ObjectPacket
    {
        WindowFinishPacket()
            {
                command = fabric::CMD_WINDOW_FINISH;
                size    = sizeof( WindowFinishPacket );
            }
    };

    struct WindowThrottleFramerate : public net::ObjectPacket
    {
        WindowThrottleFramerate()
        {
            command = fabric::CMD_WINDOW_THROTTLE_FRAMERATE;
            size    = sizeof( WindowThrottleFramerate );
        }
        float    minFrameTime; // in ms
    };
    
    struct WindowBarrierPacket : public net::ObjectPacket
    {
        WindowBarrierPacket()
            {
                command = fabric::CMD_WINDOW_BARRIER;
                size    = sizeof( WindowBarrierPacket );
            }
        net::ObjectVersion barrier;
    };
    
    struct WindowNVBarrierPacket : public net::ObjectPacket
    {
        WindowNVBarrierPacket()
            {
                command = fabric::CMD_WINDOW_NV_BARRIER;
                size    = sizeof( WindowNVBarrierPacket );
            }

        net::ObjectVersion netBarrier;
        uint32_t group;
        uint32_t barrier;
    };

    struct WindowSwapPacket : public net::ObjectPacket
    {
        WindowSwapPacket()
            {
                command = fabric::CMD_WINDOW_SWAP;
                size    = sizeof( WindowSwapPacket );
            }
    };

    struct WindowFrameStartPacket : public net::ObjectPacket
    {
        WindowFrameStartPacket()
            {
                command        = fabric::CMD_WINDOW_FRAME_START;
                size           = sizeof( WindowFrameStartPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
        uint32_t version;
    };

    struct WindowFrameFinishPacket : public net::ObjectPacket
    {
        WindowFrameFinishPacket()
            {
                command        = fabric::CMD_WINDOW_FRAME_FINISH;
                size           = sizeof( WindowFrameFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };
        
    struct WindowFrameDrawFinishPacket : public net::ObjectPacket
    {
        WindowFrameDrawFinishPacket()
            {
                command       = fabric::CMD_WINDOW_FRAME_DRAW_FINISH;
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
                command = fabric::CMD_CHANNEL_CONFIG_INIT;
                size    = sizeof( ChannelConfigInitPacket );
            }

        uint32_t        initID;
    };

    struct ChannelConfigInitReplyPacket : public net::ObjectPacket
    {
        ChannelConfigInitReplyPacket()
            {
                command   = fabric::CMD_CHANNEL_CONFIG_INIT_REPLY;
                size      = sizeof( ChannelConfigInitReplyPacket );
            }

        bool result;
    };

    struct ChannelConfigExitPacket : public net::ObjectPacket
    {
        ChannelConfigExitPacket()
            {
                command = fabric::CMD_CHANNEL_CONFIG_EXIT;
                size    = sizeof( ChannelConfigExitPacket );
            }
    };

    struct ChannelConfigExitReplyPacket : public net::ObjectPacket
    {
        ChannelConfigExitReplyPacket()
            {
                command   = fabric::CMD_CHANNEL_CONFIG_EXIT_REPLY;
                size      = sizeof( ChannelConfigExitReplyPacket );
            }

        bool     result;
    };

    struct ChannelTaskPacket : public net::ObjectPacket
    {
        RenderContext context;
    };

    struct ChannelFrameStartPacket : public ChannelTaskPacket
    {
        ChannelFrameStartPacket()
            {
                command        = fabric::CMD_CHANNEL_FRAME_START;
                size           = sizeof( ChannelFrameStartPacket );
            }

        uint32_t frameNumber;
        uint32_t version;
    };

    struct ChannelFrameFinishPacket : public ChannelTaskPacket
    {
        ChannelFrameFinishPacket()
            {
                command        = fabric::CMD_CHANNEL_FRAME_FINISH;
                size           = sizeof( ChannelFrameFinishPacket );
            }

        uint32_t frameNumber;
    };

    struct ChannelFrameFinishReplyPacket : public net::ObjectPacket
    {
        ChannelFrameFinishReplyPacket( const ChannelFrameFinishPacket* request )
            {
                command     = fabric::CMD_CHANNEL_FRAME_FINISH_REPLY;
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
                command       = fabric::CMD_CHANNEL_FRAME_DRAW_FINISH;
                size          = sizeof( ChannelFrameDrawFinishPacket );
            }

        uint32_t frameID;
        uint32_t frameNumber;
    };
        
    struct ChannelFrameClearPacket : public ChannelTaskPacket
    {
        ChannelFrameClearPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_CLEAR;
                size          = sizeof( ChannelFrameClearPacket );
            }
    };
        
    struct ChannelFrameDrawPacket : public ChannelTaskPacket
    {
        ChannelFrameDrawPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_DRAW;
                size          = sizeof( ChannelFrameDrawPacket );
            }
    };
        
    struct ChannelFrameAssemblePacket : public ChannelTaskPacket
    {
        ChannelFrameAssemblePacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_ASSEMBLE;
                size          = sizeof( ChannelFrameAssemblePacket );
            }

        uint32_t             nFrames;
        EQ_ALIGN8( net::ObjectVersion frames[1] );
    };
        
    struct ChannelFrameReadbackPacket : public ChannelTaskPacket
    {
        ChannelFrameReadbackPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_READBACK;
                size          = sizeof( ChannelFrameReadbackPacket );
            }

        uint32_t             nFrames;
        EQ_ALIGN8( net::ObjectVersion frames[1] );
    };
        
    struct ChannelFrameTransmitPacket : public ChannelTaskPacket
    {
        ChannelFrameTransmitPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_TRANSMIT;
                size          = sizeof( ChannelFrameTransmitPacket );
            }

        
        net::ObjectVersion frame;
        uint32_t           nNodes;
        EQ_ALIGN8( net::NodeID nodes[1] );
    };

    struct ChannelFrameViewStartPacket : public ChannelTaskPacket
    {
        ChannelFrameViewStartPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_VIEW_START;
                size          = sizeof( ChannelFrameViewStartPacket );
            }
    };
        
    struct ChannelFrameViewFinishPacket : public ChannelTaskPacket
    {
        ChannelFrameViewFinishPacket()
            {
                command       = fabric::CMD_CHANNEL_FRAME_VIEW_FINISH;
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
                command = fabric::CMD_FRAMEDATA_TRANSMIT;
                size    = sizeof( FrameDataTransmitPacket );
            }

        PixelViewport pvp;
        uint32_t      version;
        uint32_t      buffers;
        uint32_t      frameNumber;
        bool          ignoreAlpha;

        EQ_ALIGN8( uint8_t data[8] );
    };

    struct FrameDataReadyPacket : public net::ObjectPacket
    {
        FrameDataReadyPacket()
            {
                command = fabric::CMD_FRAMEDATA_READY;
                size    = sizeof( FrameDataReadyPacket );
            }
        Zoom     zoom;
        uint32_t version;
    };

    struct FrameDataUpdatePacket : public net::ObjectPacket
    {
        FrameDataUpdatePacket()
            {
                command = fabric::CMD_FRAMEDATA_UPDATE;
                size    = sizeof( FrameDataUpdatePacket );
            }
        uint32_t version;
    };

    //------------------------------------------------------------
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ServerChooseConfigPacket* packet )
    {
        os << (fabric::ServerPacket*)packet << " req " << packet->requestID
           << " renderer " << packet->rendererInfo;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                  const ServerChooseConfigReplyPacket* packet )
    {
        os << (fabric::ServerPacket*)packet << " req " << packet->requestID
           << " id " << packet->configID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
        const ServerReleaseConfigPacket* packet )
    {
        os << (fabric::ServerPacket*)packet << " config " << packet->configID;
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
        os << (net::ObjectPacket*)packet << " init id "
           << " frame " << packet->frameNumber;
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
                                   const ChannelFrameFinishReplyPacket* packet )
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

