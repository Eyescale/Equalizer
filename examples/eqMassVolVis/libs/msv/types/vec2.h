
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__VEC_2
#define MASS_VOL__VEC_2

#include "types.h"

#include <iostream>

namespace massVolVis
{

/**
 * Lightweight Vec3 implementation with overflowing prevention for
 * arithmetic operations.
 */
template< typename T >
struct Vec2
{
    Vec2( ) : x( 0 ), y( 0 ) {}

    explicit Vec2( T x_       ) : x( x_ ), y( x_ ) {}

    explicit Vec2( T x_, T y_ ) : x( x_ ), y( y_ ) {}

    template < typename M >
    Vec2( Vec2<M> vec ) : x( vec.x ), y( vec.y ){}

    union
    {
        struct
        {
            T x;
            T y;
        };
        struct
        {
            T w;
            T h;
        };
    };

    bool operator < ( const Vec2<T>& other ) const;

    bool operator == ( const Vec2<T>& other ) const;

    Vec2<T> operator + ( const Vec2<T>& other ) const;
    Vec2<T> operator - ( const Vec2<T>& other ) const;

    uint64_t getAreaSize() const;


    friend std::ostream& operator<<( std::ostream& out, const Vec2& vec )
    {
        out << "vec<" << TypeParseTraits<T>::name << ">(" << vec.x << ", " << vec.y << ")";
        return out;
    }
};

typedef Vec2<uint32_t> Vec2_ui32;
typedef Vec2<uint16_t> Vec2_ui16;
typedef Vec2<   float> Vec2_f;


} // namespace

#endif // MASS_VOL__VEC_2

