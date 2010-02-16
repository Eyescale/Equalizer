
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PIXELVIEWPORT_H
#define EQ_PIXELVIEWPORT_H

#include <eq/client/viewport.h> // used in inline method
#include <eq/client/pixel.h>    // used in inline method
#include <eq/client/zoom.h>     // used in inline method
#include <eq/client/types.h>

#include <eq/base/base.h>
#include <eq/base/debug.h>


#include <limits>

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
        //@{
        PixelViewport() : x(0), y(0), w(-1), h(-1)  {}

        PixelViewport( const int32_t x_, const int32_t y_, 
                       const int32_t w_, const int32_t h_ )
                : x(x_), y(y_), w(w_), h(h_)  {}
        //@}

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
        //@{
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
                if( pixel.w > 1 )
                {
                    int32_t newWidth = w / pixel.w;
                    // This would be the correct thing to do, but it would
                    // require frustum adaptations in CUV::_computeFrustum:
                    //if( w - ( newWidth * pixel.w ) > pixel.x )
                    if( w - ( newWidth * pixel.w ) != 0 )
                        ++newWidth;

                    w = newWidth;
                }
                if( pixel.h > 1 )
                {
                    int32_t newHeight = h / pixel.h;
                    // This would be the correct thing to do, but it would
                    // require frustum adaptations in CUV::_computeFrustum:
                    // if( w - ( newWidth * pixel.h ) > pixel.y )
                    if( h - ( newHeight * pixel.h ) != 0 )
                        ++newHeight;

                    h = newHeight;
                }
        }

        void apply( const Zoom& zoom )
            {
                if( zoom == Zoom::NONE )
                    return;

                w = static_cast< int32_t >( w * zoom.x() + .5f );
                h = static_cast< int32_t >( h * zoom.y() + .5f );
            }

        const PixelViewport getSubPVP( const Viewport& rhs ) const
            {
                if( rhs == Viewport::FULL )
                    return *this;

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
                if( *this == rhs )
                    return Viewport::FULL;

                if( !rhs.hasArea( ))
                    return Viewport( static_cast<float>( x ), 
                                     static_cast<float>( y ), 0.f, 0.f );

                return Viewport(  ( x - rhs.x )/ static_cast<float>( rhs.w ),
                                  ( y - rhs.y )/ static_cast<float>( rhs.h ),
                                  ( w )/ static_cast<float>( rhs.w ),
                                  ( h )/ static_cast<float>( rhs.h ));
            }

        const Zoom getZoom( const PixelViewport& rhs ) const
            {
                if( *this == rhs )
                    return Zoom::NONE;

                if( !rhs.hasArea( ))
                    return Zoom( std::numeric_limits< float >::max(), 
                                 std::numeric_limits< float >::max( ));

                return Zoom( w / static_cast<float>( rhs.w ),
                             h / static_cast<float>( rhs.h ));
            }

        /** @return the X end position */
        int32_t getXEnd() const { return x+w; }

        /** @return the Y end position */
        int32_t getYEnd() const { return y+h; }

        const PixelViewport operator + ( const Vector2i& offset ) const
            {
                return PixelViewport( x+offset.x(), y+offset.y(), w, h );
            }

        const PixelViewport operator * ( const eq::Pixel& pixel ) const
            {
                return PixelViewport( x, y, w * pixel.w, h * pixel.h );
            }
        PixelViewport& operator *= ( const eq::Pixel& pixel )
            {
                w *= pixel.w;
                h *= pixel.h;
                x += pixel.x;
                y += pixel.y;
                return *this;
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

                const int32_t sEx =     x +     w;
                const int32_t sEy =     y +     h;
                const int32_t dEx = rhs.x + rhs.w;
                const int32_t dEy = rhs.y + rhs.h;
                
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
                
                const int32_t sEx =     x +     w;
                const int32_t sEy =     y +     h;
                const int32_t dEx = rhs.x + rhs.w;
                const int32_t dEy = rhs.y + rhs.h;
                    
                x = EQ_MAX( x, rhs.x );
                y = EQ_MAX( y, rhs.y );
                w = EQ_MIN( sEx, dEx ) - x;
                h = EQ_MIN( sEy, dEy ) - y;
            }

        //@}

        /**
         * @name Tests
         */
        //@{
        /** @returns true if the point (pX,pY) is inside, false if not. */
        bool isPointInside( const int32_t pX, const int32_t pY ) const
            {
                if( pX < x || pY < y || pX > (x+w) || pY > (y+h) )
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
