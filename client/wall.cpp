
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "wall.h"

using namespace eq;

void Wall::translate( float x, float y, float z )
{
    bottomLeft[0]  += x;
    bottomRight[0] += x;
    topLeft[0]     += x;
    bottomLeft[1]  += y;
    bottomRight[1] += y;
    topLeft[1]     += y;
    bottomLeft[2]  += z;
    bottomRight[2] += z;
    topLeft[2]     += z;
}
