
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

#ifndef EQ_FRAME_H
#define EQ_FRAME_H

#include <eq/client/eye.h>    // enum Eye
#include <eq/client/window.h> // nested ObjectManager type
#include <eq/client/types.h>  // member
#include <eq/client/zoom.h>   // member

#include <eq/base/monitor.h>
#include <eq/net/object.h>

namespace eq
{

namespace server
{
    class Frame;
}

    class FrameData;
    class Image;
    class Pipe;
    class Pixel;
    class Range;

    /**
     * A holder for a frame data and parameters.
     */
    class Frame : public net::Object
    {
    public:
        /** 
         * The buffer format defines which components of the frame are to
         * be used during recomposition.
         */
        enum Buffer
        {
            BUFFER_NONE      = EQ_BIT_NONE,
            BUFFER_UNDEFINED = EQ_BIT1,  //!< Inherit, only if no others are set
            BUFFER_COLOR     = EQ_BIT5,  //!< Use color images
            BUFFER_DEPTH     = EQ_BIT9,  //!< Use depth images
            BUFFER_ALL       = EQ_BIT_ALL
        };

        /** The storage type for pixel data. */
        enum Type
        {
            TYPE_MEMORY,    //!< use main memory to store pixel data
            TYPE_TEXTURE    //!< use a GL texture to store pixel data
        };

        /** 
         * Constructs a new Frame.
         */
        EQ_EXPORT Frame();
        EQ_EXPORT virtual ~Frame();

        /**
         * @name Data Access
         */
        //@{
        EQ_EXPORT const std::string& getName() const;

        const Vector2i& getOffset() const { return _data.offset; }
        void setOffset( const Vector2i& offset ) { _data.offset = offset;}

        /** @return the database-range relative to the destination channel. */
        EQ_EXPORT const Range& getRange() const;
        EQ_EXPORT void         setRange( const Range& range );

        /** @return the pixel parameters relative to the destination channel. */
        EQ_EXPORT const Pixel& getPixel() const;

        /** @return the subpixel parameters wrt the destination channel. */
        EQ_EXPORT const SubPixel& getSubPixel() const;

        /** @return the DPlex period relative to the destination channel. */
        EQ_EXPORT uint32_t getPeriod() const;

        /** @return the DPlex phase relative to the destination channel. */
        EQ_EXPORT uint32_t getPhase() const;

        /** @return the zoom factor for readback or assemble. */
        const Zoom& getZoom() const { return _data.zoom; }

        /** Set zoom, used for frames created by user; normally zoom is set by 
            server implicitly and not through this function */
        void setZoom( const Zoom& zoom ) { _data.zoom = zoom; }

        /** The images of this frame */
        EQ_EXPORT const ImageVector& getImages() const;

        /** Set the data for this frame. */
        void setData( FrameData* data ) { _frameData = data; }
        FrameData* getData() { return _frameData; }

        /** Set the pixel viewport of the frame's data */
        EQ_EXPORT void setPixelViewport( const PixelViewport& pvp );

        const net::ObjectVersion& getDataVersion( const Eye eye ) const
            { return _data.frameData[ eye ]; }
        //@}

        /**
         * @name Operations
         */
        //@{

        /** Clear the frame, recycles the images attached to the frame data. */
        EQ_EXPORT void clear();

        /** Clear the frame and free all images attached to the frame data. */
        void flush();

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The images are added to the frame, existing images are retained.
         *
         * @param glObjects the GL object manager for the current GL context.
         * @param config the configuration of the source frame buffer.
         */
        EQ_EXPORT void startReadback( Window::ObjectManager* glObjects,
                                      const DrawableConfig& config );
        
        /** Synchronize the image readback. */
        EQ_EXPORT void syncReadback();

        /** 
         * Transmit the frame data to the specified node.
         *
         * Used internally after readback to push the image data to the input
         * frame nodes. Do not use directly.
         * 
         * @param toNode the receiving node.
         * @param frameNumber the current frame number
         */        
        void transmit( net::NodePtr toNode, const uint32_t frameNumber );

        /** 
         * Set the frame ready.
         * 
         * The frame is automatically set ready by syncReadback and upon
         * receiving of the transmit commands.
         */
        void setReady();

        /** 
         * Test the readiness of the frame.
         * 
         * The readiness of the frame is automatically managed by the frame
         * buffer readback and transmit implementation.
         * 
         * @return true if the frame is ready, false if not. 
         */
        EQ_EXPORT bool isReady() const;

        /** Wait for the frame to become available. */
        EQ_EXPORT void waitReady() const;

        /** 
         * Add a listener which will be incremented when the frame is ready.
         * 
         * @param listener the listener.
         */
        void addListener( base::Monitor<uint32_t>& listener );

        /** 
         * Remove a frame listener.
         * 
         * @param listener the listener.
         */
        void removeListener( base::Monitor<uint32_t>& listener );
        //@}

        /** @return the enabled frame buffer attachments. */
        EQ_EXPORT uint32_t getBuffers() const;

        /** 
         * Disable the usage of a frame buffer attachment for all images.
         * 
         * @param buffer the buffer to disable.
         */
        EQ_EXPORT void disableBuffer( const Buffer buffer );

        /** Enable/disable alpha usage for newly allocated images. */
        EQ_EXPORT void setAlphaUsage( const bool useAlpha );

        /** Set the compressor quality value. */
        EQ_EXPORT void setQuality( const Frame::Buffer buffer, 
                                   const float quality );

        /** @internal */
        void useSendToken( const bool use );

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }
        virtual void getInstanceData( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is );

    private:
        std::string _name;
        FrameData*  _frameData;

        /** The distributed data shared between Frame and server::Frame. */
        friend class eq::server::Frame;
        struct Data
        {
            Data() : offset( Vector2i::ZERO ), zoom( 0.f, 0.f ) {}

            Vector2i     offset;
            Zoom               zoom;
            net::ObjectVersion frameData[EYE_ALL];
        }
        _data;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Frame::Type );
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Frame::Buffer );
};
#endif // EQ_FRAME_H
