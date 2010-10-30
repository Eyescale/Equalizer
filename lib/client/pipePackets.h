
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

#ifndef EQ_PIPEPACKETS_H
#define EQ_PIPEPACKETS_H

#include <eq/client/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
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
        PipeDestroyWindowPacket( const uint32_t id )
                : windowID( id )
            {
                command = fabric::CMD_PIPE_DESTROY_WINDOW;
                size    = sizeof( PipeDestroyWindowPacket );
            }

        const uint32_t windowID;
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
        PipeConfigExitReplyPacket( const uint32_t pipeID, const bool res )
                : result( res )
            {
                command   = fabric::CMD_PIPE_CONFIG_EXIT_REPLY;
                size      = sizeof( PipeConfigExitReplyPacket );
                objectID  = pipeID;
            }

        const bool result;
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

    struct PipeFrameDrawFinishPacket : public PipePacket
    {
        PipeFrameDrawFinishPacket()
            {
                command       = fabric::CMD_PIPE_FRAME_DRAW_FINISH;
                size          = sizeof( PipeFrameDrawFinishPacket );
            }
        uint32_t frameID;
        uint32_t frameNumber;
    };

    struct PipeExitThreadPacket : public PipePacket
    {
        PipeExitThreadPacket()
            {
                command = fabric::CMD_PIPE_EXIT_THREAD;
                size    = sizeof( PipeExitThreadPacket );
            }
    };

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
}
/** @endcond */
#endif //EQ_PIPEPACKETS_H
