
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

#ifndef EQ_NODEPACKETS_H
#define EQ_NODEPACKETS_H

#include <eq/client/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{

    struct NodeConfigInitPacket : public net::ObjectPacket
    {
        NodeConfigInitPacket()
            {
                command        = fabric::CMD_NODE_CONFIG_INIT;
                size           = sizeof( NodeConfigInitPacket );
            }

        uint128_t initID;
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

    struct NodeConfigExitPacket : public NodePacket
    {
        NodeConfigExitPacket()
            {
                command = fabric::CMD_NODE_CONFIG_EXIT;
                size    = sizeof( NodeConfigExitPacket );
            }
    };

    struct NodeConfigExitReplyPacket : public NodePacket
    {
        NodeConfigExitReplyPacket( const uint32_t nodeID, const bool res )
                : result( res )
            {
                command   = fabric::CMD_NODE_CONFIG_EXIT_REPLY;
                size      = sizeof( NodeConfigExitReplyPacket );
                objectID  = nodeID;
            }

        const bool result;
    };

    struct NodeCreatePipePacket : public NodePacket
    {
        NodeCreatePipePacket()
            {
                command = fabric::CMD_NODE_CREATE_PIPE;
                size    = sizeof( NodeCreatePipePacket );
            }

        uint32_t pipeID;
        bool     threaded;
    };

    struct NodeDestroyPipePacket : public NodePacket
    {
        NodeDestroyPipePacket( const uint32_t id )
                : pipeID( id )
            {
                command = fabric::CMD_NODE_DESTROY_PIPE;
                size    = sizeof( NodeDestroyPipePacket );
            }

        const uint32_t pipeID;
    };
    
    struct NodeFrameStartPacket : public NodePacket
    {
        NodeFrameStartPacket()
                : configVersion( net::VERSION_INVALID )
            {
                command        = fabric::CMD_NODE_FRAME_START;
                size           = sizeof( NodeFrameStartPacket );
            }

        uint128_t version;
        uint128_t configVersion;
        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishPacket : public NodePacket
    {
        NodeFrameFinishPacket()
            {
                command          = fabric::CMD_NODE_FRAME_FINISH;
                size             = sizeof( NodeFrameFinishPacket );
            }

        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameFinishReplyPacket : public NodePacket
    {
        NodeFrameFinishReplyPacket()
            {
                command        = fabric::CMD_NODE_FRAME_FINISH_REPLY;
                size           = sizeof( NodeFrameFinishReplyPacket );
            }

        uint32_t frameNumber;
    };
        
    struct NodeFrameDrawFinishPacket : public NodePacket
    {
        NodeFrameDrawFinishPacket()
            {
                command       = fabric::CMD_NODE_FRAME_DRAW_FINISH;
                size          = sizeof( NodeFrameDrawFinishPacket );
            }
        uint128_t frameID;
        uint32_t frameNumber;
    };

    struct NodeFrameTasksFinishPacket : public NodePacket
    {
        NodeFrameTasksFinishPacket()
            {
                command     = fabric::CMD_NODE_FRAME_TASKS_FINISH;
                size        = sizeof( NodeFrameTasksFinishPacket );
            }

        uint128_t frameID;
        uint32_t frameNumber;
    };

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
}
/** @endcond */
#endif //EQ_NODEPACKETS_H
