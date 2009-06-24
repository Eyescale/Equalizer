
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

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/client/frame.h>         // for Frame::Buffer enum
#include <eq/client/pixelViewport.h> // member
#include <eq/client/texture.h>       // member
#include <eq/client/viewport.h>      // member
#include <eq/client/windowSystem.h>  // for OpenGL types
#include <eq/plugin/compressor.h>

namespace eq
{
    class Compressor;
    /**
     * A holder for pixel data.
     *
     * An image holds color and depth information for a rectangular region.
     */
    class Image
    {
    public:
        /** Constructs a new Image. */
        EQ_EXPORT Image();
        EQ_EXPORT virtual ~Image();

        struct PixelData : public base::NonCopyable
        {
            PixelData() : format( GL_FALSE ), type( GL_FALSE )
                        , compressorName( EQ_COMPRESSOR_NONE )
                {}
            ~PixelData();
            void flush();

            uint32_t format;         //!< the GL format
            uint32_t type;           //!< the GL type
            uint32_t compressorName; //!< the compressor used
            
            base::Bufferb pixels;   //!< The pixel data
            
            // compressed Pixel data
            base::Buffer< uint64_t > lengthData;
            base::Buffer< void * >   outCompressed;

        private:
            friend class Image;
        };

        /** @name Data Access */
        //@{

        /** @name Image parameters */
        //@{
        /**
         * Set the (OpenGL) format of the pixel data for a buffer.
         * Invalidates the pixel data.
         *
         * @param buffer the buffer type.
         * @param format the format.
         */
        EQ_EXPORT void setFormat( const Frame::Buffer buffer,
                                  const uint32_t format );

        /** @return the (OpenGL) format of the pixel data. */
        EQ_EXPORT uint32_t getFormat( const Frame::Buffer buffer ) const;

        /**
         * Set the (OpenGL) type of the pixel data for a buffer.
         * Invalidates the pixel data.
         *
         * @param buffer the buffer type.
         * @param type the type.
         */
        EQ_EXPORT void setType( const Frame::Buffer buffer, 
                                const uint32_t type );

        /** @return the (OpenGL) type of the pixel data. */
        EQ_EXPORT uint32_t getType( const Frame::Buffer buffer ) const;

        /** @return the size of a single image pixel (format*type) in bytes. */
        EQ_EXPORT uint32_t getDepth( const Frame::Buffer buffer ) const;

        /** @return true if the image has a color buffer with alpha. */
        EQ_EXPORT bool hasAlpha() const;

        /** 
         * Set the frame pixel storage type. 
         *
         * Images of storage type TYPE_MEMORY read back frame buffer data into
         * main memory. The data can be accessed through the PixelData.
         *
         * Image of storage type TYPE_TEXTURE read frame buffer data into a
         * texture, which can be accessed using getTexture().
         */
        void setStorageType( const Frame::Type type) { _type = type; }

        /** @return the pixel data storage type. */    
        Frame::Type getStorageType() const{ return _type; }

        /** @return true if the image buffer has valid data. */
        EQ_EXPORT bool hasData( const Frame::Buffer buffer ) const;

        /** @return the fractional viewport of the image. */
        //const eq::Viewport& getViewport() const { return _data.vp; }

        /**
         * Set the pixel viewport of the image.
         *
         * The image pixel data and textures will be invalidated.
         *
         * @param pvp the pixel viewport.
         */
        EQ_EXPORT void setPixelViewport( const PixelViewport& pvp );

        /** @return the pixel viewport of the image with in the frame buffer. */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** Reset the image to its default state. */
        EQ_EXPORT void reset();
        //@}


        /** @name Pixel data */
        //@{
        /** @return a pointer to the raw pixel data. */
        EQ_EXPORT const uint8_t* getPixelPointer( const Frame::Buffer buffer )
                                     const;
        EQ_EXPORT uint8_t* getPixelPointer( const Frame::Buffer buffer );

