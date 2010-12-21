
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

#ifndef EQ_ROI_EMPTY_SPACE_FINDER_H
#define EQ_ROI_EMPTY_SPACE_FINDER_H

#include <eq/types.h>
#include <vector>

namespace eq
{
    /**
     * Finds largest empty regions. @internal
     */
    class ROIEmptySpaceFinder
    {
    public:
        ROIEmptySpaceFinder() : _mask( 0 ) {};
        virtual ~ROIEmptySpaceFinder(){};

        /** Updated data structure from a given mask. Limits should be
            re-initialized after calling this function */
        void update( const uint8_t* mask, const int32_t w, const int32_t h );

        /** Returns maximal empty pvp within a given pvp.
            Uses mask data from update to check if single block is empty! */
        PixelViewport getLargestEmptyArea( const PixelViewport& pvp ) const;

        inline uint16_t getArea( const int32_t x, const int32_t y,
                                 const int32_t w, const int32_t h ) const;

        inline uint16_t getArea( const int32_t x, const int32_t y,
                                 const int32_t w, const int32_t h,
                                 const uint16_t* data ) const;

        void setLimits( const int16_t absolute, const float relative )
        {
            _limAbs = absolute;
            _limRel = relative;
        }

    protected:

    private:
        /** Updates dimensions, resizes data if needed */
        void _resize( const int32_t w, const int32_t h );

        bool _updateMaximalEmptyRegion( const int32_t x, const int32_t y,
                                        const int32_t w, const int32_t h,
                                        PixelViewport& pvp,
                                        const uint16_t* data ) const;

        int32_t _w;
        int32_t _h;

        int16_t _limAbs;
        float   _limRel;

        Vectorus _data;
        const uint8_t* _mask;
    };
}

#endif // EQ_ROI_EMPTY_SPACE_FINDER_H

