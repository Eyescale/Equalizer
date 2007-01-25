
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELVIEWPORT_H
#define EQ_PIXELVIEWPORT_H

#include "viewport.h"

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/vmmlib/Vector2.h>

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
        PixelViewport() : x(0), y(0), w(-1), h(-1)  {}

        PixelViewport( const int32_t x, const int32_t y, 
                       const int32_t w, const int32_t h )
                : x(x), y(y), w(w), h(h)  {}
        //*}

        /** 
         * Reset the pixel viewport to be empty.
         */
        void reset() { x = 0; y = 0; w = -1; h = -1; }

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
        
        /** 
         * @return true if the pixel viewport has a non-negative, but
         *         potentially empty, size.
         */
        bool isValid() const { return (w>=0 && h>=0); }
        
        /** 
         * @return true if the pixel viewport has a non-zero area, i.e, it is
         *         not empty.
         */
        bool hasArea() const { return (w>0 && h>0); }

        /**
         * @name Operators
         */
        //*{
        PixelViewport& operator *= ( const Viewport& rhs )
            {
                // honor position over size to avoid rounding artifacts
                const int32_t xEnd = x + static_cast<int32_t>((rhs.x+rhs.w)*w);
                const int32_t yEnd = y + static_cast<int32_t>((rhs.y+rhs.h)*h);

                x += static_cast<int32_t>( w * rhs.x );
                y += static_cast<int32_t>( h * rhs.y );
                w = xEnd - x;
                h = yEnd - y;

                return *this;
            }

        const PixelViewport operator * ( const Viewport& rhs ) const
            {
                PixelViewport result;

                // honor position over size to avoid rounding artifacts
                const int32_t xEnd = x + static_cast<int32_t>((rhs.x+rhs.w)*w);
                const int32_t yEnd = y + static_cast<int32_t>((rhs.y+rhs.h)*h);

                result.x = x + static_cast<int32_t>( w * rhs.x );
                result.y = y + static_cast<int32_t>( h * rhs.y );
                result.w = xEnd - result.x;
                result.h = yEnd - result.y;

                return result;
            }

        const PixelViewport operator + ( const vmml::Vector2i& offset ) const
            {
                return PixelViewport( x+offset.x, y+offset.y, w, h );
            }

        const Viewport operator / ( const PixelViewport& rhs ) const
            {
                EQASSERT( rhs.hasArea( ));
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
