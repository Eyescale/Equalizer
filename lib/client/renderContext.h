
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RENDERCONTEXT_H
#define EQ_RENDERCONTEXT_H

#include <eq/client/frustum.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/range.h>

#ifdef Darwin
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

namespace eq
{
    /**
     * The context applied by the server during rendering operations.
     */
    struct RenderContext 
    {
    public:
        uint32_t      frameID;
        GLenum        drawBuffer;
        Viewport      vp;
        PixelViewport pvp;
        Range         range;
        Frustum       frustum;
        float         headTransform[16];
    };

    std::ostream& operator << ( std::ostream& os, const RenderContext& ctx );
}

#endif // EQ_RENDERCONTEXT_H
