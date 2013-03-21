
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "vec3.h"

#include <msv/util/hlp.h>

#include "limits.h"


using hlpFuncs::myMax;
using hlpFuncs::myMin;

namespace massVolVis
{

template<>
template<>
Vec3<uint32_t>::Vec3( Vec3<int32_t> vec )
{
    x = myMax( 0, vec.x );
    y = myMax( 0, vec.y );
    z = myMax( 0, vec.z );
}


template<>
template<>
Vec3<uint16_t>::Vec3( Vec3<uint32_t> vec )
{
    x = myMin( vec.x, (uint32_t)Limits<uint16_t>::max() );
    y = myMin( vec.y, (uint32_t)Limits<uint16_t>::max() );
    z = myMin( vec.z, (uint32_t)Limits<uint16_t>::max() );
}


template< typename T >
Vec3<T> Vec3<T>::operator/( const T value ) const
{
    return Vec3<T>( x / value, y / value, z / value );
}


template< typename T >
Vec3<T> Vec3<T>::operator/( const Vec3& other ) const
{
    return Vec3<T>( x / other.x, y / other.y, z / other.z );
}


template< typename T >
Vec3<T> Vec3<T>::operator*( const T value ) const
{
    return Vec3<T>( x * value, y * value, z * value );
}


template< typename T >
Vec3<T> Vec3<T>::operator-( const Vec3<T>& other ) const
{
    T dx, dy, dz;

    if( other.x <= 0 ) dx = ( x < Limits<T>::max() + other.x ) ? ( x - other.x ) : Limits<T>::max();
    else               dx = ( x > Limits<T>::min() + other.x ) ? ( x - other.x ) : Limits<T>::min();

    if( other.y <= 0 ) dy = ( y < Limits<T>::max() + other.y ) ? ( y - other.y ) : Limits<T>::max();
    else               dy = ( y > Limits<T>::min() + other.y ) ? ( y - other.y ) : Limits<T>::min();

    if( other.z <= 0 ) dz = ( z < Limits<T>::max() + other.z ) ? ( z - other.z ) : Limits<T>::max();
    else               dz = ( z > Limits<T>::min() + other.z ) ? ( z - other.z ) : Limits<T>::min();

    return Vec3<T>( dx, dy, dz );
}


template< typename T >
Vec3<T> Vec3<T>::operator+( const Vec3<T>& other ) const
{
    T dx, dy, dz;

    if( other.x <= 0 ) dx = ( x > Limits<T>::min() - other.x ) ? ( x + other.x ) : Limits<T>::min();
    else               dx = ( x < Limits<T>::max() - other.x ) ? ( x + other.x ) : Limits<T>::max();

    if( other.y <= 0 ) dy = ( y > Limits<T>::min() - other.y ) ? ( y + other.y ) : Limits<T>::min();
    else               dy = ( y < Limits<T>::max() - other.y ) ? ( y + other.y ) : Limits<T>::max();

    if( other.z <= 0 ) dz = ( z > Limits<T>::min() - other.z ) ? ( z + other.z ) : Limits<T>::min();
    else               dz = ( z < Limits<T>::max() - other.z ) ? ( z + other.z ) : Limits<T>::max();

    return Vec3<T>( dx, dy, dz );
}


template< typename T >
Vec3<T>& Vec3<T>::operator -= ( const Vec3<T>& other )
{
    *this = *this - other;
    return *this;
}


template< typename T >
Vec3<T>& Vec3<T>::operator += ( const Vec3<T>& other )
{
    *this = *this + other;
    return *this;
}


template< typename T >
Vec3<T> Vec3<T>::operator-( const T value ) const
{
    return *this - Vec3<T>( value );
}


template< typename T >
Vec3<T> Vec3<T>::operator+( const T value ) const
{
    return *this + Vec3<T>( value );
}


template< typename T >
bool Vec3<T>::operator>( const Vec3<T>& other ) const
{
    return (x > other.x) && (y > other.y) && (z > other.z);
}


template< typename T >
bool Vec3<T>::operator<( const Vec3<T>& other ) const
{
    return (x < other.x) && (y < other.y) && (z < other.z);
}


template< typename T >
bool Vec3<T>::operator>=( const Vec3<T>& other ) const
{
    return (x >= other.x) && (y >= other.y) && (z >= other.z);
}


template< typename T >
bool Vec3<T>::operator<=( const Vec3& other ) const
{
    return (x <= other.x) && (y <= other.y) && (z <= other.z);
}


template< typename T >
bool Vec3<T>::operator == ( const Vec3<T>& other ) const
{
    return (x == other.x) && (y == other.y) && (z == other.z);
}


template< typename T >
bool Vec3<T>::operator != ( const Vec3<T>& other ) const
{
    return (x != other.x) || (y != other.y) || (z != other.z);
}


template< typename T >
uint64_t Vec3<T>::getAreaSize() const
{
    return x*y*z;
}

template< typename T >
Vec3<T> Vec3<T>::cross( const Vec3<T>& other ) const
{
    return Vec3<T>( y*other.z - z*other.y,
                    z*other.x - x*other.z,
                    x*other.y - y*other.x );
}

template< typename T >
T Vec3<T>::dot( const Vec3<T>& other ) const
{
    return x*other.x + y*other.y + z*other.z;
}


}

