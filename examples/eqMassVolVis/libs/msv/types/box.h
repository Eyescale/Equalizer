
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__BOX_H
#define MASS_VOL__BOX_H

#include "vec3.h"

namespace massVolVis
{


struct BoundingSphere { float x,y,z,r; };


template< typename T >
struct Box
{
    Box(){}

    explicit Box( const Vec3<T>& s_, const Vec3<T>& e_ ) : s( s_ ), e( e_ ) {}

    explicit Box( const Vec3<T>& e_ ) : e( e_ ) {}

    Box( const T sX, const T sY, const T sZ,
           const T eX, const T eY, const T eZ )
        : s( sX, sY, sZ ), e( eX, eY, eZ ) {}

    bool valid() const { return e > s; }

    Box intersect( const Box& box ) const;

    Vec3<T> getCenter() const { return ( s + e ) / 2; }
    Vec3<float> getCenterF() const { return Vec3<float>( s + e ) / 2.f; }

    void getQuadrants( Box* quads ) const;

    // return -1 if doesn't contain, 0..7 otherwise
    int inQuad( const Vec3<T>& p, Box& quad ) const;

    bool contain( const Vec3<T>& p ) const;
    bool contain( const Box<T>&  b ) const;

    Vec3<T> getDim() const { return e - s; }

    T getDimX() const { return e.x - s.x; }

    uint64_t getAreaSize() const;

    Box operator - ( const Vec3<T>& vec ) const;
    Box operator - ( const      T value ) const;

    bool operator == ( const Box& other ) const;
    bool operator != ( const Box& other ) const;

    friend std::ostream& operator<<( std::ostream& out, const Box<T>& box )
    {
        out << "Box<" << TypeParseTraits<T>::name << ">[" << box.s << ", "<< box.e << "]";
        return out;
    }

    // returns Bounding Sphere for current Box
    BoundingSphere getBoundingSphere() const;

    // returns Bounding Sphere for current Box, but assumes that x == y == z
    BoundingSphere getBoundingSphereForCube() const;

// data
    Vec3<T> s; //start
    Vec3<T> e; //end
};

typedef Box<uint32_t> Box_ui32;
typedef Box< int32_t> Box_i32;
typedef Box<   float> Box_f;
typedef Box<  double> Box_d;

typedef std::vector<Box_f> vecBox_f;
}

#endif // MASS_VOL__BOX_H
