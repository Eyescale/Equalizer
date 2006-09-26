
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

namespace eq
{
    class Projection;
    class Wall;

    /** 
     * A generic view definition
     *
     * The view is defined by the size of the viewport and a
     * transformation matrix.
     * @todo better doc
     */
    struct View
    {
        View() : width(0), height(0) {}

        bool isValid() const { return (width!=0 && height!=0); }

        void applyProjection( const Projection& projection );
        void applyWall( const Wall& wall );

        float width;
        float height;
        float xfm[16];
    };
}

#endif // EQ_VIEW_H
