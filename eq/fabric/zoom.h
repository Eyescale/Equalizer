
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQFABRIC_ZOOM_H
#define EQFABRIC_ZOOM_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>
#include <eq/fabric/vmmlib.h>

namespace eq
{
namespace fabric
{
/**
 * A zoom specification with methods for manipulation.
 *
 * The x, y paramenters determine the factor by which the channel's rendering is
 * zoomed.
 */
class Zoom : public Vector2f
{
public:
    /** Construct a new zoom specification set to 1, 1. @version 1.0 */
    Zoom() : Vector2f( 1.f, 1.f )  {}

    /** Construct a new zoom specification with default values.  @version 1.0 */
    Zoom( const float x_, const float y_ ) : Vector2f( x_, y_ ) {}
    //@}

    /** @internal @return true if this zoom defines a valid zoom factor. */
    bool isValid() const { return ( x() != 0.f && y() != 0.f ); }

    /** @internal Enforce the zoom to be valid. */
    void validate()
    {
        if( x() == 0.f ) x() = 1.f;
        if( y() == 0.f ) y() = 1.f;
    }

    /** @internal Make the zoom factor invalid. */
    void invalidate() { x() = y() = 0.f; }

    /** @internal Apply an additional zoom factor to this zoom. */
    void apply( const Zoom& rhs )
    {
        x() *= rhs.x();
        y() *= rhs.y();
    }

    /** The zoom NONE (1,1) value. */
    EQFABRIC_API static const Zoom NONE;
};

inline std::ostream& operator << ( std::ostream& os, const Zoom& zoom )
{
    if( zoom.isValid( ))
        os << "zoom     [ " << zoom.x() << ' ' << zoom.y() << " ]";
    return os;
}
}
}
namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Zoom& value )
{ byteswap< eq::fabric::Vector2f >( value ); }
}
#endif // EQFABRIC_ZOOM_H
