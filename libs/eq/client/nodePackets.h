
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/client/packets.h>   // base structs
#include <eq/client/frameData.h> // member

/** @cond IGNORE */
namespace eq
{
    struct NodeFrameDataTransmitPacket : public NodePacket
    {
        NodeFrameDataTransmitPacket()
            {
                command = fabric::CMD_NODE_FRAMEDATA_TRANSMIT;
                size    = sizeof( NodeFrameDataTransmitPacket );
            }

        co::ObjectVersion frameData;
        PixelViewport pvp;
        Zoom          zoom;
        uint32_t      buffers;
        uint32_t      frameNumber;
        uint64_t useAlpha; // bool + valgrind padding

        LB_ALIGN8( uint8_t data[8] );
    };
}
/** @endcond */
#endif //EQ_NODEPACKETS_H