        /** @return the size of the raw pixel data in bytes */
        uint32_t getPixelDataSize( const Frame::Buffer buffer ) const
            { return _pvp.getArea() * getDepth( buffer ); }

        /** @return the pixel data. */
        EQ_EXPORT const PixelData& getPixelData( const Frame::Buffer buffer )
                                       const;

        /** @return the compressed pixel data. */
        EQ_EXPORT const PixelData& compressPixelData( const Frame::Buffer 
                                                          buffer );

        /**
         * @return true if the image has pixel data for the buffer, false if
         * not.
         */
        bool hasPixelData( const Frame::Buffer buffer ) const
            { return _getPixels( buffer ).state == Pixels::VALID; }

        /**
         * Clear (zero-initialize) and validate an image buffer.
         *
         * @param buffer the image buffer to clear.
         */
        EQ_EXPORT void clearPixelData( const Frame::Buffer buffer );

        /** Validate an image buffer without initializing its content. */
        EQ_EXPORT void validatePixelData( const Frame::Buffer buffer );

        /**
         * Set the pixel data of one of the image buffers.
         *
         * The data is copied, and previous data for the buffer is
         * overwritten. The pixel data is validated.
         *
         * @param buffer the image buffer to set.
         * @param data the buffer data of size pvp.w * pvp.h * depth.
         */
        EQ_EXPORT void setPixelData( const Frame::Buffer buffer, 
                                     const uint8_t* data );

        /**
         * Set the pixel data of one of the image buffers.
         *
         * Previous data for the buffer is overwritten. The pixel data is
         * validated and decompressed , if needed.
         *
         * @param buffer the image buffer to set.
         * @param data the pixel data.
         */
        EQ_EXPORT void setPixelData( const Frame::Buffer buffer,
                                     const PixelData& data );

        /** Switch PBO usage for image transfers on or off. */
        void setPBO( const bool onOff ) { _usePBO = onOff; }

        /** @return if this image should use PBO for image transfers. */
        bool getPBO() const             { return _usePBO; }
        //@}


        /** @name Texture access */
        //@{
        /** Get the texture of this image. */
        EQ_EXPORT const Texture& getTexture( const Frame::Buffer buffer ) const;

        /**
         * @return true if the image has texture data for the buffer, false if
         * not.
         */
        EQ_EXPORT bool hasTextureData( const Frame::Buffer buffer ) const;

        /** 
         * @return the internal format a texture should use for the given
         *         buffer. 
         */
        EQ_EXPORT uint32_t getInternalTextureFormat( const Frame::Buffer which )
                               const;
        //@}


        /**
         * @name Operations
         */
        //@{
        /**
         * Start reading back an image from the frame buffer.
         *
         * @param buffers bit-wise combination of the frame buffer components.
         * @param pvp the area of the frame buffer wrt the drawable.
         * @param zoom the scale factor to apply during readback.
         * @param glObjects the GL object manager for the current GL context.
         * @sa setStorageType()
         */
        EQ_EXPORT void startReadback( const uint32_t buffers, 
                                      const PixelViewport& pvp,
                                      const Zoom& zoom,
                                      Window::ObjectManager* glObjects );

        /** Make sure that the last readback operation is complete. */
        EQ_EXPORT void syncReadback();

        /** Writes the pixel data as rgb image files. */
        EQ_EXPORT void writeImage( const std::string& filename,
                                   const Frame::Buffer buffer ) const;

        /** Writes all valid pixel data as separate images. */
        EQ_EXPORT void writeImages( const std::string& filenameTemplate, 
                                    const Frame::Buffer buffer ) const;

        EQ_EXPORT void writeImages( const std::string& filenameTemplate ) const;

        /** Read pixel data from an uncompressed rgb image file. */
        EQ_EXPORT bool readImage( const std::string& filename, 
                                  const Frame::Buffer buffer   );

