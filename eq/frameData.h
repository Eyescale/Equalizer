
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

#ifndef EQ_FRAMEDATA_H
#define EQ_FRAMEDATA_H

#include <eq/frame.h>         // enum Frame::Buffer
#include <eq/types.h>
#include <eq/fabric/frameData.h> // base class
#include <co/object.h>               // base class
#include <lunchbox/monitor.h>        // member
#include <lunchbox/spinLock.h>       // member

namespace eq
{
namespace detail { class FrameData; }

/**
 * A holder for multiple images.
 *
 * The FrameData is used to connect the Image data for multiple frames.
 * Equalizer uses the same frame data for all input and output frames of the
 * same name. This enables frame-specific parameters to be set on the Frame, and
 * generic parameters (of the output frame) to be set on the FrameData, as well
 * as ready synchronization of the pixel data.
 *
 * An application may allocate its own Frame and FrameData for
 * application-specific purposes.
 *
 * Parameters set on an Equalizer output frame data are automatically
 * transported to the corresponding input frames.
 */
class FrameData : public fabric::FrameData, public co::Object,
                  public lunchbox::Referenced
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
    /** The images of this frame data holder. @version 1.0 */
    EQ_API const Images& getImages() const;

    /**
     * Set alpha usage for newly allocated images.
     *
     * Disabling alpha allows the selection of download or compression
     * plugins which drop the alpha channel for better performance.
     * @version 1.0
     */
    EQ_API void setAlphaUsage( const bool useAlpha );

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

    /**
     * Sets a compressor which will be allocated and used during transmit of
     * the image buffer. The default compressor is EQ_COMPRESSOR_AUTO which
     * selects the most suitable compressor wrt the current image and buffer
     * parameters.
     * @sa _chooseCompressor()
     *
     * @param buffer the frame buffer attachment.
     * @param name the compressor name.
     */
    void useCompressor( const Frame::Buffer buffer, const uint32_t name );
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

    /** Clear the frame by recycling the attached images. @version 1.0 */
    EQ_API void clear();

    /** Flush the frame by deleting all images. @version 1.0 */
    void flush();

    /** Delete data allocated by the given object manager on all images.*/
    void deleteGLObjects( util::ObjectManager& om );

    /** Deallocate all transfer and compression plugins on all images. */
    EQ_API void resetPlugins();

#ifndef EQ_2_0_API
    /**
     * Read back an image for this frame data.
     *
     * The newly read image is added to the data using newImage(). Existing
     * images are retained.
     *
     * @param frame the corresponding output frame holder.
     * @param glObjects the GL object manager for the current GL context.
     * @param config the configuration of the source frame buffer.
     * @version 1.0
     * @deprecated @sa startReadback()
     */
    void readback( const Frame& frame, util::ObjectManager& glObjects,
                   const DrawableConfig& config );
#endif

    /**
     * Start reading back a set of images for this frame data.
     *
     * The newly read images are added to the data using
     * newImage(). Existing images are retained.
     *
     * @param frame the corresponding output frame holder.
     * @param glObjects the GL object manager for the current GL context.
     * @param config the configuration of the source frame buffer.
     * @param regions the areas to read back.
     * @return the new images which need finishReadback.
     * @version 1.3.0
     */
    Images startReadback( const Frame& frame,
                          util::ObjectManager& glObjects,
                          const DrawableConfig& config,
                          const PixelViewports& regions );

    /**
     * Set the frame data ready.
     *
     * The frame data is automatically set ready by readback() and after
     * receiving an output frame.
     * @version 1.0
     */
    void setReady();

    /** @return true if the frame data is ready. @version 1.0 */
    EQ_API bool isReady() const;

    /** Wait for the frame data to become available. @version 1.0 */
    EQ_API void waitReady( const uint32_t timeout = LB_TIMEOUT_INDEFINITE )
        const;

    /** @internal */
    void setVersion( const uint64_t version );

    typedef lunchbox::Monitor< uint32_t > Listener; //!< Ready listener

    /**
     * Add a ready listener.
     *
     * The listener value will will be incremented when the frame is ready,
     * which might happen immediately.
     *
     * @param listener the listener.
     * @version 1.0
     */
    void addListener( Listener& listener );

    /**
     * Remove a frame listener.
     *
     * @param listener the listener.
     * @version 1.0
     */
    void removeListener( Listener& listener );
    //@}

    /** @internal */
    bool addImage( const co::ObjectVersion& frameDataVersion,
                   const PixelViewport& pvp, const Zoom& zoom,
                   const uint32_t buffers, const bool useAlpha,
                   uint8_t* data );
    void setReady( const co::ObjectVersion& frameData,
                   const fabric::FrameData& data ); //!< @internal

protected:
    virtual ChangeType getChangeType() const { return INSTANCE; }
    virtual void getInstanceData( co::DataOStream& os );
    virtual void applyInstanceData( co::DataIStream& is );

private:
    detail::FrameData* const _impl;

    /** Allocate or reuse an image. */
    Image* _allocImage( const Frame::Type type,
                        const DrawableConfig& config,
                        const bool setQuality );

    /** Apply all received images of the given version. */
    void _applyVersion( const uint128_t& version );

    /** Set a specific version ready. */
    void _setReady( const uint64_t version );

    LB_TS_VAR( _commandThread );
};

/** Print the frame data to the given output stream. @version 1.4 */
EQ_API std::ostream& operator << ( std::ostream&, const FrameData& );
}

#endif // EQ_FRAMEDATA_H
