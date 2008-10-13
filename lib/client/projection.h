
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PROJECTION_H
#define EQ_PROJECTION_H

#include <eq/base/base.h>
#include <vmmlib/vmmlib.h> // member
 
#include <iostream>

namespace eq
{
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
        Projection();

        /** 
         * Resize the horizontal FOV.
         * 
         * @param ratio the amount by which the FOV is grown or shrunk.
         */
        void resizeHorizontal( const float ratio );

        /** 
         * Resize the vertical FOV.
         * 
         * @param ratio the amount by which the FOV is grown or shrunk.
         */
        void resizeVertical( const float ratio );

        vmml::Vector3f origin;
        float          distance;
        vmml::Vector2f fov;
        vmml::Vector3f hpr;
    };

    std::ostream& operator << ( std::ostream& os, const Projection& );
}
#endif // EQ_PROJECTION_H

