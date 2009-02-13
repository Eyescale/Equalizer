
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "wall.h"
#include "projection.h"
#include <eq/base/log.h>

using namespace eq::base;
using namespace std;

#define DEG2RAD( angle ) ((angle) * static_cast<float>(M_PI) / 180.f)

namespace eq
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

void Wall::apply( const Viewport& viewport)
{
    vmml::Vector3f u = bottomRight - bottomLeft;
    vmml::Vector3f v = topLeft - bottomLeft;
    
    bottomLeft  = bottomLeft + u * viewport.x + v * viewport.y;
    bottomRight = bottomLeft + u * viewport.w;
    topLeft     = bottomLeft + v * viewport.h;  
    
}

Wall& Wall::operator = ( const Projection& projection )
{
       
    const float width  = abs(projection.distance * 2.0 * tanf(DEG2RAD( .5f * projection.fov[0] )));
    const float height = abs(projection.distance * 2.0 * tanf(DEG2RAD( .5f * projection.fov[1] )));
   
    bottomLeft[0]  = -width * 0.5f;
    bottomLeft[1]  = -height * 0.5f;
    bottomLeft[2]  = 0;
    bottomRight[0] = width * 0.5f;
    bottomRight[1] = -height * 0.5f;
    bottomRight[2] = 0;
    topLeft[0]     = -width * 0.5f;
    topLeft[1]     = height * 0.5f;
    topLeft[2]     = 0;

    const float A       = cos(DEG2RAD( projection.hpr[1]));
    const float B       = sin(DEG2RAD( projection.hpr[1]));
    const float C       = cos(DEG2RAD( projection.hpr[0]));
    const float D       = sin(DEG2RAD( projection.hpr[0]));
    const float E       = cos(DEG2RAD( projection.hpr[2]));
    const float F       = sin(DEG2RAD( projection.hpr[2]));

    vmml::Matrix4f  mat ;
    const float AD      =   A * D;
    const float BD      =   B * D;

    mat.ml[0]  =   C * E;
    mat.ml[1]  =  -C * F;
    mat.ml[2]  =  -D;
    mat.ml[4]  = -BD * E + A * F;
    mat.ml[5]  =  BD * F + A * E;
    mat.ml[6]  =  -B * C;
    mat.ml[8]  =  AD * E + B * F;
    mat.ml[9]  = -AD * F + B * E;
    mat.ml[10] =   A * C;

    mat.ml[3]  =  mat.ml[7] = mat.ml[11] = mat.ml[12] = mat.ml[13] = mat.ml[14] = 0;
    mat.ml[15] =  1;

    bottomLeft  = mat * bottomLeft;
    bottomRight = mat * bottomRight;
    topLeft     = mat * topLeft;

    bottomLeft[2]  = bottomLeft[2]  - projection.distance;
    bottomRight[2] = bottomRight[2] - projection.distance;
    topLeft[2]     = topLeft[2]     - projection.distance;

    return *this;
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
    os << exdent << "}" << endl;
    return os;
}

}
