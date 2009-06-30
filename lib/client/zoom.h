
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_ZOOM_H
#define EQ_ZOOM_H

#include <eq/base/base.h>
#include <eq/base/log.h>
#include <eq/client/types.h>  // base class

namespace eq
{
    /**
     * Holds a zoom specification along with some methods for manipulation.
     *
     * The x, y paramenters determine the factor by which the channel's
     * rendering is zoomed.
     */
    class Zoom : public Vector2f
    {
    public:
        /**
         * @name Constructors
         */
        //@{
        Zoom() : Vector2f( 1.f, 1.f )  {}
        Zoom( const float x_, const float y_ ) : Vector2f( x_, y_ ) {}
        //@}

        /** @return true if this zoom defines a valid zoom factor. */
        bool isValid() const { return ( x() != 0.f && y() != 0.f ); }

        /** Enforce the zoom to be valid. */
        void validate()
            {
                if( x() == 0.f ) x() = 1.f;
                if( y() == 0.f ) y() = 1.f;
            }
            

        
        /** Make the zoom factor invalid. */
        void invalidate() { x() = y() = 0.f; }

        EQ_EXPORT static const Zoom NONE;
    };

    inline std::ostream& operator << ( std::ostream& os, const Zoom& zoom )
    {
        if( zoom.isValid( ))
            os << "zoom     [ " << zoom.x() << ' ' << zoom.y() << " ]";
        return os;
    }
}

#endif // EQ_ZOOM_H
