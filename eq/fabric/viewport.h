
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_VIEWPORT_H
#define EQFABRIC_VIEWPORT_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>
#include <eq/fabric/vmmlib.h>
#include <lunchbox/debug.h>
#include <iostream>

namespace eq
{
namespace fabric
{
std::ostream& operator << ( std::ostream& os, const Viewport& vp );

/** A fractional viewport with methods for manipulation. */
class Viewport
{
public:
    /** @name Constructors */
    //@{
    /** Construct a full fractional viewport. @version 1.0 */
    Viewport() : x(0.0f), y(0.0f), w(1.0f), h(1.0f)  {}

    /** Construct a fractional viewport with default values. @version 1.0 */
    Viewport( const float x_, const float y_,
              const float w_, const float h_ )
        : x( x_ ), y( y_ ), w( w_ ), h( h_ )  {}

    /** Construct a fractional viewport from a Vector4f. @version 1.3.0 */
    explicit Viewport( const Vector4f& from )
        : x( from[0] ), y( from[1] ), w( from[2] ), h( from[3] )  {}
    //@}

    /** Make the viewport invalid. @internal */
    void invalidate() { x=0.0f; y=0.0f; w=-1.0f; h=-1.0f; }

    /** Apply (accumulate) another viewport. @internal */
    void apply( const Viewport& rhs )
    {
        LBASSERTINFO( isValid(), *this);
        LBASSERTINFO( rhs.isValid(), rhs );
        x += rhs.x * w;
        y += rhs.y * h;
        w *= rhs.w;
        h *= rhs.h;
    }

    /** Transform this viewport into the rhs viewport space. @internal */
    void transform( const Viewport& rhs )
    {
        w = w / rhs.w;
        h = h / rhs.h;
        x = ( x - rhs.x ) / rhs.w;
        y = ( y - rhs.y ) / rhs.h;
    }

    /**
     * @return true if the two viewports are identical.
     * @version 1.0
     */
    bool operator == ( const Viewport& rhs ) const
    {
        return ( x==rhs.x && y==rhs.y && w==rhs.w && h==rhs.h);
    }

    /**
     * @return true if the two viewports are not identical.
     * @version 1.0
     */
    bool operator != ( const Viewport& rhs ) const
    {
        return ( x!=rhs.x || y!=rhs.y || w!=rhs.w || h!=rhs.h);
    }

    /**
     * @return true if the viewport has a non-negative, but potentially
     *         empty, size.
     * @version 1.0
     */
    bool isValid() const
    { return ( x>=0.0f && y>=0.0f && w>=0.0f && h>=0.0f ); }

    /**
     * @return true if the viewport has a non-zero area, i.e, it is
     *         not empty.
     * @version 1.0
     */
    bool hasArea() const { return (w>0.0f && h>0.0f); }

    /** @return the area of this viewport. @version 1.0 */
    float getArea() const { return w*h; }

    /** @return the X end position. @version 1.0 */
    float getXEnd() const { return x+w; }

    /** @return the Y end position. @version 1.0 */
    float getYEnd() const { return y+h; }

    /** Create the intersection of the two viewports. @version 1.0 */
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

        x = LB_MAX( x, rhs.x );
        y = LB_MAX( y, rhs.y );
        w = LB_MIN( sEx, dEx ) - x;
        h = LB_MIN( sEy, dEy ) - y;
    }

    /** Create the union of the two viewports. @version 1.0 */
    void unite( const Viewport& rhs )
    {
        const float xEnd = LB_MAX( getXEnd(), rhs.getXEnd( ));
        const float yEnd = LB_MAX( getYEnd(), rhs.getYEnd( ));
        x = LB_MIN( x, rhs.x );
        y = LB_MIN( y, rhs.y );
        w = xEnd - x;
        h = yEnd - y;
    }

    /**
     * Compute the coverage of another viewport on this viewport.
     * @internal
     */
    Viewport getCoverage( const Viewport& with ) const
    {
        Viewport coverage( with );
        coverage.intersect( *this ); // intersection
        coverage.transform( *this ); // in our coordinate system

        return coverage;
    }

    /** Apply the view coverage to this viewport. @internal */
    EQFABRIC_API void applyView( const Viewport& segmentVP,
                                 const Viewport& viewVP,
                                 const PixelViewport& pvp,
                                 const Vector4i& overdraw );

    float x; //!< The X coordinate
    float y; //!< The Y coordinate
    float w; //!< The width
    float h; //!< The height

    EQFABRIC_API static const Viewport FULL; //!< A full viewport.
};

inline std::ostream& operator << ( std::ostream& os, const Viewport& vp )
{
    os << "[ " << vp.x << " " << vp.y << " " << vp.w << " " << vp.h << " ]";
    return os;
}
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Viewport& value )
{
    byteswap( value.x );
    byteswap( value.y );
    byteswap( value.w );
    byteswap( value.h );
}
}
#endif // EQFABRIC_VIEWPORT_H
