
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

#ifndef EQ_FRAMEDATA_H
#define EQ_FRAMEDATA_H

#include <eq/frame.h>         // enum Frame::Buffer
#include <eq/types.h>         // member

#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/pixel.h>         // member
#include <eq/fabric/range.h>         // member
#include <eq/fabric/subPixel.h>      // member
#include <co/object.h>           // base class
#include <co/base/monitor.h>         // member
#include <co/base/scopedMutex.h>     // member
#include <co/base/spinLock.h>        // member

#include <set>                       // member

namespace eq
{

namespace server
{
    class FrameData;
}
    class  ROIFinder;
    struct NodeFrameDataTransmitPacket;
    struct NodeFrameDataReadyPacket;

    /**
     * A holder for multiple images.
     *
     * The FrameData is used to connect the Image data for multiple frames.
     * Equalizer uses the same frame data for all input and output frames of the
     * same name. This enables frame-specific parameters to be set on the Frame,
     * and generic parameters (of the output frame) to be set on the FrameData,
     * as well as ready synchronization of the pixel data.
     *
     * An application may allocate its own Frame and FrameData for
     * application-specific purposes.
     *
     * Parameters set on an Equalizer output frame data are automatically
     * transported to the corresponding input frames.
     */
    class FrameData : public co::Object
    {
    public:
        void assembleFrame( Frame* frame, Channel* channel );
        struct ImageHeader
        {
            uint32_t                internalFormat;
            uint32_t                externalFormat;
            uint32_t                pixelSize;
            fabric::PixelViewport   pvp;
            uint32_t                compressorName;
            uint32_t                compressorFlags;
            uint32_t                nChunks;
            float                   quality;
        };

        /** Construct a new frame data holder. @version 1.0 */
        EQ_API FrameData();

        /** Destruct this frame data. @version 1.0 */
        EQ_API virtual ~FrameData();

        /** @name Data Access */
        //@{
        /** @return the enabled frame buffer attachments. @version 1.0 */
        uint32_t getBuffers() const { return _data.buffers; }

        /**
         * Set the enabled frame buffer attachments.
         *
         * The default buffers are set for Equalizer input and output frames
         * according to the configuration, or to 0 for application-created frame
         * datas.
         */
        void setBuffers( const uint32_t buffers ){ _data.buffers = buffers;}

        /**
         * Get the range.
         *
         * The range is set for Equalizer frames to the range of the frame data
         * relative to the destination channel.
         *
         * @return the database-range.
         * @version 1.0
         */
        const Range& getRange() const { return _data.range; }

        /** Set the range of this frame. @version 1.0 */
        void setRange( const Range& range ) { _data.range = range; }
        
        /**
         * @return the pixel decomposition wrt the destination channel.
         * @version 1.0
         */
        const Pixel& getPixel() const { return _data.pixel; }
        
        /**
         * @return the subpixel decomposition wrt the destination channel.
         * @version 1.0
         */
        const SubPixel& getSubPixel() const { return _data.subpixel; }

        /**
         * @return the DPlex period relative to the destination channel.
         * @version 1.0
         */
        uint32_t getPeriod() const { return _data.period; }

        /**
         * @return the DPlex phase relative to the destination channel.
         * @version 1.0
         */
        uint32_t getPhase() const { return _data.phase; }

        /** The images of this frame data holder. @version 1.0 */
        const Images& getImages() const { return _images; }

        /**
         * Set the covered area for readbacks.
         *
         * Preset for Equalizer output frames. The given pixel viewport is used
         * together with the Frame offset to compute the area for the readback()
         * operation.
         * @version 1.0
         */
        void setPixelViewport( const PixelViewport& pvp ) { _data.pvp = pvp; }
        
        /**
         * Set alpha usage for newly allocated images.
         *
         * Disabling alpha allows the selection of download or compression
         * plugins which drop the alpha channel for better performance.
         * @version 1.0
         */
        void setAlphaUsage( const bool useAlpha ) { _useAlpha = useAlpha; }

        /**
         * Set the minimum quality after download and compression.
         *
         * Setting a lower quality decreases the image quality while increasing
         * the performance of scalable rendering. An application typically
         * selects a lower quality during interaction. Setting a quality of 1.0
         * provides lossless image compositing.
         * @version 1.0
         */
        void setQuality( const Frame::Buffer buffer, const float quality );

