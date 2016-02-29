
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "wall.h"
#include "projection.h"
#include "pixelViewport.h"

#include <lunchbox/log.h>

#ifndef M_PI
#  define M_PI 3.14159265358979323846264338327
#endif
#define DEG2RAD( angle ) ((angle) * static_cast<float>(M_PI) / 180.f)

namespace eq
{
namespace fabric
{

Wall::Wall()
    : bottomLeft( -.8f, -.5f, -1.f )
    , bottomRight( .8f, -.5f, -1.f )
    , topLeft(    -.8f,  .5f, -1.f )
    , type( TYPE_FIXED )
{
}

Wall::Wall( const Vector3f& bl, const Vector3f& br, const Vector3f& tl )
    : bottomLeft( bl )
    , bottomRight( br )
    , topLeft( tl )
    , type( TYPE_FIXED )
{
}

void Wall::resizeHorizontal( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const Vector3f u_2   = (bottomRight - bottomLeft) * .5f;
    const Vector3f delta = u_2 * (ratio - 1.f);
    bottomLeft  -= delta;
    bottomRight += delta;
    topLeft     -= delta;
}

void Wall::resizeVertical( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const Vector3f v_2   = (topLeft - bottomLeft) * .5f;
    const Vector3f delta = v_2 * (ratio - 1.f);
    bottomLeft  -= delta;
    bottomRight -= delta;
    topLeft     += delta;
}

void Wall::resizeLeft( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const Vector3f u   = bottomRight - bottomLeft;
    const Vector3f delta = u * (ratio - 1.f);
    bottomLeft  -= delta;
    topLeft     -= delta;
}

void Wall::resizeRight( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const Vector3f u   = bottomRight - bottomLeft;
    const Vector3f delta = u * (ratio - 1.f);
    bottomRight += delta;
}

void Wall::resizeTop( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const Vector3f v = topLeft - bottomLeft;
    const Vector3f delta = v * (ratio - 1.f);
    topLeft     += delta;
}

void Wall::resizeBottom( const float ratio )
{
    if( ratio == 1.f || ratio < 0.f )
        return;

    const Vector3f v = topLeft - bottomLeft;
    const Vector3f delta = v * (ratio - 1.f);
    bottomLeft  -= delta;
    bottomRight -= delta;
}

void Wall::resizeHorizontalToAR( const float aspectRatio )
{
    const Vector3f u_2   = (bottomRight - bottomLeft) * .5f;
    const Vector3f v_2   = (topLeft - bottomLeft) * .5f;

    const float currentAR = u_2.length() / v_2.length();
    const float ratio = aspectRatio / currentAR;

    // Same as resizeHorizontal, but C&P since we have u_2 already
    const Vector3f delta = u_2 * (ratio - 1.f);
    bottomLeft  -= delta;
    bottomRight += delta;
    topLeft     -= delta;
}

void Wall::moveFocus( const Vector3f& eye, const float ratio )
{
    if( ratio == 1.0f )
        return;

    bottomLeft  = eye + ( bottomLeft - eye ) * ratio;
    bottomRight = eye + ( bottomRight - eye ) * ratio;
    topLeft     = eye + ( topLeft - eye ) * ratio;
}

void Wall::apply( const Viewport& viewport )
{
    const Vector3f u = bottomRight - bottomLeft;
    const Vector3f v = topLeft - bottomLeft;

    bottomLeft  = bottomLeft + u * viewport.x + v * viewport.y;
    bottomRight = bottomLeft + u * viewport.w;
    topLeft     = bottomLeft + v * viewport.h;
}

void Wall::scale( const float ratio )
{
    if( ratio == 1.0f )
        return;

    bottomLeft *= ratio;
    bottomRight *= ratio;
    topLeft *= ratio;
}

Wall& Wall::operator = ( const Projection& projection )
{
    const float width  = fabs( projection.distance * 2.0 *
                               tanf(DEG2RAD( .5f * projection.fov[0] )));
    const float height = fabs( projection.distance * 2.0 *
                               tanf(DEG2RAD( .5f * projection.fov[1] )));

    bottomLeft[0]  = -width * 0.5f;
    bottomLeft[1]  = -height * 0.5f;
    bottomLeft[2]  = 0;
    bottomRight[0] = width * 0.5f;
    bottomRight[1] = -height * 0.5f;
    bottomRight[2] = 0;
    topLeft[0]     = -width * 0.5f;
    topLeft[1]     = height * 0.5f;
    topLeft[2]     = 0;

    const float cosP = cos(DEG2RAD( projection.hpr[1] ));
    const float sinP = sin(DEG2RAD( projection.hpr[1] ));
    const float cosH = cos(DEG2RAD( projection.hpr[0] ));
    const float sinH = sin(DEG2RAD( projection.hpr[0] ));
    const float cosR = cos(DEG2RAD( projection.hpr[2] ));
    const float sinR = sin(DEG2RAD( projection.hpr[2] ));

    Matrix3f  mat ;
    const float cosPsinH = cosP * sinH;
    const float sinPsinH = sinP * sinH;

    mat.array[0] =  cosH * cosR;
    mat.array[1] = -cosH * sinR;
    mat.array[2] = -sinH;
    mat.array[3] = -sinPsinH * cosR + cosP * sinR;
    mat.array[4] =  sinPsinH * sinR + cosP * cosR;
    mat.array[5] = -sinP * cosH;
    mat.array[6] =  cosPsinH * cosR + sinP * sinR;
    mat.array[7] = -cosPsinH * sinR + sinP * cosR;
    mat.array[8] =  cosP * cosH;

    bottomLeft  = mat * bottomLeft;
    bottomRight = mat * bottomRight;
    topLeft     = mat * topLeft;

    Vector3f w( mat.array[6], mat.array[7], mat.array[8] );

    bottomLeft  = bottomLeft  + projection.origin  + w *  projection.distance;
    bottomRight = bottomRight + projection.origin  + w *  projection.distance;
    topLeft     = topLeft     + projection.origin  + w *  projection.distance;

    return *this;
}

Wall& Wall::operator = ( const Matrix4f& xfm )
{
    bottomLeft  = xfm * Vector4f( -1.0f, -1.0f, -1.0f, 1.0f );
    bottomRight = xfm * Vector4f(  1.0f, -1.0f, -1.0f, 1.0f );
    topLeft     = xfm * Vector4f( -1.0f,  1.0f, -1.0f, 1.0f );
    return *this;
}

bool Wall::operator == ( const Wall& rhs ) const
{
    return ( bottomLeft.equals( rhs.bottomLeft, 0.0001f )   &&
             bottomRight.equals( rhs.bottomRight, 0.0001f ) &&
             topLeft.equals( rhs.topLeft, 0.0001f ));
}

bool Wall::operator != ( const Wall& rhs ) const
{
    return ( !bottomLeft.equals( rhs.bottomLeft, 0.0001f )   ||
             !bottomRight.equals( rhs.bottomRight, 0.0001f ) ||
             !topLeft.equals( rhs.topLeft, 0.0001f ));
}

std::ostream& operator << ( std::ostream& os, const Wall& wall )
{
    const std::ios::fmtflags flags = os.flags();
    os.setf( std::ios::fixed, std::ios::floatfield );

    os << lunchbox::disableHeader << lunchbox::disableFlush
       << "wall" << std::endl
       << "{" << std::endl << lunchbox::indent
       << "bottom_left  " << wall.bottomLeft << std::endl
       << "bottom_right " << wall.bottomRight << std::endl
       << "top_left     " << wall.topLeft << std::endl;
    if( wall.type != Wall::TYPE_FIXED )
        os << "type         " << wall.type << std::endl;
    os << lunchbox::exdent << "}"
       << lunchbox::enableFlush << lunchbox::enableHeader << std::endl;

    os.setf( flags );
    return os;
}

std::ostream& operator << ( std::ostream& os, const Wall::Type& type )
{
    switch( type )
    {
    case Wall::TYPE_HMD:
        os << "HMD";
        break;

    default:
        LBASSERT( false );

    case Wall::TYPE_FIXED:
        os << "fixed";
        break;
    }
    return os;
}

}
}
