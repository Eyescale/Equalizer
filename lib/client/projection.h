
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

#ifndef EQ_PROJECTION_H
#define EQ_PROJECTION_H

#include <eq/base/base.h>
#include <eq/client/types.h>
  
#include <iostream>

namespace eq
{
    class Wall;
    /**
     * A projection definition defining a view frustum.
     * 
     * The frustum is defined by a projection system positioned at origin,
     * orientated as defined by the head-pitch-roll angles projecting to a
     * wall at the given distance. The fov defines the horizontal and
     * vertical field of view of the projector.
     */
    class Projection
    {
    public:
        EQ_EXPORT Projection();

        /** 
         * Resize the horizontal FOV.
         * 
         * @param ratio the amount by which the FOV is grown or shrunk.
         */
        EQ_EXPORT void resizeHorizontal( const float ratio );

        /** 
         * Resize the vertical FOV.
         * 
         * @param ratio the amount by which the FOV is grown or shrunk.
         */
        EQ_EXPORT void resizeVertical( const float ratio );

        /** Set the projection parameters from a wall description. */
        EQ_EXPORT Projection& operator = ( const Wall& wall );

        EQ_EXPORT bool operator == ( const Projection& rhs ) const;
        EQ_EXPORT bool operator != ( const Projection& rhs ) const;
        
        Vector3f origin;
        float    distance;
        Vector2f fov;
        Vector3f hpr;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Projection& );
}
#endif // EQ_PROJECTION_H

