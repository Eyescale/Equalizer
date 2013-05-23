
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "box.h"
#include <msv/util/hlp.h> // min, max
#include <cmath> // sqrt

namespace massVolVis
{

using hlpFuncs::myMax;
using hlpFuncs::myMin;

template< typename T >
Box<T> Box<T>::intersect( const Box<T>& box ) const
{
    Box<T> r;

    r.s.x = myMax( s.x, box.s.x );
    r.s.y = myMax( s.y, box.s.y );
    r.s.z = myMax( s.z, box.s.z );

    r.e.x = myMin( e.x, box.e.x );
    r.e.y = myMin( e.y, box.e.y );
    r.e.z = myMin( e.z, box.e.z );

    return r;
}

template< typename T >
void Box<T>::getQuadrants( Box<T>* quads ) const
{
    Vec3<T> c  = getCenter();

    quads[0] = Box<T>(  s.x, s.y, s.z,   c.x, c.y, c.z  );
    quads[1] = Box<T>(  c.x, s.y, s.z,   e.x, c.y, c.z  );
    quads[2] = Box<T>(  s.x, c.y, s.z,   c.x, e.y, c.z  );
    quads[3] = Box<T>(  c.x, c.y, s.z,   e.x, e.y, c.z  );

    quads[4] = Box<T>(  s.x, s.y, c.z,   c.x, c.y, e.z  );
    quads[5] = Box<T>(  c.x, s.y, c.z,   e.x, c.y, e.z  );
    quads[6] = Box<T>(  s.x, c.y, c.z,   c.x, e.y, e.z  );
    quads[7] = Box<T>(  c.x, c.y, c.z,   e.x, e.y, e.z  );
}


template< typename T >
bool Box<T>::contain( const Vec3<T>& p ) const
{
    if( s <= p && p < e )
        return true;

    return false;
}


template< typename T >
bool Box<T>::contain( const Box<T>& b ) const
{
    if( s <= b.s && b.s <  e &&
        s <  b.e && b.e <= e )
        return true;

    return false;
}


template< typename T >
BoundingSphere Box<T>::getBoundingSphere() const
{
    const Vec3<float> c = getCenterF();
    const Vec3<T> d = getDim();
    const float radius = std::sqrt( static_cast<float>( d.x*d.x + d.y*d.y + d.z*d.z )/4.f );

    BoundingSphere bs = { c.x, c.y, c.z, radius };

    return bs;
}


template< typename T >
BoundingSphere Box<T>::getBoundingSphereForCube() const
{
    const Vec3<float> c = getCenterF();

    const float dimX = getDimX();
    BoundingSphere bs = { c.x, c.y, c.z, std::sqrt( dimX*dimX*3.f/4.f )};

    return bs;
}

template< typename T >
int Box<T>::inQuad( const Vec3<T>& p, Box<T>& quad ) const
{
    if( !contain( p ))
        return -1;

    Box<T> quads[8];
    getQuadrants( quads );

    for( int i = 0; i < 7; ++i )
        if( quads[i].contain( p ))
        {
            quad = quads[i];
            return i;
        }

    quad = quads[7];
    return 7;
}


template< typename T >
Box<T> Box<T>::operator - ( const Vec3<T>& vec ) const
{
    return Box<T>( s-vec, e-vec );
}

template< typename T >
Box<T> Box<T>::operator - ( const T value ) const
{
    return Box<T>( s-Vec3<T>(value), e-Vec3<T>(value) );
}


template< typename T >
bool Box<T>::operator == ( const Box<T>& other ) const
{
    return (s == other.s) && (e == other.e);
}

template< typename T >
bool Box<T>::operator != ( const Box<T>& other ) const
{
    return (s != other.s) || (e != other.e);
}

template< typename T >
uint64_t Box<T>::getAreaSize() const
{
    Vec3<T> dim = getDim();
    return dim.x * dim.y * dim.z;
}

}


