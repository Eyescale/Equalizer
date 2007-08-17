
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "projection.h"

#include <eq/base/log.h>

using namespace eqBase;
using namespace std;

#define DEG2RAD( angle ) ((angle) * static_cast<float>(M_PI) / 180.f)
#define RAD2DEG( angle ) ((angle) * 180.f / static_cast<float>(M_PI))

namespace eqs
{
Projection::Projection()
        : origin( 0.f, 0.f, 0.f ),
          distance( 3.f ),
          fov( 54.f, 47.f ),
          hpr( 0.f, 0.f, 0.f )
{}

void Projection::resizeHorizontal( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    //const float newWidth_2 = ratio * distance * tanf(DEG2RAD( .5f * fov[1] ));
    //fov[1] = 2.f * atanf( newWidth_2 / distance );
    // -> distance can be removed:
    fov[0] = RAD2DEG( 2.f * atanf( ratio * tanf( DEG2RAD( .5f * fov[0] ))));
}

void Projection::resizeVertical( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    // see resizeHorizontal
    fov[1] = RAD2DEG( 2.f * atanf( ratio * tanf( DEG2RAD( .5f * fov[1] ))));
}

ostream& operator << ( ostream& os, const Projection& projection )
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
