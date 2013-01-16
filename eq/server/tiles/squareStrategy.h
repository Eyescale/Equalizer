
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

#ifndef EQSERVER_TILES_SQUARESTRATEGY_H
#define EQSERVER_TILES_SQUARESTRATEGY_H

namespace eq
{
namespace server
{
namespace tiles
{
    /** Generates tiles for a channel using a square strategy */
    class SquareStrategy
    {
    public:
        void operator()( std::vector< Vector2i >& tiles, const Vector2i& dim )
        {
            const int32_t dimX = dim.x();
            const int32_t dimY = dim.y();
            const size_t nTiles = dim.x() * dim.y();
            int32_t x = ( dimX - 1 ) / 2;
            int32_t y = ( dimY - 1 ) / 2;
            int32_t walk = 0;
            int32_t radius = 0;
            int32_t xStep = 0;
            int32_t yStep = -1; // down

            while( tiles.size() < nTiles )
            {
                LBASSERT ( (yStep == 0 && xStep != 0) ||
                           (yStep != 0 && xStep == 0) );

                if( x>=0 && x<dimX && y>=0 && y<dimY )
                    tiles.push_back( Vector2i( x, y ));

                ++walk;
                if( walk < radius )
                {
                    // keep direction
                    x += xStep;
                    y += yStep;
                }
                else // change direction
                {
                    if( xStep == 1 && yStep == 0 )
                    {
                        // right -> up
                        xStep = 0;
                        yStep = 1;
                    }
                    else if( xStep == 0 && yStep == 1  )
                    {
                        // up -> left
                        xStep = -1;
                        yStep = 0;
                        ++radius;
                    }
                    else if( xStep == -1 && yStep == 0 )
                    {
                        // left -> down
                        xStep = 0;
                        yStep = -1;
                    }
                    else // ( xStep == 0 && yStep == -1 )
                    {
                        // down -> right
                        xStep = 1;
                        yStep = 0;
                        ++radius;
                    }
                    walk = 0;
                    x += xStep;
                    y += yStep;
                }
            }
        }
    };
}
}
}

#endif // EQSERVER_TILES_SQUARESTRATEGY_H
