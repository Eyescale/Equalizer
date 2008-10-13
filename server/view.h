
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_VIEW_H
#define EQSERVER_VIEW_H

#include <eq/base/base.h>
#include <vmmlib/matrix4.h>

namespace eq
{
    class Projection;
    class Wall;

namespace server
{
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

        void applyProjection( const eq::Projection& projection );
        void applyWall( const eq::Wall& wall );

        float width;
        float height;
        vmml::Matrix4f xfm;
    };

    std::ostream& operator << ( std::ostream& os, const View& view ); 
}
}
#endif // EQ_VIEW_H
