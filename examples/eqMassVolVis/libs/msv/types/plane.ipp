
/* Copyright (c) 2012, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "plane.h"

namespace massVolVis
{

template< typename T >
double Plane<T>::distanceToPoint( const Vec3<T>& p ) const
{
    return p.dot( normal ) + w;
}

}
