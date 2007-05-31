
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_RENDERCONTEXT_H
#define EQ_RENDERCONTEXT_H

#include <eq/base/base.h>
#include <eq/client/colorMask.h>
#include <eq/client/eye.h>
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
        uint32_t       frameID;        //<! identifier from Config::beginFrame

        uint32_t       buffer;         //<! buffer as passed to glDrawBuffer() 
        ColorMask      drawBufferMask; //<! draw color mask for anaglyph stereo
        PixelViewport  pvp;            //<! pixel viewport of channel wrt window
        vmml::Frustumf frustum;        //<! frustum for projection matrix
        vmml::Matrix4f headTransform;  //<! frustum transform for modelview

        Viewport       vp;             //<! fractional viewport wrt dest channel
        Range          range;          //<! database-range wrt to dest channel
        vmml::Vector2i offset;         //<! absolute position wrt dest channel
        Eye            eye;            //<! current eye pass
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const RenderContext& ctx );
}

#endif // EQ_RENDERCONTEXT_H
