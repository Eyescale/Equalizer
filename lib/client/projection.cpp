
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "projection.h"
#include "wall.h"
#include <eq/base/log.h>

using namespace eq::base;
using namespace std;

#define DEG2RAD( angle ) ((angle) * static_cast<float>(M_PI) / 180.f)
#define RAD2DEG( angle ) ((angle) * 180.f / static_cast<float>(M_PI))

namespace eq
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

Projection& Projection::operator = ( const Wall& wall )
{
    vmml::Vector3f u = wall.bottomRight - wall.bottomLeft;
    vmml::Vector3f v = wall.topLeft - wall.bottomLeft;
    vmml::Vector3f w( u[1]*v[2] - u[2]*v[1],
                     u[2]*v[0] - u[0]*v[2],
                     u[0]*v[1] - u[1]*v[0] );
    float width  = u.normalize();
    float height = v.normalize();

    w.normalize();

    vmml::Vector3f center((wall.bottomRight[0] + wall.topLeft[0]) * 0.5f,
                                (wall.bottomRight[1] + wall.topLeft[1]) * 0.5f,
                                (wall.bottomRight[2] + wall.topLeft[2]) * 0.5f );

    float distTemp = center.length(); 
    if ( distance == 0.f )
    {
        distance = distTemp;
        distTemp = 0;
    }

    vmml::Matrix4f  mat ;
    mat.ml[0]  = u[0];
    mat.ml[1]  = u[1];
    mat.ml[2]  = u[2];
    mat.ml[3]  = 0.f;
             
    mat.ml[4]  = v[0];
    mat.ml[5]  = v[1];
    mat.ml[6]  = v[2];
    mat.ml[7]  = 0.f;
             
    mat.ml[8]  = w[0];
    mat.ml[9]  = w[1];
    mat.ml[10] = w[2];
    mat.ml[11] = 0.f;


    if (distance > 0.0000001f)
    {
        fov[0] = RAD2DEG(atan(0.5f * width / distance ))*2.0;
        fov[1] = RAD2DEG(atan(0.5f * height / distance ))*2.0;
    }

    hpr[0] = -asin(mat.ml[2]);   
    float C  =  cos(hpr[0]);
    hpr[0] =  RAD2DEG(hpr[0]);
    float tr_x;
    float tr_y;

    if (fabs(C) > 0.005)      
    {
        tr_x      =  mat.ml[10] / C;     
        tr_y      = -mat.ml[6]  / C;
        hpr[1]  = RAD2DEG(atan2(tr_y, tr_x));
        tr_x      =  mat.ml[0] / C;          
        tr_y      = -mat.ml[1] / C;
        hpr[2]  = RAD2DEG(atan2(tr_y, tr_x));
    }
    else                                  
    {
        hpr[1]  = 0;         

        tr_x      = mat.ml[5];  
        tr_y      = mat.ml[4];

        hpr[2]  = RAD2DEG(atan2(tr_y, tr_x));
    }
    
    center.normalize();
    origin = center * distance - center * distTemp ;
    
    return *this;
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
