
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RECTANGLE_H
#define MASS_VOL__RECTANGLE_H

#include "vec2.h"

namespace massVolVis
{

template< typename T >
struct Rectangle
{
    Rectangle(){}

    explicit Rectangle( const Vec2<T>& s_, const Vec2<T>& e_ ) : s( s_ ), e( e_ ) {}

    explicit Rectangle( const Vec2<T>& e_ ) : e( e_ ) {}

    Rectangle( const T sX, const T sY, const T eX, const T eY )
        : s( sX, sY ), e( eX, eY ) {}

    bool valid() const { return s < e; }

    void expand( const Vec2<T>& point );
    void expand( const Rectangle<T>& rect );

    T getWidth()  const;
    T getHeight() const;

    float getDiagonalSize();

    uint64_t getAreaSize() const { return (e.x-s.x)*(e.y-s.y); }


    friend std::ostream& operator<<( std::ostream& out, const Rectangle<T>& rect )
    {
        out << "Rect<" << TypeParseTraits<T>::name << ">[" << rect.s << ", "<< rect.e << "]";
        return out;
    }

// data
    Vec2<T> s; //start
    Vec2<T> e; //end
};

typedef Rectangle<uint32_t> Rect_ui32;
typedef Rectangle< int32_t> Rect_i32;
typedef Rectangle<   float> Rect_f;
}

#endif // MASS_VOL__RECTANGLE_H
