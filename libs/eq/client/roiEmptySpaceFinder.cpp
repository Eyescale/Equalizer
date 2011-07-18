
/* Copyright (c) 2009, Maxim Makhinya
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "roiEmptySpaceFinder.h"

#include <eq/fabric/pixelViewport.h>

namespace eq
{

void ROIEmptySpaceFinder::_resize( const int32_t w, const int32_t h )
{
    _w = w;
    _h = h;

    _mask = 0;

    if( static_cast<int32_t>(_data.size()) < w * h )
        _data.resize( w * h );

    setLimits( (w-1)*(h-1), 1.0 );
}


void ROIEmptySpaceFinder::update( const uint8_t* mask,
                                 const int32_t  w,
                                 const int32_t  h  )
{
    _resize( w, h );

    _mask = mask;
    const uint8_t*  s = mask + _w*(_h-1);
          uint16_t* d = &_data[_w*(_h-1)];

    // First element
    d[_w-1] = s[_w-1] > 0 ? 1 : 0;

    // First raw
    for( int32_t x = _w-2; x >= 0; x-- )
        d[x] = d[x+1] + ( s[x] > 0 ? 1 : 0 );

    // All other raws
    for( int32_t y = 1; y < _h; y++ )
    {
        const uint16_t* dp = d; // previous calculated raw
        s -= _w;
        d -= _w;
        uint16_t rawSum = 0;
        for( int32_t x = _w-1; x >= 0; x-- )
        {
            rawSum += s[x] > 0 ? 1 : 0;
            d[x] = dp[x] + rawSum;
        }
    }
}


inline uint16_t ROIEmptySpaceFinder::getArea(
                                    const int32_t x, const int32_t y,
                                    const int32_t w, const int32_t h ) const
{
    return  getArea( x, y, w, h, &_data[0] + y * _w + x );
}

inline uint16_t ROIEmptySpaceFinder::getArea(
                                    const int32_t x, const int32_t y,
                                    const int32_t w, const int32_t h,
                                    const uint16_t* data ) const
{
    EQASSERT( x >= 0 && w > 0 && x+w < _w );
    EQASSERT( y >= 0 && h > 0 && y+h < _h );

    const uint16_t* data_ = data + h*_w;
    return  *data - data[ w ] - *data_ + data_[ w ];
}


bool ROIEmptySpaceFinder::_updateMaximalEmptyRegion(
                                    const int32_t x, const int32_t y,
                                    const int32_t w, const int32_t h,
                                    PixelViewport& pvp,
                                    const uint16_t* data ) const
{
    uint16_t maxArea = pvp.w * pvp.h;
    bool updated = false;
    int32_t maxW = 0;
    int32_t maxH = 0;

    // find biggest diagonal
    int32_t cwh = 2;
    while( cwh <= w && cwh <= h && getArea( x, y, cwh, cwh, data ) == 0 )
        cwh++;

    cwh--;

    if( cwh*cwh > maxArea )
    {
        maxArea = cwh*cwh;
        maxW = cwh;
        maxH = cwh;
        updated = true;
    }

    // check parallelepipids
    if( cwh * w > maxArea )
    {
        int32_t ch = cwh;
        for( int32_t cw = cwh+1; cw <= w; cw++ )
        {
            while( getArea( x, y, cw, ch, data ) != 0 )
            {
                ch--;
                if( ch == 0 )
                {
                    cw = w + 1;
                    break;
                }
            }
            if( cw*ch > maxArea )
            {
                maxArea = cw*ch;
                maxW = cw;
                maxH = ch;
                updated = true;
            }
        }
    }

    if( cwh * h > maxArea )
    {
        int32_t cw = cwh;
        for( int32_t ch = cwh+1; ch <= h; ch++ )
        {
            while( getArea( x, y, cw, ch, data ) != 0 )
            {
                cw--;
                if( cw == 0 )
                {
                    ch = h + 1;
                    break;
                }
            }
            if( cw*ch > maxArea )
            {
                maxArea = cw*ch;
                maxW = cw;
                maxH = ch;
                updated = true;
            }
        }
    }

    if( updated )
        pvp = PixelViewport( x, y, maxW, maxH );

    return updated;
}

PixelViewport ROIEmptySpaceFinder::getLargestEmptyArea(const PixelViewport& pvp)
const
{
    EQASSERT(   pvp.x >= 0    && pvp.w > 0 &&
                pvp.y >= 0    && pvp.h > 0 &&
                pvp.x + pvp.w < _w && 
                pvp.y + pvp.h < _h );

    EQASSERT( _mask );

    PixelViewport res( pvp.x, pvp.y, 0, 0 );

          uint16_t maxArea = getArea( pvp.x, pvp.y, pvp.w, pvp.h );
    const uint16_t minRel  = static_cast<uint16_t>( pvp.w * pvp.h * _limRel );

    // totally empty
    if( maxArea == 0 )
        return pvp;

    // totally full
    if( maxArea == pvp.w*pvp.h )
        return res;

    // over limit
    if( maxArea < _limAbs || maxArea < minRel )
        return res;

    // search for biggest empty pvp
    const uint16_t* data    = &_data[0] + pvp.y * _w;
    const uint8_t*  m       = _mask     + pvp.y * _w;

    maxArea = 0;

    for( int y=pvp.y, h=pvp.h; h > 0; y++, h-- )
    {
        // skeep if found area bigger than the rest of image
        if( h * pvp.w - getArea( pvp.x, y, pvp.w, h ) <= maxArea )
            break;

        for( int x=pvp.x, w=pvp.w; w > 0; x++, w-- )
        {
            // skeep non empty blocks
            if( m[x] != 0 )
                continue;

            // skeep if found area bigger than the rest of image
            if( w*h - getArea( x, y, w, h, data + x ) <= maxArea )
            {
                w = 0;
                continue;
            }

            if( _updateMaximalEmptyRegion( x, y, w, h, res, data + x ))
                maxArea = res.w * res.h;
        }
        m    += _w;
        data += _w;
    }

    const uint16_t curArea = res.w * res.h;
    if( curArea < _limAbs || curArea < minRel )
        return PixelViewport( pvp.x, pvp.y, 0, 0 );

    return res;
}


}
