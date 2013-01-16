
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSERVER_TILES_RASTERSTRATEGY_H
#define EQSERVER_TILES_RASTERSTRATEGY_H

namespace eq
{
namespace server
{
namespace tiles
{
    /** Generates tiles for a channel using a raster strategy */
    class RasterStrategy
    {
    public:
        void operator()( std::vector< Vector2i >& tiles, const Vector2i& dim )
        {
            for( int y = 0; y < dim.y(); ++y )
                for( int x = 0; x < dim.x(); ++x )
                    tiles.push_back( Vector2i( x, y ));
        }
    };
}
}
}

#endif // EQSERVER_TILES_RASTERSTRATEGY_H
