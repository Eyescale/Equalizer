
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef EQ_WINDOWPACKETS_H
#define EQ_WINDOWPACKETS_H

#include <eq/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
    struct WindowConfigInitPacket : public WindowPacket
    {
        WindowConfigInitPacket()
            {
                command = fabric::CMD_WINDOW_CONFIG_INIT;
                size    = sizeof( WindowConfigInitPacket );
            }

        uint128_t       initID;
    };

    struct WindowConfigInitReplyPacket : public WindowPacket
    {
        WindowConfigInitReplyPacket()
            {
                command   = fabric::CMD_WINDOW_CONFIG_INIT_REPLY;
                size      = sizeof( WindowConfigInitReplyPacket );
            }

        bool           result;
    };

    struct WindowConfigExitPacket : public WindowPacket
    {
        WindowConfigExitPacket()
            {
                command = fabric::CMD_WINDOW_CONFIG_EXIT;
                size    = sizeof( WindowConfigExitPacket );
            }
    };

    struct WindowConfigExitReplyPacket : public WindowPacket
    {
        WindowConfigExitReplyPacket( const co::base::UUID& windowID, const bool res )
                : result( res )
            {
                command   = fabric::CMD_WINDOW_CONFIG_EXIT_REPLY;
                size      = sizeof( WindowConfigExitReplyPacket );
                objectID  = windowID;
            }

        const bool result;
    };

    struct WindowCreateChannelPacket : public WindowPacket
    {
        WindowCreateChannelPacket()
            {
                command = fabric::CMD_WINDOW_CREATE_CHANNEL;
                size    = sizeof( WindowCreateChannelPacket );
            }

        co::base::UUID channelID;
    };

    struct WindowDestroyChannelPacket : public WindowPacket
    {
        WindowDestroyChannelPacket( const co::base::UUID& channelID_ )
                : channelID( channelID_ )
            {
                command = fabric::CMD_WINDOW_DESTROY_CHANNEL;
                size    = sizeof( WindowDestroyChannelPacket );
            }

        const co::base::UUID channelID;
    };

    struct WindowFinishPacket : public WindowPacket
    {
        WindowFinishPacket()
            {
                command = fabric::CMD_WINDOW_FINISH;
                size    = sizeof( WindowFinishPacket );
            }
    };

    struct WindowThrottleFramerate : public WindowPacket
    {
        WindowThrottleFramerate()
        {
            command = fabric::CMD_WINDOW_THROTTLE_FRAMERATE;
            size    = sizeof( WindowThrottleFramerate );
        }
        float    minFrameTime; // in ms
    };
    
    struct WindowBarrierPacket : public WindowPacket
    {
        WindowBarrierPacket()
            {
                command = fabric::CMD_WINDOW_BARRIER;
                size    = sizeof( WindowBarrierPacket );
            }
        co::ObjectVersion barrier;
    };
    
    struct WindowNVBarrierPacket : public WindowPacket
    {
        WindowNVBarrierPacket()
            {
                command = fabric::CMD_WINDOW_NV_BARRIER;
                size    = sizeof( WindowNVBarrierPacket );
            }

        co::ObjectVersion netBarrier;
        uint32_t group;
        uint32_t barrier;
    };

    struct WindowSwapPacket : public WindowPacket
    {
        WindowSwapPacket()
            {
                command = fabric::CMD_WINDOW_SWAP;
                size    = sizeof( WindowSwapPacket );
            }
    };

    struct WindowFrameStartPacket : public WindowPacket
    {
        WindowFrameStartPacket()
            {
                command        = fabric::CMD_WINDOW_FRAME_START;
                size           = sizeof( WindowFrameStartPacket );
            }

        uint128_t version;
        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct WindowFrameFinishPacket : public WindowPacket
    {
        WindowFrameFinishPacket()
            {
                command        = fabric::CMD_WINDOW_FRAME_FINISH;
                size           = sizeof( WindowFrameFinishPacket );
            }

        uint128_t frameID;
        uint32_t frameNumber;
    };
        
    struct WindowFrameDrawFinishPacket : public WindowPacket
    {
        WindowFrameDrawFinishPacket()
            {
                command       = fabric::CMD_WINDOW_FRAME_DRAW_FINISH;
                size          = sizeof( WindowFrameDrawFinishPacket );
            }
        uint128_t frameID;
        uint32_t frameNumber;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowCreateChannelPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " id " << packet->channelID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                      const WindowDestroyChannelPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " id " << packet->channelID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowFrameStartPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                     const WindowFrameDrawFinishPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " frame " << packet->frameNumber
           << " id " << packet->frameID;
        return os;
    }
    inline std::ostream& operator << ( std::ostream& os, 
                                       const WindowBarrierPacket* packet )
    {
        os << (co::ObjectPacket*)packet << " barrier " << packet->barrier;
        return os;
    }
}
/** @endcond */
#endif //EQ_WINDOWPACKETS_H
