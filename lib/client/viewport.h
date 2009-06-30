
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

#ifndef EQ_VIEWPORT_H
#define EQ_VIEWPORT_H

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/client/types.h>

#include <vector>         // WAR: vector4.h does not include these headers. Fix 
#include <limits> //      vector4.h after vmmlib upgrade and remove them

#include <iostream>

namespace eq
{
    class PixelViewport;
    class Viewport;
    std::ostream& operator << ( std::ostream& os, const Viewport& vp );

    /**
     * Holds a fractional viewport along with some methods for manipulation.
     */
    class Viewport 
    {
    public:
        /**
         * @name Constructors
         */
        //@{
        Viewport() : x(0.0f), y(0.0f), w(1.0f), h(1.0f)  {}

        Viewport( const float x_, const float y_, const float w_,const float h_)
                : x(x_), y(y_), w(w_), h(h_)  {}
        //@}

        void invalidate() { x=0.0f; y=0.0f; w=-1.0f; h=-1.0f; }
        void apply ( const Viewport& rhs )
            {
                EQASSERTINFO( isValid(), *this);
                EQASSERTINFO( rhs.isValid(), rhs );                
                x += rhs.x * w;
                y += rhs.y * h;
                w *= rhs.w;
                h *= rhs.h;
            }
            
        void transform ( const Viewport& rhs )
            {
                w = w / rhs.w;
                h = h / rhs.h;
                x = ( x - rhs.x ) / rhs.w;
                y = ( y - rhs.y ) / rhs.h;
            }

        bool operator == ( const Viewport& rhs ) const 
            { 
                return ( x==rhs.x && y==rhs.y && w==rhs.w && h==rhs.h);
            }

        bool operator != ( const Viewport& rhs ) const 
            { 
                return ( x!=rhs.x || x!=rhs.y || w!=rhs.w || h!=rhs.h);
            }

        /** 
         * @return true if the viewport has a non-negative, but potentially
         *         empty, size.
         */
        bool isValid() const 
            { return ( x>=0.0f && y>=0.0f && w>=0.0f && h>=0.0f ); }
        
        /** 
         * @return true if the viewport has a non-zero area, i.e, it is
         *         not empty.
         */
        bool hasArea() const { return (w>0.0f && h>0.0f); }

        /** @return the area of this viewport */
        float getArea() const { return w*h; }

        /** @return the X end position */
        float getXEnd() const { return x+w; }

        /** @return the Y end position */
        float getYEnd() const { return y+h; }

        /** create the intersection viewport  */
        void intersect( const Viewport& rhs )
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
                
                const float sEx = static_cast< float >(     x +     w );
                const float sEy = static_cast< float >(     y +     h );
                const float dEx = static_cast< float >( rhs.x + rhs.w );
                const float dEy = static_cast< float >( rhs.y + rhs.h );
                    
                x = EQ_MAX( x, rhs.x );
                y = EQ_MAX( y, rhs.y );
                w = EQ_MIN( sEx, dEx ) - x;
                h = EQ_MIN( sEy, dEy ) - y;
            }

        /** Compute the coverage of another Viewport on this viewport. */
        Viewport getCoverage( const Viewport& with ) const
            {
                Viewport coverage( with );
                coverage.intersect( *this ); // intersection
                coverage.transform( *this ); // in our coordinate system

                return coverage;
            }

        /** Apply the view coverage to this viewport. */
        EQ_EXPORT void applyView( const Viewport& segmentVP, 
                                  const Viewport& viewVP,
                                  const PixelViewport& pvp, 
                                  const Vector4i& overdraw );

        float x;
        float y;
        float w;
        float h;

        EQ_EXPORT static const Viewport FULL;
    };

    inline std::ostream& operator << ( std::ostream& os, const Viewport& vp )
    {
        os << "[ " << vp.x << " " << vp.y << " " << vp.w << " " << vp.h << " ]";
        return os;
    }
}

#endif // EQ_VIEWPORT_H
