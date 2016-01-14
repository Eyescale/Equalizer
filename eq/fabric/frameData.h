
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include <eq/fabric/renderContext.h> // member

namespace eq
{
namespace fabric
{
class FrameData
{
public:
    FrameData() : _frameType( Frame::TYPE_MEMORY ), _buffers( 0 ) {}

    /**
     * Set the covered area for readbacks.
     *
     * Preset for Equalizer output frames. The given pixel viewport is used
     * together with the Frame offset to compute the area for the readback()
     * operation.
     */
    void setPixelViewport( const PixelViewport& pvp ) { _pvp = pvp; }

    /** @return the covered area for readbacks. */
    const PixelViewport& getPixelViewport() const { return _pvp; }

    /** Set the (color, depth) buffers of the source frame. */
    void setBuffers( const uint32_t buffers ) { _buffers = buffers; }

    /** Disable the usage of a frame buffer attachment for all images. */
    void disableBuffer( const Frame::Buffer buffer ) { _buffers &= ~buffer; }

    /** @return the (color, depth) buffers of the source frame. */
    uint32_t getBuffers() const { return _buffers; }

    /** Set the source context decomposition wrt dest channel. */
    void setContext( const RenderContext& context ) { _context = context; }

    /** @return the source context decomposition wrt dest channel. */
    const RenderContext& getContext() const { return _context; }
    RenderContext& getContext() { return _context; }

    /** Set additional zoom for input frames. */
    void setZoom( const Zoom& zoom ) { _zoom = zoom; }

    /** @return the additional zoom. */
    const Zoom& getZoom() const { return _zoom; }

    /**
     * Set the frame storage type.
     *
     * @param type frame storage type.
     */
    void setType( const fabric::Frame::Type type ) { _frameType = type; }

    /** @return the frame storage type. */
    Frame::Type getType() const { return _frameType; }

    EQFABRIC_API void serialize( co::DataOStream& os ) const;
    EQFABRIC_API void deserialize( co::DataIStream& is );

protected:
    PixelViewport _pvp;
    RenderContext _context; //<! source channel render context
    Zoom          _zoom;
    Frame::Type   _frameType;
    uint32_t      _buffers;
    template< class T > friend void lunchbox::byteswap( T& );
};

}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::FrameData& value )
{
    byteswap( value._pvp );
    byteswap( value._context );
    byteswap( value._zoom );
    byteswap( value._frameType );
    byteswap( value._buffers );
}
}

#endif // EQFABRIC_FRAMEDATA_H
