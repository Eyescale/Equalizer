
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "wall.h"

using namespace eq;
using namespace eqBase;
using namespace std;

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

EQ_EXPORT ostream& eq::operator << ( ostream& os, const Wall& wall )
{
    os << "wall" << endl;
    os << "{" << endl << indent;
    os << "bottom_left  [ " << wall.bottomLeft[0] << " " 
       << wall.bottomLeft[1] << " " << wall.bottomLeft[2] << " ]" 
       << endl;
    os << "bottom_right [ " << wall.bottomRight[0] << " " 
       << wall.bottomRight[1] << " " << wall.bottomRight[2] << " ]"
       << endl;
    os << "top_left     [ " << wall.topLeft[0] << " " << wall.topLeft[1]
       << " " << wall.topLeft[2] << " ]" << endl;
    os << exdent << "}";
    return os;
}
