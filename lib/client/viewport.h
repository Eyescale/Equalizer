
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEWPORT_H
#define EQ_VIEWPORT_H

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
        Viewport() : x(0), y(0), w(1), h(1)  {}

        Viewport( const float x, const float y, const float w, const float h )
                : x(x), y(y), w(w), h(h)  {}
        //*}

        void invalidate() { x=0; y=0; w=0; h=0; }
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

        bool isValid() const { return (w>0 && h>0); }
        bool isFullScreen() const { return ( x==0 && y==0 && w==1 && h==1 ); }

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
