
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

    const float cosP       = cos(DEG2RAD( projection.hpr[1]));
    const float sinP       = sin(DEG2RAD( projection.hpr[1]));
    const float cosH       = cos(DEG2RAD( projection.hpr[0]));
    const float sinH       = sin(DEG2RAD( projection.hpr[0]));
    const float cosR       = cos(DEG2RAD( projection.hpr[2]));
    const float sinR       = sin(DEG2RAD( projection.hpr[2]));

    vmml::Matrix4f  mat ;
    const float cosPsinH      =   cosP * sinH;
    const float sinPsinH      =   sinP * sinH;

    mat.ml[0]  =   cosH * cosR;
    mat.ml[1]  =  -cosH * sinR;
    mat.ml[2]  =  -sinH;
    mat.ml[4]  = -sinPsinH * cosR + cosP * sinR;
    mat.ml[5]  =  sinPsinH * sinR + cosP * cosR;
    mat.ml[6]  =  -sinP * cosH;
    mat.ml[8]  =  cosPsinH * cosR + sinP * sinR;
    mat.ml[9]  = -cosPsinH * sinR + sinP * cosR;
    mat.ml[10] =   cosP * cosH;

    mat.ml[3]  =  mat.ml[7] = mat.ml[11] = mat.ml[12] = mat.ml[13] = mat.ml[14] = 0;
    mat.ml[15] =  1;

    bottomLeft  = mat * bottomLeft;
    bottomRight = mat * bottomRight;
    topLeft     = mat * topLeft;
    
    vmml::Vector3f distance(projection.origin);
    distance.normalize();
    distance = distance * projection.origin.length() + distance * projection.distance;
    bottomLeft  = bottomLeft  - distance;
    bottomRight = bottomRight - distance;
    topLeft     = topLeft      - distance;

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
