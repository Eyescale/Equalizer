
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEWPORT_H
#define EQ_VIEWPORT_H

#include <iostream>

namespace eq
{
    /**
     * Holds a fractional viewport along with some methods for manipulation.
     */
    class Viewport 
    {
    public:
        /**
         * @name Constructors
         */
        //*{
        Viewport() : x(0.0f), y(0.0f), w(1.0f), h(1.0f)  {}

        Viewport( const float x, const float y, const float w, const float h )
                : x(x), y(y), w(w), h(h)  {}
        //*}

        void invalidate() { x=0.0f; y=0.0f; w=-1.0f; h=-1.0f; }
        Viewport& operator *= ( const Viewport& rhs )
            {
                x += rhs.x * w;
                y += rhs.y * h;
                w *= rhs.w;
                h *= rhs.h;
                return *this;
            }

        bool operator == ( const Viewport& rhs ) const 
            { 
                return ( x==rhs.x && x==rhs.x && 
                         y==rhs.y && w==rhs.w && h==rhs.h);
            }

        /** 
         * @return true if the viewport has a non-negative, but potentially
         *         empty, size.
         */
        bool isValid() const { return (w>=0.0f && h>=0.0f); }
        
        /** 
         * @return true if the viewport has a non-zero area, i.e, it is
         *         not empty.
         */
        bool hasArea() const { return (w>0.0f && h>0.0f); }

        bool isFullScreen() const 
            { return ( x==0.0f && y==0.0f && w==1.0f && h==1.0f ); }

        float x;
        float y;
        float w;
        float h;
    };

    inline std::ostream& operator << ( std::ostream& os, const Viewport& vp )
    {
        os << "[ " << vp.x << " " << vp.y << " " << vp.w << " " << vp.h << " ]";
        return os;
    }
}

#endif // EQ_VIEWPORT_H
