
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef MESH_SPLITAXIS_H
#define MESH_SPLITAXIS_H

namespace mesh
{
    /** The split axis for a kd-tree node. */
    enum SplitAxis
    {
        AXIS_X,
        AXIS_Y,
        AXIS_Z,
        AXIS_ALL
    };
}

#endif // MESH_SPLITAXIS_H
