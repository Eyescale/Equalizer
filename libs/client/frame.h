
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_FRAME_H
#define EQ_FRAME_H

#include <eq/eye.h>    // enum Eye

#include <eq/api.h>
#include <eq/types.h>
#include <eq/zoomFilter.h>   // member

#include <eq/fabric/zoom.h>   // member
#include <co/object.h>
#include <co/objectVersion.h>
#include <co/types.h>
#include <co/base/bitOperation.h> // function getIndexOfLastBit
#include <co/base/monitor.h>

namespace eq
{
namespace util
{
    template< typename T > class ObjectManager;
}
namespace server
{
    class Frame;
}
    class FrameData;
    class Image;
    class Pipe;

    /**
     * A holder for a frame data and related parameters.
     *
     * A Frame has to be set up with a FrameData which ultimately holds the
     * pixel data using images. Input and output frames provided by the
     * eq::Channel are set up correctly with FrameData to connect one output
     * frame to one ore more input frames. Some Frame parameters are
     * (input-)frame-specific, while others are set by the output frame on its
     * data and therefore shared between all input frames.
     */
    class Frame : public co::Object
    {
    public:
        /** 
         * The buffer format defines which components of the frame are to
         * be used during readback and assembly.
         * @version 1.0
         */
        enum Buffer
        {
            BUFFER_NONE      = EQ_BIT_NONE,
            BUFFER_UNDEFINED = EQ_BIT1,  //!< Inherit, only if no others are set
            BUFFER_COLOR     = EQ_BIT5,  //!< Use color images
            BUFFER_DEPTH     = EQ_BIT9,  //!< Use depth images
            BUFFER_ALL       = EQ_BIT_ALL
        };

        /** The storage type for pixel data. @version 1.0 */
        enum Type
        {
            TYPE_MEMORY,    //!< use main memory to store pixel data
            TYPE_TEXTURE    //!< use a GL texture to store pixel data
        };

        /** Construct a new frame. @version 1.0 */
        EQ_API Frame();

        /** Destruct the frame. @version 1.0 */
        EQ_API virtual ~Frame();

        /** @name Data Access */
        //@{
        /** @return the name of the frame. @version 1.0 */
        EQ_API const std::string& getName() const;

        /** @return the position of the frame wrt the channel. @version 1.0 */
        const Vector2i& getOffset() const { return _data.offset; }

        /**
         * Set the position of the frame wrt the channel.
         *
         * The offset is only applied for operations on this frame holder, i.e.,
         * it does not apply to other (input) frames using the same underlying
         * frame data.
         * @version 1.0
         */
        void setOffset( const Vector2i& offset ) { _data.offset = offset;}

        /**
         * @return the database range relative to the destination channel.
         * @version 1.0
         */
        EQ_API const Range& getRange() const;

        /**
         * @return the pixel parameters relative to the destination channel.
         * @version 1.0
         */
        EQ_API const Pixel& getPixel() const;

        /**
         * @return the subpixel parameters wrt the destination channel.
         * @version 1.0
         */
        EQ_API const SubPixel& getSubPixel() const;

        /**
         * @return the DPlex period relative to the destination channel.
         * @version 1.0
         */
        EQ_API uint32_t getPeriod() const;

        /**
         * @return the DPlex phase relative to the destination channel.
         * @version 1.0
         */
        EQ_API uint32_t getPhase() const;

        /**
         * Set the zoom for this frame holder.
         *
         * The zoom is only applied for operations on this frame holder, i.e.,
         * it does not apply to other (input) frames using the same underlying
         * frame data.
         * @version 1.0
         */
        void setZoom( const Zoom& zoom ) { _data.zoom = zoom; }

        /** @return the zoom factor for readback or assemble. @version 1.0 */
        const Zoom& getZoom() const { return _data.zoom; }

        /** 
         * Set the filter applied to zoomed assemble operations.
         * @version 1.0
         */
        void setZoomFilter( const ZoomFilter zoomFilter )
            { _zoomFilter = zoomFilter; }

        /**
         * @return the filter applied to zoomed assemble operations.
         * @version 1.0
         */
        ZoomFilter getZoomFilter() const { return _zoomFilter; }

        /** @return all images of this frame. @version 1.0 */
        EQ_API const Images& getImages() const;

        /** Set the data for this frame. @version 1.0 */
        void setData( FrameData* data ) { _frameData = data; }

        /** @return the frame's data. @version 1.0 */
        FrameData* getData() { return _frameData; }

        /** @return the frame's data. @version 1.0 */
        const FrameData* getData() const { return _frameData; }

        /** @return the enabled frame buffer attachments. @version 1.0 */
        EQ_API uint32_t getBuffers() const;

        /** 
         * Disable the usage of a frame buffer attachment for all images.
         * 
         * @param buffer the buffer to disable.
         * @version 1.0
         */
        EQ_API void disableBuffer( const Buffer buffer );

        /** Set alpha usage for newly allocated images. @version 1.0 */
        EQ_API void setAlphaUsage( const bool useAlpha );

        /** Set the minimum quality after compression. @version 1.0 */
        EQ_API void setQuality( const Frame::Buffer buffer,
                                   const float quality );

        /** @internal */
        const co::ObjectVersion& getDataVersion( const Eye eye ) const
            { return _data.frameData[co::base::getIndexOfLastBit( eye ) ]; }
        //@}

        /** @name Operations */
        //@{
        /** @internal Recycle images attached  to the frame data. */
        EQ_API void clear();

        /** @internal Clear and free all images attached to the frame data. */
        void flush();

        /**
         * Read back a set of images according to the current frame data.
         * 
         * The images are added to the data, existing images are retained.
         *
         * @param glObjects the GL object manager for the current GL context.
         * @param config the configuration of the source frame buffer.
         * @version 1.0
         */
        EQ_API void readback( util::ObjectManager< const void* >* glObjects,
                                 const DrawableConfig& config );

        /**
         * Set the frame ready.
         * 
         * Equalizer sets the frame automatically ready after readback and after
         * network transmission.
         * @version 1.0
         */
        void setReady();

        /** 
         * Test the readiness of the frame.
         * 
         * @return true if the frame is ready, false if not. 
         * @version 1.0
         */
        EQ_API bool isReady() const;

        /** Wait for the frame to become available. @version 1.0 */
        EQ_API void waitReady() const;

        /** 
         * Add a listener which will be incremented when the frame is ready.
         * 
         * @param listener the listener.
         * @version 1.0
         */
        void addListener( co::base::Monitor<uint32_t>& listener );

        /** 
         * Remove a frame listener.
         * 
         * @param listener the listener.
         * @version 1.0
         */
        void removeListener( co::base::Monitor<uint32_t>& listener );
        //@}

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }
        virtual void getInstanceData( co::DataOStream& os );
        virtual void applyInstanceData( co::DataIStream& is );

    private:
        std::string _name;
        FrameData*  _frameData;

        ZoomFilter _zoomFilter; // texture filter

        /** The distributed data shared between Frame and server::Frame. */
        friend class eq::server::Frame;
        struct Data
        {
            Data() : offset( Vector2i::ZERO ) {}

            Vector2i offset;
            Zoom zoom;
            co::ObjectVersion frameData[ NUM_EYES ];
        }
        _data;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes
    };

    /** Print the frame type to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const Frame::Type );
    /** Print the frame buffer value to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream&, const Frame::Buffer );
};
#endif // EQ_FRAME_H
