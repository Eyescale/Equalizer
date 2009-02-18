
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
    const float width  = u.normalize();
    const float height = v.normalize();
    vmml::Vector3f w( u );
    w = w.cross( v );

    const vmml::Vector3f center((wall.bottomRight[0] + wall.topLeft[0]) * 0.5f,
                                (wall.bottomRight[1] + wall.topLeft[1]) * 0.5f,
                                (wall.bottomRight[2] + wall.topLeft[2]) * 0.5f);
    
    if ( distance <= std::numeric_limits< float >::epsilon( ))
        distance = center.length(); 

    vmml::Matrix3f  mat;
    mat.ml[0] = u[0];
    mat.ml[1] = u[1];
    mat.ml[2] = u[2];
             
    mat.ml[3] = v[0];
    mat.ml[4] = v[1];
    mat.ml[5] = v[2];
             
    mat.ml[6] = w[0];
    mat.ml[7] = w[1];
    mat.ml[8] = w[2];

    fov[0] = RAD2DEG( atanf(0.5f * width / distance  )) * 2.0f;
    fov[1] = RAD2DEG( atanf(0.5f * height / distance )) * 2.0f;

    hpr[0] = -asinf( mat.ml[2] );
    const float cosH = cosf(hpr[0]);
    hpr[0] =  RAD2DEG(hpr[0]);

    if( fabs( cosH ) > std::numeric_limits< float >::epsilon( ))      
    {
        float tr_x      =  mat.ml[8] / cosH;     
        float tr_y      = -mat.ml[5] / cosH;
        hpr[1]  = RAD2DEG( atan2f( tr_y, tr_x ));

        tr_x      =  mat.ml[0] / cosH;          
        tr_y      = -mat.ml[1] / cosH;
        hpr[2]  = RAD2DEG( atan2f( tr_y, tr_x ));
    }
    else                                  
    {
        hpr[1]  = 0.f;         

        const float tr_x = mat.ml[4];  
        const float tr_y = mat.ml[3];

        hpr[2]  = RAD2DEG( atan2f( tr_y, tr_x ));
    }
    
    origin = center - w * distance;
    return *this;
}

bool Projection::operator == ( const Projection& rhs ) const
{
    return ( origin   == rhs.origin   &&
             distance == rhs.distance &&
             fov      == rhs.fov      &&
             hpr      == rhs.hpr );
}

bool Projection::operator != ( const Projection& rhs ) const
{
    return ( origin   != rhs.origin   ||
             distance != rhs.distance ||
             fov      != rhs.fov      ||
             hpr      != rhs.hpr );
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
