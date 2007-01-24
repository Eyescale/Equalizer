
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEW_H
#define EQ_VIEW_H

#include <eq/base/base.h>
#include <eq/vmmlib/Matrix4.h>

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
    struct EQ_EXPORT View
    {
        View() : width(0.f), height(0.f) {}

        bool isValid() const { return (width!=0.f && height!=0.f); }

        void applyProjection( const Projection& projection );
        void applyWall( const Wall& wall );

        float width;
        float height;
        vmml::Matrix4f xfm;
    };
}

#endif // EQ_VIEW_H
