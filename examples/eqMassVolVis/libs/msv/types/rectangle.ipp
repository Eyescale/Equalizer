
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "rectangle.h"
#include <msv/util/hlp.h> // min, max
#include <cmath>

namespace massVolVis
{

template< typename T >
void Rectangle<T>::expand( const Vec2<T>& point )
{
    if( s.x > point.x ) s.x = point.x;
    if( e.x < point.x ) e.x = point.x;
    if( s.y > point.y ) s.y = point.y;
    if( e.y < point.y ) e.y = point.y;
}


template< typename T >
void Rectangle<T>::expand( const Rectangle<T>& rect )
{
    expand( rect.s );
    expand( rect.e );
}


template< typename T >
float Rectangle<T>::getDiagonalSize()
{
    float dx = e.x-s.x;
    float dy = e.y-s.y;
    return sqrt(dx*dx+dy*dy);
}


template< typename T >
T Rectangle<T>::getWidth()  const
{
    if( e.x < s.x )
        return 0;

    return e.x - s.x;
}

template< typename T >
T Rectangle<T>::getHeight() const
{
    if( e.y < s.y )
        return 0;

    return e.y - s.y;
}


}


