
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
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

#include "imageOp.h"

#include "frame.h"
#include "frameData.h"
#include "image.h"

namespace eq
{
ImageOp::ImageOp( const Frame* frame, const Image* img )
    : image( img )
    , buffers( frame->getFrameData()->getBuffers( ))
    , offset( frame->getFrameData()->getContext().offset )
    , zoom( frame->getZoom( ))
{
    ConstFrameDataPtr data = frame->getFrameData();
    zoom.apply( data->getZoom( ));
    zoom.apply( image->getZoom( ));
    zoomFilter = zoom == Zoom::NONE ? FILTER_NEAREST : frame->getZoomFilter();
}
}
