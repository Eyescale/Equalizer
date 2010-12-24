
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

#ifndef EQ_CONFIGPACKETS_H
#define EQ_CONFIGPACKETS_H

#include <eq/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
    struct ConfigCreateNodePacket : public ConfigPacket
    {
        ConfigCreateNodePacket()
            {
                command = fabric::CMD_CONFIG_CREATE_NODE;
                size    = sizeof( ConfigCreateNodePacket );
            }

        co::base::UUID nodeID;
    };

    struct ConfigDestroyNodePacket : public ConfigPacket
    {
        ConfigDestroyNodePacket( const co::base::UUID& id )
                : nodeID( id )
            {
                command = fabric::CMD_CONFIG_DESTROY_NODE;
                size    = sizeof( ConfigDestroyNodePacket );
            }

        const co::base::UUID nodeID;
    };

    struct ConfigInitPacket : public ConfigPacket
    {
        ConfigInitPacket()
            {
                command   = fabric::CMD_CONFIG_INIT;
                size      = sizeof( ConfigInitPacket );
            }

        uint128_t initID;
        uint32_t requestID;
    };

    struct ConfigInitReplyPacket : public ConfigPacket
    {
        ConfigInitReplyPacket( const ConfigInitPacket* requestPacket )
            {
                command   = fabric::CMD_CONFIG_INIT_REPLY;
                size      = sizeof( ConfigInitReplyPacket );
                requestID = requestPacket->requestID;
                objectID  = requestPacket->objectID;
            }

        uint128_t version;
        uint32_t  requestID;
        bool      result;
    };

    struct ConfigUpdatePacket : public ConfigPacket
    {
        ConfigUpdatePacket()
            {
                command   = fabric::CMD_CONFIG_UPDATE;
                size      = sizeof( ConfigUpdatePacket );
            }
       
        uint32_t versionID;
        uint32_t finishID;
        uint32_t requestID;
    };

    struct ConfigUpdateVersionPacket : public ConfigPacket
    {
        ConfigUpdateVersionPacket( const ConfigUpdatePacket* request,
                                   const uint128_t& v, const uint32_t req )
                : version( v )
                , versionID( request->versionID )
                , finishID( request->finishID )
                , requestID( req )
            {
                command   = fabric::CMD_CONFIG_UPDATE_VERSION;
                size      = sizeof( ConfigUpdateVersionPacket );
                objectID  = request->objectID;
            }

        const uint128_t version;
        const uint32_t versionID;
        const uint32_t finishID;
        const uint32_t requestID;
    };

    struct ConfigUpdateReplyPacket : public ConfigPacket
    {
        ConfigUpdateReplyPacket( const ConfigUpdatePacket* request )
                : requestID( request->requestID )
            {
                command   = fabric::CMD_CONFIG_UPDATE_REPLY;
                size      = sizeof( ConfigUpdateReplyPacket );
                objectID  = request->objectID;
            }
        uint128_t version;
        const uint32_t requestID;
        bool result;
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
        ConfigStartFramePacket( const uint128_t& frameID_ )
                : frameID( frameID_ )
            {
                command   = fabric::CMD_CONFIG_START_FRAME;
                size      = sizeof( ConfigStartFramePacket );
            }
        const uint128_t frameID;
    };

    struct ConfigStopFramesPacket : public ConfigPacket
    {
        ConfigStopFramesPacket()
            {
                command   = fabric::CMD_CONFIG_STOP_FRAMES;
                size      = sizeof( ConfigStopFramesPacket );
            }
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

    struct ConfigSwapObjectPacket : public ConfigPacket
    {
        ConfigSwapObjectPacket()
                : requestID( EQ_UNDEFINED_UINT32 )
        {
            command   = fabric::CMD_CONFIG_SWAP_OBJECT;
            size      = sizeof( ConfigSwapObjectPacket ); 
        }
        uint32_t         requestID;
        co::Object*     object;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ConfigFrameFinishPacket* packet )
    {
        os << (ConfigPacket*)packet << " frame " << packet->frameNumber;
        return os;
    }    
}
/** @endcond */
#endif //EQ_CONFIGPACKETS_H
