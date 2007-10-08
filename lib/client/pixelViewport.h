
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELVIEWPORT_H
#define EQ_PIXELVIEWPORT_H

#include "viewport.h"

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <vmmlib/vector2.h>

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
         * Invalidates the pixel viewport.
         */
        void invalidate() { x = 0; y = 0; w = -1; h = -1; }

        /** 
         * Apply a fractional viewport to the current pvp.
         * 
         * @param vp the fractional viewport.
         */
        void applyViewport( const Viewport& vp ) { (*this) *= vp; }
        
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

        /** @return the area in pixels. */
        uint32_t getArea() const { return w * h; }

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

        const Viewport operator / ( const PixelViewport& rhs ) const
            {
                EQASSERT( rhs.hasArea( ));
                return Viewport(  ( x - rhs.x )/ static_cast<float>( rhs.w ),
                                  ( y - rhs.y )/ static_cast<float>( rhs.h ),
                                  ( w )/ static_cast<float>( rhs.w ),
                                  ( h )/ static_cast<float>( rhs.h ));
            }

        const PixelViewport operator + ( const vmml::Vector2i& offset ) const
            {
                return PixelViewport( x+offset.x, y+offset.y, w, h );
            }

        bool operator == ( const PixelViewport& rhs ) const 
            { 
                return ( x==rhs.x && y==rhs.y && w==rhs.w && h==rhs.h );
            }
        bool operator != ( const PixelViewport& rhs ) const 
            { 
                return ( x!=rhs.x || y!=rhs.y || w!=rhs.w || h!=rhs.h );
            }

        /** create a pixel viewport that includes both viewports (union) */
        const PixelViewport& operator += ( const PixelViewport& rhs )
            {
                if( *this == rhs || !rhs.hasArea() )
                    return *this;

                if( !hasArea() )
                {
                    *this = rhs;
                    return rhs;
                }

                int32_t sEx =     x +     w;
                int32_t sEy =     y +     h;
                int32_t dEx = rhs.x + rhs.w;
                int32_t dEy = rhs.y + rhs.h;
                
                x = MIN( x, rhs.x );
                y = MIN( y, rhs.y );
                w = MAX( sEx, dEx ) - x;
                h = MAX( sEy, dEy ) - y;

                return *this;
            }

        /** create the intersection pixel viewport  */
        PixelViewport operator ^= ( const PixelViewport& rhs )
            {
                if( *this == rhs )
                    return *this;
                if( !rhs.hasArea() || !hasArea() )
                    return PixelViewport();
                
                int32_t sEx =     x +     w;
                int32_t sEy =     y +     h;
                int32_t dEx = rhs.x + rhs.w;
                int32_t dEy = rhs.y + rhs.h;
                    
                x = MAX( x, rhs.x );
                y = MAX( y, rhs.y );
                w = MIN( sEx, dEx ) - x;
                h = MIN( sEy, dEy ) - y;
                    
                return *this;
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
