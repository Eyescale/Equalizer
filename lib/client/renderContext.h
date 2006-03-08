
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RENDERCONTEXT_H
#define EQ_RENDERCONTEXT_H

#include "pixelViewport.h"

#include <GL/gl.h>

namespace eq
{
    /**
     * The context applied by the server during rendering operations.
     */
    class RenderContext 
    {
    public:
        /** 
         * Constructs a new render context.
         */
        RenderContext() : hints(0)
            {}

#       define HINT_BUFFER             0x01
#       define HINT_FRUSTUM            0x02

        uint32_t      hints;

        // HINT_BUFFER
        GLenum        drawBuffer;
        PixelViewport pvp;

        // HINT_FRUSTUM
        float         frustum[6];
        float         headTransform[16];
    };
}

#endif // EQ_RENDERCONTEXT_H
