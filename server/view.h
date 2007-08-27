
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_VIEW_H
#define EQS_VIEW_H

#include <eq/base/base.h>
#include <vmmlib/matrix4.h>

namespace eqs
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
        View() : width(0.f), height(0.f) {}

        bool isValid() const { return (width!=0.f && height!=0.f); }

        void applyProjection( const Projection& projection );
        void applyWall( const Wall& wall );

        float width;
        float height;
        vmml::Matrix4f xfm;
    };

    std::ostream& operator << ( std::ostream& os, const View& view ); 
}

#endif // EQ_VIEW_H