        /** Setting image offset, used after readback to correct position 
            if necessary */
        void setOffset( int32_t x, int32_t y ) { _pvp.x = x; _pvp.y = y; }

        /** Delete all cache data of this image. */
        void flush();
        //@}

        /** @return the GL function table, valid during readback. */
        GLEWContext* glewGetContext() { return _glObjects->glewGetContext(); }

        /** @return the number of channels in a pixel. */
        EQ_EXPORT uint8_t getNumChannels( const Frame::Buffer buffer ) const;

        /** @return the size in bytes for one channel. */
        EQ_EXPORT uint8_t getChannelSize( const Frame::Buffer buffer ) const;

    private:
        /** All distributed data. */
        struct Data
        {
            Viewport vp;
        } _data;

        /** The rectangle of the current pixels data. */
        PixelViewport _pvp;

        /**
         * Raw image data.
         */
        class Pixels : public base::NonCopyable
        {
        public:
            /** The current state of the pixels */
            enum State
            {
                INVALID,
                PBO_READBACK,
                ZOOM_READBACK,
                VALID
            };

            Pixels() : pboSize(0), state( INVALID ){}

            void resize( uint32_t size );
            void flush();

            PixelData data;
            uint32_t  pboSize; // the size of the PBO
            State     state;   // current state
        };


        class CompressedPixels
        {
        public:
            void flush();

            PixelData data;
            bool valid;   // data is currently valid
        };


        /** @return an apropriate compressor name for the buffer data type.*/
        uint32_t _getCompressorName( const Frame::Buffer buffer );

        /** The GL object manager, valid during a readback operation. */
        Window::ObjectManager* _glObjects;

        /** The storage type for the pixel data. */
        Frame::Type _type;

        /** The individual parameters for a buffer. */
        class Attachment
        {
        public:  
            
            Attachment ()
                    : compressorName( EQ_COMPRESSOR_NONE )
                    , compressor(0)
                    , plugin(0)
                {}

            /** The name of compressor used to compress/uncompress pixel data.*/
            uint32_t compressorName;

            /** The compressor instance. */
            void* compressor;

            /** Compression plugin handle used to allocate instances */
            Compressor* plugin;

            /** The color for this component image. */
            Texture texture;

            /** Current pixel data. */
            Pixels pixels;

            /** compressed pixel data. */
            CompressedPixels compressedPixels;
        }; 
        
        Attachment _color;
        Attachment _depth;

        Attachment& _getAttachment( const Frame::Buffer buffer );

        /** Find and activate a compressor / uncompressor engine */
        void _allocCompressor( const Frame::Buffer buffer,
                               uint32_t compressorName );


        /** PBO Usage. */
        bool _usePBO;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        Pixels& _getPixels( const Frame::Buffer buffer );
        CompressedPixels& _getCompressedPixels( const Frame::Buffer buffer );
        const Pixels&           _getPixels( const Frame::Buffer buffer ) const;
        const CompressedPixels& _getCompressedPixels( const Frame::Buffer
                                                      buffer ) const;
        Texture& _getTexture( const Frame::Buffer buffer );

        /** @return a unique key for the frame buffer attachment. */
        const void* _getBufferKey( const Frame::Buffer buffer ) const;

        void _startReadback( const Frame::Buffer buffer, const Zoom& zoom );
        void _startReadbackPBO( const Frame::Buffer buffer );
        void _startReadbackZoom( const Frame::Buffer buffer, const Zoom& zoom );
                                
        void _syncReadback( const Frame::Buffer buffer );
        void _syncReadbackPBO( const Frame::Buffer buffer );
        void _syncReadbackZoom( const Frame::Buffer buffer );

        friend std::ostream& operator << ( std::ostream& os, const Image* );
    };
    std::ostream& operator << ( std::ostream& os, const Image* image );
};
#endif // EQ_IMAGE_H
