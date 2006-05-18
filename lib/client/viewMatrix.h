
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEWMATRIX_H
#define EQ_VIEWMATRIX_H

namespace eq
{
    /** 
     * A generic view definition
     *
     * The view is defined by the size of the viewport and a
     * transformation matrix.
     * @todo better doc
     */
    struct ViewMatrix
    {
        ViewMatrix() : width(0), height(0) {}

        bool isValid() const { return (width!=0 && height!=0); }

        float width;
        float height;
        float xfm[16];
    };
}

#endif // EQ_VIEWMATRIX_H
