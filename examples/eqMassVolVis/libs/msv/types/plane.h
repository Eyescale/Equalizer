
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__PLANE
#define MASS_VOL__PLANE

#include "vec3.h"

#include "types.h"

#include <iostream>

namespace massVolVis
{

template< typename T >
struct Plane
{
    Plane() : w( 0 ) {}

    Plane( T x_, T y_, T z_, T w_ ) : normal( x_, y_, z_ ), w( w_ ) {}
    Plane( Vec3<T> normal_,  T w_ )  : normal( normal_ )  , w( w_ ) {}

    double distanceToPoint( const Vec3<T>& p ) const;

    Vec3<T> normal;
    T w;

    friend std::ostream& operator<<( std::ostream& out, const Plane& plane )
    {
        out << "Plane<" << TypeParseTraits<T>::name << ">(" << plane.normal.x << ", "
                                                            << plane.normal.y << ", "
                                                            << plane.normal.z << ", "
                                                            << plane.w << ")";
        return out;
    }
};

typedef Plane<double> Plane_d;


} // namespace

#endif // MASS_VOL__PLANE

