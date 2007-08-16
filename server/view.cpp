
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

#include "projection.h"
#include "wall.h"

#define DEG2RAD( angle ) ( (angle) * static_cast<float>(M_PI) / 180.f )

namespace eqs
{
void View::applyWall( const Wall& wall )
{
    vmml::Vector3f u = wall.bottomRight - wall.bottomLeft;
    vmml::Vector3f v = wall.topLeft - wall.bottomLeft;
    vmml::Vector3f w( u[1]*v[2] - u[2]*v[1],
                      u[2]*v[0] - u[0]*v[2],
                      u[0]*v[1] - u[1]*v[0] );

    width = u.normalize();
    height = v.normalize();
    w.normalize();

    xfm.ml[0]  = u[0];
    xfm.ml[1]  = v[0];
    xfm.ml[2]  = w[0];
    xfm.ml[3]  = 0.;
             
    xfm.ml[4]  = u[1];
    xfm.ml[5]  = v[1];
    xfm.ml[6]  = w[1];
    xfm.ml[7]  = 0.;
             
    xfm.ml[8]  = u[2];
    xfm.ml[9]  = v[2];
    xfm.ml[10] = w[2];
    xfm.ml[11] = 0.;

    const vmml::Vector3f center((wall.bottomRight[0] + wall.topLeft[0]) * 0.5f,
                                (wall.bottomRight[1] + wall.topLeft[1]) * 0.5f,
                                (wall.bottomRight[2] + wall.topLeft[2]) * 0.5f);

    xfm.ml[12] = -(u[0]*center[0] + u[1]*center[1] + u[2]*center[2]);
    xfm.ml[13] = -(v[0]*center[0] + v[1]*center[1] + v[2]*center[2]);
    xfm.ml[14] = -(w[0]*center[0] + w[1]*center[1] + w[2]*center[2]);
    xfm.ml[15] = 1.;
}

void View::applyProjection( const Projection& projection )
{
    const float cosH = cosf( DEG2RAD( projection.hpr[0] ));
    const float sinH = sinf( DEG2RAD( projection.hpr[0] ));
    const float cosP = cosf( DEG2RAD( projection.hpr[1] ));
    const float sinP = sinf( DEG2RAD( projection.hpr[1] ));
    const float cosR = cosf( DEG2RAD( projection.hpr[2] ));
    const float sinR = sinf( DEG2RAD( projection.hpr[2] ));

    // HPR Matrix = -roll[z-axis] * -pitch[x-axis] * -head[y-axis]
    const float rot[9] =
        {
            sinR*sinP*sinH + cosR*cosH,  cosR*sinP*sinH - sinR*cosH,  cosP*sinH,
            cosP*sinR,                   cosP*cosR,                  -sinP,
            sinR*sinP*cosH - cosR*sinH,  cosR*sinP*cosH + sinR*sinH,  cosP*cosH 
        };

    // translation = HPR x -origin
    const vmml::Vector3f& origin = projection.origin;
    const float  distance = projection.distance;
    const vmml::Vector3f 
        trans( -( rot[0]*origin[0] + rot[3]*origin[1] + rot[6]*origin[2] ),
               -( rot[1]*origin[0] + rot[4]*origin[1] + rot[7]*origin[2] ),
               -( rot[2]*origin[0] + rot[5]*origin[1] + rot[8]*origin[2] ));

    xfm.ml[0]  = rot[0];
    xfm.ml[1]  = rot[1];
    xfm.ml[2]  = rot[2];
    xfm.ml[3]  = 0.;

    xfm.ml[4]  = rot[3];
    xfm.ml[5]  = rot[4];
    xfm.ml[6]  = rot[5];
    xfm.ml[7]  = 0.;
            
    xfm.ml[8]  = rot[6];                
    xfm.ml[9]  = rot[7];
    xfm.ml[10] = rot[8];
    xfm.ml[11] = 0.;

    xfm.ml[12] = trans[0];
    xfm.ml[13] = trans[1];
    xfm.ml[14] = trans[2] + distance;
    xfm.ml[15] = 1.;

    width  = distance * tan(DEG2RAD( projection.fov[0] ));
    height = distance * tan(DEG2RAD( projection.fov[1] ));
}
 
std::ostream& operator << ( std::ostream& os, const View& view )
{
    os << "WxH: " << view.width << "x" << view.height << " xfm: " << view.xfm
       << std::endl;
    return os;
}

}
