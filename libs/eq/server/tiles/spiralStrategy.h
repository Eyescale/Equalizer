
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

#ifndef EQSERVER_TILES_SPIRALSTRATEGY_H
#define EQSERVER_TILES_SPIRALSTRATEGY_H

namespace eq
{
namespace server
{
namespace tiles
{
    /** Generates tiles for a channel using a spiral strategy */
    class SpiralStrategy
    {
    public:
        void operator()( std::vector< Vector2i >& tiles, const Vector2i& dim )
        {
            const int32_t dimX = dim.x();
            const int32_t dimY = dim.y();

            int32_t level = ( dimY < dimX ? dimY-1 : dimX-1 ) / 2;
            int32_t x = 0;
            int32_t y = 0;

            while( level >= 0 )
            {
                for( x = level, y = level+1; y < dimY-level; ++y )
                    tiles.push_back( Vector2i( x, y ));

                for( x = level+1, y = dimY-1-level; x < dimX-1-level; ++x )
                    tiles.push_back( Vector2i( x, y ));

                for( x = dimX-1-level, y = dimY-1-level; y > level ; --y )
                    tiles.push_back( Vector2i( x, y ));

                for( x = dimX-1-level, y = level; x > level-1 ; --x )
                    tiles.push_back( Vector2i( x, y ));
                --level;
            }
        }
    };
}
}
}

#endif // EQSERVER_TILES_SPIRALSTRATEGY_H
