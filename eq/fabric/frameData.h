
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
#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/pixel.h>         // member
#include <eq/fabric/range.h>         // member
#include <eq/fabric/subPixel.h>      // member

namespace eq
{
namespace fabric
{
class FrameData
{
public:
    FrameData() : _frameType( Frame::TYPE_MEMORY ), _buffers( 0 ), _period( 1 )
                , _phase( 0 ) {}

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

    /** Set the source range wrt dest channel. */
    void setRange( const Range& range ) { _range = range; }

    /** @return the source range wrt dest channel. */
    const Range& getRange() const { return _range; }

    /** Set the source pixel decomposition wrt dest channel. */
    void setPixel( const Pixel& pixel ) { _pixel = pixel; }

    /** @return the source pixel decomposition wrt dest channel. */
    const Pixel& getPixel() const { return _pixel; }

    /** Set the source pixel decomposition wrt dest channel. */
    void setSubPixel( const SubPixel& subpixel ) { _subpixel = subpixel; }

    /** @return the source pixel decomposition wrt dest channel. */
    const SubPixel& getSubPixel() const { return _subpixel; }

    /** Set the source DPlex period wrt dest channel. */
    void setPeriod( const uint32_t period ) { _period = period; }

    /** @return the source DPlex period wrt dest channel. */
    uint32_t getPeriod() const { return _period; }

    /** Set the source DPlex phase wrt dest channel. */
    void setPhase( const uint32_t phase ) { _phase = phase; }

    /** @return the source DPlex phase wrt dest channel. */
    uint32_t getPhase() const { return _phase; }

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
    Range         _range;     //<! database-range of src wrt to dest
    Pixel         _pixel;     //<! pixel decomposition of source
    SubPixel      _subpixel;  //<! subpixel decomposition of source
    Zoom          _zoom;
    Frame::Type   _frameType;
    uint32_t      _buffers;
    uint32_t      _period;
    uint32_t      _phase;
    template< class T > friend void lunchbox::byteswap( T& );
};

}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::FrameData& value )
{
    byteswap( value._pvp );
    byteswap( value._frameType );
    byteswap( value._buffers );
    byteswap( value._period );
    byteswap( value._phase );
    byteswap( value._range );
    byteswap( value._pixel );
    byteswap( value._subpixel );
    byteswap( value._zoom );
}
}

#endif // EQFABRIC_FRAMEDATA_H
