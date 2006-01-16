
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_VIEWPORT_H
#define EQBASE_VIEWPORT_H

namespace eqBase
{
    /**
     * Holds a fractional viewport along with some methods for manipulation.
     */
    class Viewport 
    {
    public:
        /** 
         * Constructs a new viewport.
         */
        Viewport() : x(0), y(0), w(1), h(1)
            {}

        void multiply( const Viewport& vp )
            {
                x += vp.x * w;
                y += vp.y * h;
                w *= vp.w;
                h *= vp.h;
            }

        float x;
        float y;
        float w;
        float h;
    };
}

#endif //EQBASE_VIEWPORT_H