        /** @internal Set additional zoom for input frames. */
        void setZoom( const Zoom& zoom ) { _data.zoom = zoom; }

        /** @internal @return the additional zoom. */
        const Zoom& getZoom() const { return _data.zoom; }
        //@}

        /** @name Operations */
        //@{
        /** 
         * Allocate and add a new image.
         *
         * The allocated image inherits the current quality, storage type from
         * this frame data and the internal format corresponding to the given
         * drawable config.
         *
         * @return the image.
         * @version 1.0
         */
        EQ_API Image* newImage( const Frame::Type type,
                                const DrawableConfig& config );

        /** Flush the frame by deleting all images. @version 1.0 */
        void flush();

        /** Clear the frame by recycling the attached images. @version 1.0 */
        EQ_API void clear();

        /** 
         * Read back an image for this frame data.
         * 
         * The newly read images are added to the data using
         * newImage(). Existing images are retained.
         *
         * @param frame the corresponding output frame holder.
         * @param glObjects the GL object manager for the current GL context.
         * @param config the configuration of the source frame buffer.
         * @version 1.0
         */
        void readback( const Frame& frame, 
                       util::ObjectManager< const void* >* glObjects,
                       const DrawableConfig& config );

        /**
         * Set the frame data ready.
         * 
         * The frame data is automatically set ready by readback() and after
         * receiving an output frame.
         * @version 1.0
         */
        void setReady();

        /** @return true if the frame data is ready. @version 1.0 */
        bool isReady() const   { return _readyVersion.get() >= _version; }

        /** Wait for the frame data to become available. @version 1.0 */
        void waitReady() const { _readyVersion.waitGE( _version ); }
        
        /** @internal */
        void setVersion( const uint64_t version );

        /** 
         * Add a ready listener.
         *
         * The listener value will will be incremented when the frame is ready,
         * which might happen immediately.
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
        
        /** 
         * Disable the usage of a frame buffer attachment for all images.
         * 
         * @param buffer the buffer to disable.
         * @version 1.0
         */
        void disableBuffer( const Frame::Buffer buffer )
            { _data.buffers &= ~buffer; }
 
        /** @internal */
        void setSendToken( const bool use ) { _useSendToken = use; }
        /** @internal */       
        bool getSendToken(){ return _useSendToken; }
        //@}

        /** @internal */
        bool addImage( const NodeFrameDataTransmitPacket* packet );
        void setReady( const NodeFrameDataReadyPacket* packet ); //!< @internal

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }
        virtual void getInstanceData( co::DataOStream& os );
        virtual void applyInstanceData( co::DataIStream& is );

    private:
        friend struct NodeFrameDataReadyPacket;
        struct Data
        {
            Data() : frameType( Frame::TYPE_MEMORY ), buffers( 0 ), period( 1 )
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
        } _data;

        friend class server::FrameData;

        Images _images;
        Images _imageCache;
        co::base::Lock _imageCacheLock;

        ROIFinder* _roiFinder;

        Images _pendingImages;

        uint64_t _version; //!< The current version

        typedef co::base::Monitor< uint64_t > Monitor;

        /** Data ready monitor synchronization primitive. */
        Monitor _readyVersion;

        typedef co::base::Monitor< uint32_t > Listener;
        typedef std::vector< Listener* > Listeners;
        /** External monitors for readiness synchronization. */
        co::base::Lockable< Listeners, co::base::SpinLock > _listeners;

        bool _useAlpha;
        bool _useSendToken;
        float _colorQuality;
        float _depthQuality;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** Allocate or reuse an image. */
        Image* _allocImage( const Frame::Type type,
                            const DrawableConfig& config );

        /** Apply all received images of the given version. */
        void _applyVersion( const uint128_t& version );

        /** Set a specific version ready. */
        void _setReady( const uint64_t version );

        EQ_TS_VAR( _commandThread );
    };

    /** Print the frame data to the given output stream. @version 1.0 */
    std::ostream& operator << ( std::ostream& os, const FrameData* data );
}

#endif // EQ_FRAMEDATA_H

