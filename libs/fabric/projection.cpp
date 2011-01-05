
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "projection.h"
#include "wall.h"

#include <co/base/log.h>

#define DEG2RAD( angle ) ((angle) * static_cast<float>(M_PI) / 180.f)
#define RAD2DEG( angle ) ((angle) * 180.f / static_cast<float>(M_PI))

namespace eq
{
namespace fabric
{
Projection::Projection()
        : origin( 0.f, 0.f, 0.f ),
          distance( 1.f ),
          fov( 77.3196f, 53.1301f ),
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
    Vector3f u = wall.bottomRight - wall.bottomLeft;
    Vector3f v = wall.topLeft - wall.bottomLeft;
    const float width  = u.normalize();
    const float height = v.normalize();

    Vector3f w;
    w.cross( u, v );

    const Vector3f center( (wall.bottomRight[0] + wall.topLeft[0]) * 0.5f,
                           (wall.bottomRight[1] + wall.topLeft[1]) * 0.5f,
                           (wall.bottomRight[2] + wall.topLeft[2]) * 0.5f );
    
    if ( distance <= std::numeric_limits< float >::epsilon( ))
        distance = center.length(); 

    Matrix3f  mat;
    mat.array[0] = u[0];
    mat.array[1] = u[1];
    mat.array[2] = u[2];
             
    mat.array[3] = v[0];
    mat.array[4] = v[1];
    mat.array[5] = v[2];
             
    mat.array[6] = w[0];
    mat.array[7] = w[1];
    mat.array[8] = w[2];

    fov[0] = RAD2DEG( atanf(0.5f * width / distance  )) * 2.0f;
    fov[1] = RAD2DEG( atanf(0.5f * height / distance )) * 2.0f;

    hpr[0] = -asinf( mat.array[2] );
    const float cosH = cosf(hpr[0]);
    hpr[0] =  RAD2DEG(hpr[0]);

    if( fabs( cosH ) > std::numeric_limits< float >::epsilon( ))      
    {
        float tr_x      =  mat.array[8] / cosH;     
        float tr_y      = -mat.array[5] / cosH;
        hpr[1]  = RAD2DEG( atan2f( tr_y, tr_x ));

        tr_x      =  mat.array[0] / cosH;          
        tr_y      = -mat.array[1] / cosH;
        hpr[2]  = RAD2DEG( atan2f( tr_y, tr_x ));
    }
    else                                  
    {
        hpr[1]  = 0.f;         

        const float tr_x = mat.array[4];  
        const float tr_y = mat.array[3];

        hpr[2]  = RAD2DEG( atan2f( tr_y, tr_x ));
    }
    
    origin = center - w * distance;
    return *this;
}

bool Projection::operator == ( const Projection& rhs ) const
{
    return(  origin.equals( rhs.origin, 0.0001f )   &&
           (fabsf( distance - rhs.distance ) < 0.0001f ) &&
           fov.equals( rhs.fov, 0.0001f ) &&
           hpr.equals( rhs.hpr, 0.0001f ));
}

bool Projection::operator != ( const Projection& rhs ) const
{
    return ( !origin.equals( rhs.origin, 0.0001f )  ||
            (fabsf(distance - rhs.distance) >= 0.0001f ) ||
            !fov.equals( rhs.fov, 0.0001f ) ||
            !hpr.equals( rhs.hpr, 0.0001f ));
}


std::ostream& operator << ( std::ostream& os, const Projection& projection )
{
    os << "projection" << std::endl;
    os << "{" << std::endl << co::base::indent;
    os << "origin   " << projection.origin << std::endl;
    os << "distance " << projection.distance << std::endl;
    os << "fov      " << projection.fov << std::endl;
    os << "hpr      " << projection.hpr << std::endl;
    os << co::base::exdent << "}";
    return os;
}

}
}
