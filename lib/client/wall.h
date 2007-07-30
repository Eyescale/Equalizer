
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WALL_H
#define EQ_WALL_H

#include <eq/base/base.h>
#include <eq/vmmlib/vmmlib.h>

#include <iostream>

namespace eq
{
    /**
     * A wall defining a view frustum.
     * 
     * The three points describe the bottom left, bottom right and top left
     * coordinate of the wall in real-world coordinates.
     */
    class EQ_EXPORT Wall
    {
    public:
        Wall();

        vmml::Vector3f bottomLeft;
        vmml::Vector3f bottomRight;
        vmml::Vector3f topLeft;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Wall& wall );
}

#endif // EQ_WALL_H

