
/* Copyright (c) 2006-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include <eq/client/api.h>
#include <eq/client/types.h>
#include <eq/client/zoomFilter.h> // enum
#include <eq/fabric/frame.h>   // base class

namespace eq
{
namespace detail{ class Frame; }

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
    class Frame : public fabric::Frame
    {
    public:
        /** Construct a new frame. @version 1.0 */
        EQ_API Frame();

        /** Destruct the frame. @version 1.0 */
        EQ_API virtual ~Frame();

        /** @name Data Access */
        //@{
        /** @return the name of the frame. @version 1.0 */
        EQ_API const std::string& getName() const;

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
         * Set the filter applied to zoomed assemble operations.
         * @version 1.0
         */
        EQ_API void setZoomFilter( const ZoomFilter zoomFilter );

        /**
         * @return the filter applied to zoomed assemble operations.
         * @version 1.0
         */
        EQ_API ZoomFilter getZoomFilter() const;

        /** @return all images of this frame. @version 1.0 */
        EQ_API const Images& getImages() const;

        /** Set the data for this frame. @version 1.3.2 */
        EQ_API void setFrameData( FrameDataPtr data );

        /** @return the frame's data. @version 1.3.2 */
        EQ_API FrameDataPtr getFrameData();

        /** @return the frame's data. @version 1.4 */
        EQ_API ConstFrameDataPtr getFrameData() const;

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
        EQ_API void setQuality( const Buffer buffer, const float quality );

        /** Sets a compressor for compression for following transmissions. */
        EQ_API void useCompressor( const Buffer buffer, const uint32_t name );
        //@}

        /** @name Operations */
        //@{
        /** @internal Recycle images attached  to the frame data. */
        EQ_API void clear();

        /** @internal Deallocate all data from the given object manager. */
        void deleteGLObjects( util::ObjectManager& om );

        /**
         * Read back an image.
         *
         * The image is added to the data, existing images are retained.
         *
         * @param glObjects the GL object manager for the current GL context.
         * @param config the configuration of the source frame buffer.
         * @version 1.0
         */
        EQ_API void readback( util::ObjectManager& glObjects,
                              const DrawableConfig& config );

        /**
         * Read back a set of images.
         *
         * The images are added to the data, existing images are retained.
         *
         * @param glObjects the GL object manager for the current GL context.
         * @param config the configuration of the source frame buffer.
         * @param regions the areas to read back.
         * @version 1.0
         */
        EQ_API void readback( util::ObjectManager& glObjects,
                              const DrawableConfig& config,
                              const PixelViewports& regions );

        /**
         * Start reading back a set of images for this frame.
         *
         * The newly read images are added to the data, existing images are
         * retained. The finish for the new images has to be done by the
         * caller. The regions are relative to the current OpenGL viewport.
         *
         * @param glObjects the GL object manager for the current GL context.
         * @param config the configuration of the source frame buffer.
         * @param regions the areas to read back.
         * @return the new images which need finishReadback.
         * @version 1.3.2
         */
        EQ_API Images startReadback( util::ObjectManager& glObjects,
                                     const DrawableConfig& config,
                                     const PixelViewports& regions );

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
        EQ_API void waitReady( const uint32_t timeout =
                               LB_TIMEOUT_INDEFINITE ) const;

        /**
         * Add a listener which will be incremented when the frame is ready.
         *
         * @param listener the listener.
         * @version 1.0
         */
        void addListener( lunchbox::Monitor<uint32_t>& listener );

        /**
         * Remove a frame listener.
         *
         * @param listener the listener.
         * @version 1.0
         */
        void removeListener( lunchbox::Monitor<uint32_t>& listener );
        //@}

    private:
        detail::Frame* const _impl;
    };
};
#endif // EQ_FRAME_H
