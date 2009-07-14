
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/client/frame.h>         // enum Frame::Buffer
#include <eq/client/pixelViewport.h> // member
#include <eq/client/pixel.h>         // member
#include <eq/client/range.h>         // member
#include <eq/client/types.h>         // member

#include <eq/base/monitor.h>         // member
#include <eq/net/object.h>           // base class

#include <set>                       // member

namespace eq
{

namespace server
{
    class FrameData;
}

    class  Image;
    class  ROIFinder;
    struct FrameDataTransmitPacket;

    /**
     * A frame data holds multiple images and is used by frames.
     * It is not intended to be used directly by application code.
     */
    class FrameData : public net::Object
    {
    public:
        EQ_EXPORT FrameData();
        EQ_EXPORT virtual ~FrameData();

        /** 
         * @name Data Access
         */
        //@{
        /** The enabled frame buffer attachments. */
        uint32_t getBuffers() const { return _data.buffers; }
        void     setBuffers( const uint32_t buffers ){ _data.buffers = buffers;}

        /** The database-range relative to the destination channel. */
        const Range& getRange() const { return _data.range; }
        void setRange( const Range& range ) { _data.range = range; }
        
        /** The pixel decomposition relative to the destination channel. */
        const Pixel& getPixel() const { return _data.pixel; }
        
        /** The images of this frame data holder */
        const ImageVector& getImages() const { return _images; }

        /** The covered area. */
        void setPixelViewport( const PixelViewport& pvp ) { _data.pvp = pvp; }

        /* Set color buffer type to read */
        void setColorType( const GLuint colorType ) { _colorType = colorType; }
        
        /** Enable/disable alpha usage for newly allocated images. */
        void setAlphaUsage( const bool useAlpha ) { _useAlpha = useAlpha; }
        //@}

        /**
         * @name Operations
         */
        //@{

        /** Flush the frame by deleting all images. */
        void flush();

        /** 
         * Allocate and add a new image.
         * 
         * @return the image.
         */
        EQ_EXPORT Image* newImage( const Frame::Type type = Frame::TYPE_MEMORY);

        /** Clear the frame by recycling the attached images. */
        EQ_EXPORT void clear();

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The newly read images are added to the data, existing images are
         * retained.
         *
         * @param frame the corresponding output frame holder.
         * @param glObjects the GL object manager for the current GL context.
         */
        void startReadback( const Frame& frame, 
                            Window::ObjectManager* glObjects );

        /** Synchronize the last image readback. */
        void syncReadback();

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
         * Set the frame data ready.
         * 
         * The frame data is automatically set ready by syncReadback
         * and upon receiving of the transmit commands.
         */
        void setReady();

        /** @return true if the frame data is ready, false if not. */
        bool isReady() const   { return _readyVersion >= getVersion(); }

        /** Wait for the frame data to become available. */
        void waitReady() const { _readyVersion.waitGE( getVersion( )); }

        /** 
         * Add a listener which will be incremented when the frame is
         * ready.
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
        
        /** 
         * Disable the usage of a frame buffer attachment for all images.
         * 
         * @param buffer the buffer to disable.
         */
        void disableBuffer( const Frame::Buffer buffer )
            { _data.buffers &= ~buffer; }
 
        /** @internal */
        void useSendToken( const bool use ) { _useSendToken = use; }
        //@}

        /** @warning internal use only. */
        void update( const uint32_t version );

    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }
        virtual void getInstanceData( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is );

        /** @sa net::Object::attachToSession */
        virtual void attachToSession( const uint32_t id, 
                                      const uint32_t instanceID, 
                                      net::Session* session );

    private:
        struct Data
        {
            Data() : offset( Vector2i::ZERO ), buffers( 0 ), format( 0 )
                   , type( 0 ), frameType( Frame::TYPE_MEMORY ){}

            PixelViewport pvp;
            Vector2i      offset;
            uint32_t      buffers;
            uint32_t      format;
            uint32_t      type;
            Frame::Type   frameType;
            Range         range;     //<! database-range of src wrt to dest
            Pixel         pixel;     //<! pixel decomposition of source
        } _data;

        friend class eq::server::FrameData;

        ImageVector  _images;
        ImageVector  _imageCache;
        base::Lock   _imageCacheLock;

        GLuint     _colorType; 
        ROIFinder* _roiFinder;

        struct ImageVersion
        {
            ImageVersion( Image* _image, const uint32_t _version )
                    : image( _image ), version( _version ) {}

            Image*   image;
            uint32_t version;
        };
        std::list<ImageVersion> _pendingImages;
        std::set< uint32_t >    _readyVersions;

        /** Data ready monitor synchronization primitive. */
        base::Monitor<uint32_t> _readyVersion;

        /** External monitors for readyness synchronization. */
        std::vector< base::Monitor<uint32_t>* > _listeners;
        base::Lock                              _listenersMutex;

        bool _useAlpha;
        bool _useSendToken;
        
        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** Allocate or reuse an image. */
        Image* _allocImage( const eq::Frame::Type type );

        /** Apply all received images of the given version. */
        void _applyVersion( const uint32_t version );

        /** Set a specific version ready. */
        void _setReady( const uint32_t version );

        /* The command handlers. */
        net::CommandResult _cmdTransmit( net::Command& command );
        net::CommandResult _cmdReady( net::Command& command );
        net::CommandResult _cmdUpdate( net::Command& command );

        CHECK_THREAD_DECLARE( _commandThread );
    };
    std::ostream& operator << ( std::ostream& os, const FrameData* data );
}

#endif // EQ_FRAMEDATA_H

