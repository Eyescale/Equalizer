
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RENDERCONTEXT_H
#define EQ_RENDERCONTEXT_H

#include <eq/base/base.h>
#include <eq/client/colorMask.h>
#include <eq/client/eye.h>
#include <eq/client/pixel.h>
#include <eq/client/pixelViewport.h>
#include <eq/client/range.h>
#include <vmmlib/vmmlib.h>

namespace eq
{
    /**
     * The context applied by the server during rendering operations.
     */
    struct RenderContext 
    {
    public:        
        uint32_t       frameID;        //<! identifier from Config::beginFrame

        uint32_t       buffer;         //<! buffer as passed to glDrawBuffer() 
        ColorMask      drawBufferMask; //<! draw color mask for anaglyph stereo
        PixelViewport  pvp;            //<! pixel viewport of channel wrt window
        vmml::Frustumf frustum;        //<! frustum for projection matrix
        vmml::Frustumf ortho;          //<! ortho frustum for projection matrix
        vmml::Matrix4f headTransform;  //<! frustum transform for modelview

        Viewport       vp;             //<! fractional viewport wrt dest channel
        Range          range;          //<! database-range wrt to dest channel
        Pixel          pixel;          //<! pixel decomposition wrt to dest
        vmml::Vector2i offset;         //<! absolute position wrt dest channel
        vmml::Vector2i screenOrigin;   //<! absolute position wrt screen
        vmml::Vector2i screenSize;     //<! size of screen
        Eye            eye;            //<! current eye pass
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const RenderContext& ctx );
}

#endif // EQ_RENDERCONTEXT_H
