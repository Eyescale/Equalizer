/* Copyright (c) 2009       Maxim Makhinya
   All rights reserved. */

#ifndef EQ_RB_EMPTY_AREAS_FINDER_H
#define EQ_RB_EMPTY_AREAS_FINDER_H

#include "pixelViewport.h"
#include <vector>

namespace eq
{
    typedef std::vector<uint8_t>    byteVec;
    typedef std::vector<uint16_t>   shortVec;

    /**
     * Finds largest empty regions.
     */
    class EQ_EXPORT RBEmptyAreasFinder
    {
    public:
        RBEmptyAreasFinder() : _mask( 0 ) {};
        virtual ~RBEmptyAreasFinder(){};

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

        shortVec _data;
        const uint8_t* _mask;
    };
}

#endif // EQ_RB_EMPTY_AREAS_FINDER_H

