
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

#ifndef EQSERVER_FRAMEDATA_H
#define EQSERVER_FRAMEDATA_H

#include <eq/frameData.h>
#include <eq/frame.h>      // for FrameType

namespace eq
{
namespace server
{
    /**
     * A holder for a Frame Data and parameters.
     */
    class FrameData : public co::Object
    {
    public:
        /** 
         * Constructs a new FrameData.
         */
        FrameData();

        /**
         * @name Data Access
         */
        //@{
        /** Set the number of the frame when this data was last used. */
        void setFrameNumber( const uint32_t number ) { _frameNumber = number; }
        uint32_t getFrameNumber() const { return _frameNumber; }

        /** Set the data's area within the channel */
        void setPixelViewport( const eq::PixelViewport& pvp ) 
            { _data.pvp = pvp; }

        /** Set the position of the data relative to the window. */
        void setOffset( const Vector2i& offset ) 
            { _offset = offset; }
        /** @return the position of the data relative to the window. */
        const Vector2i& getOffset() const { return _offset; }

        /** Set the (color, depth) buffers of the source frame. */
        void setBuffers( const uint32_t buffers ) 
            { _data.buffers = buffers; }
        uint32_t getBuffers() const { return _data.buffers; }

        /** Set the source range wrt dest channel. */
        void setRange( const eq::Range& range )
            { _data.range = range; }

        /** Set the source pixel decomposition wrt dest channel. */
        void setPixel( const eq::Pixel& pixel )
            { _data.pixel = pixel; }
        
        /** Set the source pixel decomposition wrt dest channel. */
        void setSubPixel( const eq::SubPixel& subpixel )
            { _data.subpixel = subpixel; }

        /** Set the source DPlex period wrt dest channel. */
        void setPeriod( const uint32_t period ) { _data.period = period; }

        /** Set the source DPlex phase wrt dest channel. */
        void setPhase( const uint32_t phase ) { _data.phase = phase; }

        /** Set the output frame zoom factor. */
        void setZoom( const eq::Zoom& zoom ) { _zoom = zoom; }
        const eq::Zoom& getZoom() const      { return _zoom; }

        /** return the frame storage type. */    
        eq::Frame::Type getType()const{ return _data.frameType; }

        /** 
         * Set the frame storage type.
         * 
         * @param type frame storage type.
         */
        void setType( const eq::Frame::Type type ){ _data.frameType = type; }

        //@}

    protected:
        virtual ~FrameData(){}
        virtual ChangeType getChangeType() const { return INSTANCE; }
        virtual void getInstanceData( co::DataOStream& os );
        virtual void applyInstanceData( co::DataIStream& is );

    private:
        friend class Frame;
        eq::FrameData::Data _data;
        
        /** The zoom factor of the output frame after readback. */
        eq::Zoom _zoom;

        /** Position wrt destination view. */
        Vector2i _offset;

        /** The number of the config frame when this data was last used. */
        uint32_t _frameNumber;
    };
}
}
#endif // EQSERVER_FRAMEDATA_H
