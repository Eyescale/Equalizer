
/* Copyright (c) 2012-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_TILE_H
#define EQFABRIC_TILE_H

#include <eq/fabric/pixelViewport.h>
#include <eq/fabric/viewport.h>

namespace eq
{
namespace fabric
{
/** @internal */
class Tile
{
public:
    Tile() {}
    Tile( const PixelViewport& pvp_, const Viewport& vp_ )
        : pvp( pvp_ ), vp( vp_ ) {}

    Frustumf frustum;
    Frustumf ortho;
    PixelViewport pvp;
    Viewport vp;
};
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Tile& tile )
{
    byteswap( tile.frustum );
    byteswap( tile.ortho );
    byteswap( tile.pvp );
    byteswap( tile.vp );
}
}

#endif // EQFABRIC_TILE_H
