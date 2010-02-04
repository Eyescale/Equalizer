
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQ_RENDERCONTEXT_H
#define EQ_RENDERCONTEXT_H

#include <eq/client/colorMask.h>        // member
#include <eq/client/eye.h>              // member
#include <eq/client/pixel.h>            // member
#include <eq/client/subPixel.h>         // member
#include <eq/client/pixelViewport.h>    // member
#include <eq/client/range.h>            // member
#include <eq/client/types.h>
#include <eq/client/zoom.h>             // member

#include <eq/net/objectVersion.h>
#include <eq/base/base.h>



namespace eq
{
    /**
     * The context applied by the server during rendering operations.
     */
    struct RenderContext 
    {
    public: 
        EQ_EXPORT RenderContext();

        Matrix4f       headTransform;  //<! frustum transform for modelview

        PixelViewport  pvp;            //<! pixel viewport of channel wrt window
        Pixel          pixel;          //<! pixel decomposition wrt to dest
        Vector4i       overdraw;       //<! @internal for pw pp filters
        Viewport       vp;             //<! fractional viewport wrt dest channel

        Vector2i       offset;         //<! absolute position wrt dest channel
        net::ObjectVersion view;       //<! destination view id and version
        Range          range;          //<! database-range wrt to dest channel
        SubPixel       subpixel;       //<! subpixel decomposition wrt to dest
        Zoom           zoom;           //<! up/downsampling wrt to dest

        uint32_t       frameID;        //<! identifier from Config::beginFrame
        uint32_t       buffer;         //<! buffer as passed to glDrawBuffer() 
        uint32_t       taskID;         //<! @internal per-channel task counter
        uint32_t       period;         //<! DPlex period
        uint32_t       phase;          //<! DPlex phase
        Eye            eye;            //<! current eye pass

        Frustumf       frustum;        //<! frustum for projection matrix
        Frustumf       ortho;          //<! ortho frustum for projection matrix

        ColorMask      bufferMask;     //<! color mask for anaglyph stereo
        bool           alignDummy[29]; //<! @internal padding
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const RenderContext& ctx );
}

#endif // EQ_RENDERCONTEXT_H
