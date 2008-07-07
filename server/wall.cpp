
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "wall.h"

#include <eq/base/log.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{
Wall::Wall()
        : bottomLeft( -.8f, -.5f, -1.f ),
          bottomRight( .8f, -.5f, -1.f ),
          topLeft(    -.8f,  .5f, -1.f )
{
}

void Wall::resizeHorizontal( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const vmml::Vector3f u_2   = (bottomRight - bottomLeft) * .5f;
    const vmml::Vector3f delta = u_2 * (ratio - 1.f);
    bottomLeft  -= delta;
    bottomRight += delta;
    topLeft     -= delta;
}

void Wall::resizeVertical( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const vmml::Vector3f v_2   = (topLeft - bottomLeft) * .5f;
    const vmml::Vector3f delta = v_2 * (ratio - 1.f);
    bottomLeft  -= delta;
    bottomRight -= delta;
    topLeft     += delta;
}

bool Wall::operator == ( const Wall& rhs ) const
{
    return ( bottomLeft  == rhs.bottomLeft  &&
             bottomRight == rhs.bottomRight &&
             topLeft     == rhs.topLeft );
}

bool Wall::operator != ( const Wall& rhs ) const
{
    return ( bottomLeft  != rhs.bottomLeft  ||
             bottomRight != rhs.bottomRight ||
             topLeft     != rhs.topLeft );
}

ostream& operator << ( ostream& os, const Wall& wall )
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
}
