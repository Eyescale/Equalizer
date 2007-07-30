
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "projection.h"

#include <eq/base/log.h>

using namespace eqBase;
using namespace std;

namespace eq
{
Projection::Projection()
        : origin( 0.f, 0.f, 0.f ),
          distance( 3.f ),
          fov( 54.f, 47.f ),
          hpr( 0.f, 0.f, 0.f )
{}

EQ_EXPORT ostream& operator << ( ostream& os, const Projection& projection )
{
    os << "projection" << endl;
    os << "{" << endl << indent;
    os << "origin   " << projection.origin << endl;
    os << "distance " << projection.distance << endl;
    os << "fov      " << projection.fov << endl;
    os << "hpr      " << projection.hpr << endl;
    os << exdent << "}";
    return os;
}
}
