
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PROJECTION_H
#define EQ_PROJECTION_H

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
    struct Projection
    {
        float origin[3];
        float distance;
        float fov[2];
        float hpr[3];
    };
}

#endif // EQ_PROJECTION_H

