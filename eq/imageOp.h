
/* Copyright (c) 2016-2017, Stefan.Eilemann@epfl.ch
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

#ifndef EQ_IMAGEOP_H
#define EQ_IMAGEOP_H

#include <eq/api.h>
#include <eq/fabric/pixel.h>    // member
#include <eq/fabric/subPixel.h> // member
#include <eq/fabric/zoom.h>     // member
#include <eq/frame.h>           // member
#include <eq/types.h>
#include <eq/zoomFilter.h> // member

namespace eq
{
/** A structure describing an image assembly task, used by the Compositor. */
struct ImageOp
{
    ImageOp()
        : image(0)
        , buffers(Frame::Buffer::none)
        , zoomFilter(FILTER_LINEAR)
    {
    }
    EQ_API ImageOp(const Frame* frame, const Image* image);

    const Image* image;    //!< The image to assemble
    Frame::Buffer buffers; //!< The Frame buffer attachments to use
    Vector2i offset;       //!< The offset wrt destination window
    ZoomFilter zoomFilter; //!< The zoom Filter from Frame
    Zoom zoom;             //!< The zoom factor
};
}

#endif // EQ_IMAGEOP_H
