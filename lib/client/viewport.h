
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

        void multiply( const Viewport& vp )
            {
                x += vp.x * w;
                y += vp.y * h;
                w *= vp.w;
                h *= vp.h;
            }

        bool isValid() const { return (w>0 && h>0); }

        float x;
        float y;
        float w;
        float h;
    };

    inline std::ostream& operator << ( std::ostream& os, const Viewport& vp )
    {
        os << "{" << vp.x << " " << vp.y << " " << vp.w << " " << vp.h << "}";
        return os;
    }
}

#endif // EQ_VIEWPORT_H
