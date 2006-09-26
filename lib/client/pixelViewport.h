
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELVIEWPORT_H
#define EQ_PIXELVIEWPORT_H

#include <eq/base/base.h>

#include "viewport.h"

namespace eq
{
    /**
     * Holds a pixel viewport along with some methods for manipulation.
     */
    class PixelViewport 
    {
    public:
        /**
         * @name Constructors
         */
        //*{
        PixelViewport() : x(0), y(0), w(0), h(0)  {}

        PixelViewport( const int32_t x, const int32_t y, 
                       const int32_t w, const int32_t h )
                : x(x), y(y), w(w), h(h)  {}
        //*}

        /** 
         * Reset the pixel viewport to be empty.
         */
        void reset() { x = 0; y = 0; w = 0; h = 0; }

        /** 
         * Invalidates the pixel viewport.
         */
        void invalidate() { reset(); }

        /** 
         * Apply a fractional viewport to the current pvp.
         * 
         * @param vp the fractional viewport.
         */
        void applyViewport( const Viewport& vp )
            {
                x += (uint32_t)(vp.x * w);
                y += (uint32_t)(vp.y * h);
                w  = (uint32_t)(w*vp.w);
                h  = (uint32_t)(h*vp.h);
            }

        bool isValid() const { return (w>0 && h>0); }

        /**
         * @name Operators
         */
        //*{
        PixelViewport& operator *= ( const Viewport& rhs )
            {
                x += (int32_t)rhs.x * w;
                y += (int32_t)rhs.y * h;
                w  = (int32_t)w*rhs.w;
                h  = (int32_t)h*rhs.h;
                return *this;
            }

        const PixelViewport operator * ( const Viewport& rhs ) const
            {
                return PixelViewport( (int32_t)(x+w*rhs.x),(int32_t)(y+h*rhs.y),
                                      (int32_t)(w*rhs.w),  (int32_t)(h*rhs.h) );
            }

        const Viewport operator / ( const PixelViewport& rhs ) const
            {
                return Viewport(  x / (float)rhs.w,  y / (float)rhs.w,
                                  w / (float)rhs.w,  h / (float)rhs.h );
            }

        bool operator == ( const PixelViewport& rhs ) const 
            { 
                return ( x==rhs.x && x==rhs.x && 
                         y==rhs.y && w==rhs.w && h==rhs.h);
            }
        //*}

        int32_t x;
        int32_t y;
        int32_t w;
        int32_t h;
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const PixelViewport& pvp )
    {
        os << "[ " << pvp.x << " " << pvp.y << " " << pvp.w << " " << pvp.h
           <<" ]";
        return os;
    }
}

#endif // EQ_PIXELVIEWPORT_H
