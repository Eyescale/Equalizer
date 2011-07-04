
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@googlemail.com>
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

#ifndef EQ_VIEWPACKETS_H
#define EQ_VIEWPACKETS_H

#include <eq/packets.h> // base structs

/** @cond IGNORE */
namespace eq
{
    struct ViewFreezeLoadBalancingPacket : public co::ObjectPacket
    {
        ViewFreezeLoadBalancingPacket()
            {
                command = fabric::CMD_VIEW_FREEZE_LOAD_BALANCING;
                size    = sizeof( ViewFreezeLoadBalancingPacket );
            }
        bool freeze;
    };

    struct ViewTileSizePacket : public co::ObjectPacket
    {
        ViewTileSizePacket()
        {
            command = fabric::CMD_VIEW_TILE_SIZE;
            size    = sizeof( ViewTileSizePacket );
        }
        Vector2i tileSize;
    }; 
}
/** @endcond */

#endif //EQ_VIEWPACKETS_H
