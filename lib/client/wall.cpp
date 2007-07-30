
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "wall.h"

#include <eq/base/log.h>

using namespace eqBase;
using namespace std;

namespace eq
{
Wall::Wall()
        : bottomLeft( -.8f, -.5f, -1.f ),
          bottomRight( .8f, -.5f, -1.f ),
          topLeft(    -.8f,  .5f, -1.f )
{
}

EQ_EXPORT ostream& operator << ( ostream& os, const Wall& wall )
{
    os << "wall" << endl;
    os << "{" << endl << indent;
    os << "bottom_left  " << wall.bottomLeft << endl;
    os << "bottom_right " << wall.bottomRight << endl;
    os << "top_left     " << wall.topLeft << endl;
    os << exdent << "}";
    return os;
}
}
