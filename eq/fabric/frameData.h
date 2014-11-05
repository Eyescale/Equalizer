
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_FRAMEDATA_H
#define EQFABRIC_FRAMEDATA_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>

#include <eq/fabric/frame.h>         // for Frame::Type
#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/pixel.h>         // member
#include <eq/fabric/range.h>         // member
#include <eq/fabric/subPixel.h>      // member

namespace eq
{
namespace fabric
{
/** @internal */
struct FrameData
{
    FrameData() : frameType( Frame::TYPE_MEMORY ), buffers( 0 ), period( 1 )
                , phase( 0 ) {}

    PixelViewport pvp;
    Frame::Type   frameType;
    uint32_t      buffers;
    uint32_t      period;
    uint32_t      phase;
    Range         range;     //<! database-range of src wrt to dest
    Pixel         pixel;     //<! pixel decomposition of source
    SubPixel      subpixel;  //<! subpixel decomposition of source
    Zoom          zoom;

    EQFABRIC_API void serialize( co::DataOStream& os ) const;
    EQFABRIC_API void deserialize( co::DataIStream& is );
};

}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::FrameData& value )
{
    byteswap( value.pvp );
    byteswap( value.frameType );
    byteswap( value.buffers );
    byteswap( value.period );
    byteswap( value.phase );
    byteswap( value.range );
    byteswap( value.pixel );
    byteswap( value.subpixel );
    byteswap( value.zoom );
}
}

#endif // EQFABRIC_FRAMEDATA_H

