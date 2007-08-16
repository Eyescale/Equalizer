
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_PROJECTION_H
#define EQS_PROJECTION_H

#include <eq/base/base.h>
#include <eq/vmmlib/vmmlib.h>
 
#include <iostream>

namespace eqs
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

        vmml::Vector3f origin;
        float          distance;
        vmml::Vector2f fov;
        vmml::Vector3f hpr;
    };

    std::ostream& operator << ( std::ostream& os, const Projection& );
}

#endif // EQS_PROJECTION_H

