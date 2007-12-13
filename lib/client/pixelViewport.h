
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELVIEWPORT_H
#define EQ_PIXELVIEWPORT_H

#include <eq/client/viewport.h> // used in inline method
#include <eq/client/pixel.h>    // used in inline method

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

        PixelViewport( const int32_t x_, const int32_t y_, 
                       const int32_t w_, const int32_t h_ )
                : x(x_), y(y_), w(w_), h(h_)  {}
        //*}

        /** 
         * Invalidates the pixel viewport.
         */
        void invalidate() { x = 0; y = 0; w = -1; h = -1; }

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
        void apply( const Viewport& rhs )
            {
                // honor position over size to avoid rounding artifacts
                const int32_t xEnd = x + static_cast<int32_t>((rhs.x+rhs.w)*w);
                const int32_t yEnd = y + static_cast<int32_t>((rhs.y+rhs.h)*h);

                x += static_cast<int32_t>( w * rhs.x );
                y += static_cast<int32_t>( h * rhs.y );
                w = xEnd - x;
                h = yEnd - y;
            }

        void apply( const Pixel& pixel )
            {
                if( pixel == eq::Pixel::ALL )
                    return;

                int32_t newWidth = w / pixel.size;
                // This would be the correct thing to do, but it would require
                // frustum adaptations in CUV::_computeFrustum:
                //if( w - ( newWidth * pixel.size ) > pixel.index )
                if( w - ( newWidth * pixel.size ) != 0 )
                    ++newWidth;

                w = newWidth;
            }

        const PixelViewport getSubPVP( const Viewport& rhs ) const
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

        const Viewport getSubVP( const PixelViewport& rhs ) const
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
        void merge( const PixelViewport& rhs )
            {
                if( *this == rhs || !rhs.hasArea() )
                    return;

                if( !hasArea() )
                {
                    *this = rhs;
                    return;
                }

                int32_t sEx =     x +     w;
                int32_t sEy =     y +     h;
                int32_t dEx = rhs.x + rhs.w;
                int32_t dEy = rhs.y + rhs.h;
                
                x = EQ_MIN( x, rhs.x );
                y = EQ_MIN( y, rhs.y );
                w = EQ_MAX( sEx, dEx ) - x;
                h = EQ_MAX( sEy, dEy ) - y;
            }

        /** create the intersection pixel viewport  */
        void intersect( const PixelViewport& rhs )
            {
                if( *this == rhs )
                    return;

                if( !rhs.isValid() || !isValid() )
                {
                    invalidate();
                    return;
                }
                
                if( !rhs.hasArea() || !hasArea() )
                {
                    x = 0;
                    y = 0;
                    w = 0;
                    h = 0;
                    return;
                }
                
                int32_t sEx =     x +     w;
                int32_t sEy =     y +     h;
                int32_t dEx = rhs.x + rhs.w;
                int32_t dEy = rhs.y + rhs.h;
                    
                x = EQ_MAX( x, rhs.x );
                y = EQ_MAX( y, rhs.y );
                w = EQ_MIN( sEx, dEx ) - x;
                h = EQ_MIN( sEy, dEy ) - y;
            }

        //*}

        /**
         * @name Tests
         */
        //*{
        /** @returns true if the point (pX,pY) is inside, false if not. */
        bool isPointInside( const int32_t pX, const int32_t pY ) const
            {
                if( pX < x || pX < y || pX > (x+w) || pY > (y+h) )
                    return false;
                return true;
            }

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
