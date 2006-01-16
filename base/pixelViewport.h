
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_PIXELVIEWPORT_H
#define EQBASE_PIXELVIEWPORT_H

#include "base.h"
#include "viewport.h"

namespace eqBase
{
    /**
     * Holds a pixel viewport along with some methods for manipulation.
     */
    class PixelViewport 
    {
    public:
        /** 
         * Construct a new pixel viewport.
         */
        PixelViewport() : x(0), y(0), w(0), h(0)
            {}

        /** 
         * Reset the pixel viewport to be empty.
         */
        void reset() { x = 0; y = 0; w = 0; h = 0; }

        /** 
         * Apply a fractional viewport to the current pvp.
         * 
         * @param vp the fractional viewport.
         */
        void applyViewport( const Viewport& vp )
            {
                x += (uint32_t)(vp.x * w);
                y += (uint32_t)(vp.y * h);
                w *= (uint32_t)(vp.w);
                h *= (uint32_t)(vp.h);
            }

        uint32_t x;
        uint32_t y;
        uint32_t w;
        uint32_t h;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const PixelViewport& pvp )
    {
        os << "[ " << pvp.x << " " << pvp.y << " " << pvp.w << " " << pvp.h
           << " ]" << std::endl;
        return os;
    }
}

#endif //EQBASE_PIXELVIEWPORT_H
