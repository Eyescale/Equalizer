
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_PROJECTION_H
#define EQFABRIC_PROJECTION_H

#include <eq/fabric/types.h>
#include <eq/base/base.h>
  
#include <iostream>

namespace eq
{
namespace fabric
{
    class Wall;

    /**
     * A projector definition defining a view frustum.
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
         * @version 1.0
         */
        EQ_EXPORT void resizeHorizontal( const float ratio );

        /** 
         * Resize the vertical FOV.
         * 
         * @param ratio the amount by which the FOV is grown or shrunk.
         * @version 1.0
         */
        EQ_EXPORT void resizeVertical( const float ratio );

        /**
         * Convert the projection parameters from a wall description.
         * @version 1.0
         */
        EQ_EXPORT Projection& operator = ( const Wall& wall );

        /**
         * @return true if the two projection definitions are identical.
         * @version 1.0
         */
        EQ_EXPORT bool operator == ( const Projection& rhs ) const;

        /**
         * @return true if the two projection definitions are not identical.
         * @version 1.0
         */
        EQ_EXPORT bool operator != ( const Projection& rhs ) const;
        
        Vector3f origin;
        float    distance;
        Vector2f fov;
        Vector3f hpr;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Projection& );
}
}
#endif // EQFABRIC_PROJECTION_H

