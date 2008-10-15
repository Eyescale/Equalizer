
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_VIEWDATA_H
#define EQSERVER_VIEWDATA_H

#include <eq/base/base.h>
#include <vmmlib/matrix4.h>

namespace eq
{
    class Projection;
    class Wall;

namespace server
{
    /** 
     * Data derived from eq::View, in a general, optimized format used for
     * frustum calculations during rendering.
     */
    struct ViewData
    {
        ViewData() : width(0.f), height(0.f) {}

        bool isValid() const { return (width!=0.f && height!=0.f); }
        void invalidate() { width = 0.f; height = 0.f; }

        void applyProjection( const eq::Projection& projection );
        void applyWall( const eq::Wall& wall );

        float width;
        float height;
        vmml::Matrix4f xfm;
    };

    std::ostream& operator << ( std::ostream& os, const ViewData& viewData ); 
}
}
#endif // EQ_VIEWDATA_H
