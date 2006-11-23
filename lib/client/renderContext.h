
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RENDERCONTEXT_H
#define EQ_RENDERCONTEXT_H

#include <eq/client/frustum.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/range.h>
#include <eq/vmmlib/VMMLib.h>

namespace eq
{
    /**
     * The context applied by the server during rendering operations.
     */
    struct RenderContext 
    {
    public:
        uint32_t       frameID;       //<! identifier from Config::beginFrame
        uint32_t       buffer;        //<! buffer as passed to glDrawBuffer()
        PixelViewport  pvp;           //<! pixel viewport of channel wrt window
        Range          range;         //<! database-range to be rendered
        Frustum        frustum;       //<! frustum for projection matrix
        vmml::Matrix4f headTransform; //<! frustum transform for modelview
    };

    std::ostream& operator << ( std::ostream& os, const RenderContext& ctx );
}

#endif // EQ_RENDERCONTEXT_H
