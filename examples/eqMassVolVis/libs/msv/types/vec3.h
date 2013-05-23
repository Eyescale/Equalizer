
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__VEC_3_UINT
#define MASS_VOL__VEC_3_UINT

#include "types.h"

#include <iostream>

namespace massVolVis
{

/**
 * Lightweight Vec3 implementation with overflowing prevention for
 * arithmetic operations.
 */
template< typename T >
struct Vec3
{
    Vec3( ) : x( 0 ), y( 0 ), z( 0 ) {}

    explicit Vec3( T x_             ) : x( x_ ), y( x_ ), z( x_ ) {}

    explicit Vec3( T x_, T y_, T z_ ) : x( x_ ), y( y_ ), z( z_ ) {}

    void set( T x_, T y_, T z_ ) { x = x_; y = y_; z = z_; }

    template < typename M >
    Vec3( Vec3<M> vec ) : x( vec.x ), y( vec.y ), z( vec.z ){}

    union
    {
        struct
        {
            T x;
            T y;
            T z;
        };
        struct
        {
            T w;
            T h;
            T d;
        };
        struct
        {
            T r;
            T g;
            T b;
        };
    };

    uint64_t getAreaSize() const;


    Vec3 operator / ( const T value ) const;
    Vec3 operator * ( const T value ) const;

    Vec3 operator / ( const Vec3& other) const;

    Vec3 operator - ( const Vec3& other ) const;
    Vec3 operator + ( const Vec3& other ) const;

    Vec3& operator -= ( const Vec3& other );
    Vec3& operator += ( const Vec3& other );

    Vec3 operator + ( const T value ) const;
    Vec3 operator - ( const T value ) const;

    Vec3 cross( const Vec3& other ) const;
    T    dot( const Vec3& other ) const;

    bool operator >  ( const Vec3& other ) const;
    bool operator <  ( const Vec3& other ) const;
    bool operator <= ( const Vec3& other ) const;
    bool operator >= ( const Vec3& other ) const;

    bool operator == ( const Vec3& other ) const;
    bool operator != ( const Vec3& other ) const;

    friend std::ostream& operator<<( std::ostream& out, const Vec3& vec )
    {
        out << "vec<" << TypeParseTraits<T>::name << ">(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return out;
    }
};

typedef Vec3<uint16_t> Vec3_ui16;
typedef Vec3<uint32_t> Vec3_ui32;
typedef Vec3< int32_t> Vec3_i32;
typedef Vec3< int64_t> Vec3_i64;
typedef Vec3<   float> Vec3_f;
typedef Vec3<  double> Vec3_d;


} // namespace

#endif // MASS_VOL__VEC_3_UINT

