
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

#ifndef EQ_FRAMEDATAPACKETS_H
#define EQ_FRAMEDATAPACKETS_H

#include <eq/client/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
//------------------------------------------------------------
    // Frame Data
    //------------------------------------------------------------
    struct FrameDataTransmitPacket : public FrameDataPacket
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

    struct FrameDataReadyPacket : public FrameDataPacket
    {
        FrameDataReadyPacket()
            {
                command = fabric::CMD_FRAMEDATA_READY;
                size    = sizeof( FrameDataReadyPacket );
            }
        Zoom     zoom;
        uint32_t version;
    };

    struct FrameDataUpdatePacket : public FrameDataPacket
    {
        FrameDataUpdatePacket()
            {
                command = fabric::CMD_FRAMEDATA_UPDATE;
                size    = sizeof( FrameDataUpdatePacket );
            }
        uint32_t version;
    };
}
#endif //EQ_FRAMEDATAPACKETS_H
