
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "vec2.h"

#include <msv/util/hlp.h>

#include "limits.h"


using hlpFuncs::myMax;
using hlpFuncs::myMin;

namespace massVolVis
{

template<>
template<>
Vec2<uint32_t>::Vec2( Vec2<int32_t> vec )
{
    x = myMax( 0, vec.x );
    y = myMax( 0, vec.y );
}


template<>
template<>
Vec2<uint16_t>::Vec2( Vec2<uint32_t> vec )
{
    x = myMin( vec.x, (uint32_t)Limits<uint16_t>::max() );
    y = myMin( vec.y, (uint32_t)Limits<uint16_t>::max() );
}


template< typename T >
uint64_t Vec2<T>::getAreaSize() const
{
    return x*y;
}


template< typename T >
bool Vec2<T>::operator == ( const Vec2<T>& other ) const
{
    return (x == other.x) && (y == other.y);
}


template< typename T >
Vec2<T> Vec2<T>::operator+( const Vec2<T>& other ) const
{
    T dx, dy;

    if( other.x <= 0 ) dx = ( x > Limits<T>::min() - other.x ) ? ( x + other.x ) : Limits<T>::min();
    else               dx = ( x < Limits<T>::max() - other.x ) ? ( x + other.x ) : Limits<T>::max();

    if( other.y <= 0 ) dy = ( y > Limits<T>::min() - other.y ) ? ( y + other.y ) : Limits<T>::min();
    else               dy = ( y < Limits<T>::max() - other.y ) ? ( y + other.y ) : Limits<T>::max();

    return Vec2<T>( dx, dy );
}


template< typename T >
Vec2<T> Vec2<T>::operator-( const Vec2<T>& other ) const
{
    T dx, dy;

    if( other.x <= 0 ) dx = ( x < Limits<T>::max() + other.x ) ? ( x - other.x ) : Limits<T>::max();
    else               dx = ( x > Limits<T>::min() + other.x ) ? ( x - other.x ) : Limits<T>::min();

    if( other.y <= 0 ) dy = ( y < Limits<T>::max() + other.y ) ? ( y - other.y ) : Limits<T>::max();
    else               dy = ( y > Limits<T>::min() + other.y ) ? ( y - other.y ) : Limits<T>::min();

    return Vec2<T>( dx, dy );
}


template< typename T >
bool Vec2<T>::operator < ( const Vec2<T>& other ) const
{
    return (x < other.x) && (y < other.y);
}

}

