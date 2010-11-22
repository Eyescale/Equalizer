
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/client/frame.h>         // for Frame::Buffer enum
#include <eq/client/pixelData.h>     // member
#include <eq/client/windowSystem.h>  // for OpenGL types

#include <eq/fabric/pixelViewport.h> // member
#include <eq/fabric/viewport.h>      // member
#include <eq/util/texture.h>         // member
#include <eq/util/types.h>
#include <eq/base/buffer.h>          // member

#include <eq/plugins/compressor.h> // EqCompressorInfos typedef

namespace eq
{
    /**
     * A holder for pixel data.
     *
     * An image holds color and depth information for one rectangular region.
     */
    class Image
    {
    public:
        /** Construct a new Image. @version 1.0 */
        EQ_API Image();

        /** Destruct the Image. @version 1.0 */
        EQ_API virtual ~Image();

        /** @name Image parameters */
        //@{
        /** @return the format and type of the pixel data. */
        uint32_t getExternalFormat( const Frame::Buffer buffer ) const
            {  return _getMemory( buffer ).externalFormat; }
        
#if 0
        /**
         * @return the external OpenGL format of the pixel data, or 0 for
         *          external formats which are unknown to OpenGL.
         */
        uint32_t getGLFormat( const Frame::Buffer buffer ) const;
        
        /**
         * @return the external OpenGL type of the pixel data, or 0 for
         *          external formats which are unknown to OpenGL.
         */
        uint32_t getGLType( const Frame::Buffer buffer ) const;
#endif

        /**
         * Get the size, in bytes, of one pixel in pixel data.
         *
         * @param buffer the buffer type.
         */
        uint32_t getPixelSize( const Frame::Buffer buffer ) const
            { return _getMemory( buffer ).pixelSize; }

        /**
         * Set the internal GPU format of the pixel data for the given buffer.
         *
         * All internal formats need to have a corresponding OpenGL format with
         * the same value.
         *
         * @param buffer the buffer type.
         * @param internalFormat the internal format.
         */
        EQ_API void setInternalFormat( const Frame::Buffer buffer, 
                                          const uint32_t internalFormat );

        /** @return the internal GPU format of the pixel data. */
        EQ_API uint32_t getInternalFormat( const Frame::Buffer buffer )const;

        /** @return true if the image has a color buffer with alpha values. */
        EQ_API bool hasAlpha() const;

        /** 
         * Set the frame pixel storage type. 
         *
         * Images of storage type TYPE_MEMORY read back frame buffer data into
         * main memory using a transfer plugin. The data can be accessed through
         * the PixelData.
         *
         * Image of storage type TYPE_TEXTURE read frame buffer data into a
         * texture, which can be accessed using getTexture().
         */
        void setStorageType( const Frame::Type type ) { _type = type; }

        /** @return the pixel data storage type. */    
        Frame::Type getStorageType() const{ return _type; }

        /** @return true if the image buffer has valid data. */
        EQ_API bool hasData( const Frame::Buffer buffer ) const;

        /** @return the fractional viewport of the image. */
        //const eq::Viewport& getViewport() const { return _data.vp; }

        /**
         * Set the pixel viewport of the image.
         *
         * The image pixel data and textures will be invalidated.
         *
         * @param pvp the pixel viewport.
         */
        EQ_API void setPixelViewport( const PixelViewport& pvp );

        /** @return the pixel viewport of the image within 
          *         the frame buffer.
          */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** Reset the image to its default state. */
        EQ_API void reset();
        //@}

        /** @name Pixel data */
        //@{
        /** @return a pointer to the raw pixel data. */
        EQ_API const uint8_t* getPixelPointer( const Frame::Buffer buffer )
                                     const;
        EQ_API uint8_t* getPixelPointer( const Frame::Buffer buffer );

        /** @return the size of the raw pixel data in bytes */
        EQ_API uint32_t getPixelDataSize( const Frame::Buffer buffer ) const;

        /** @return the pixel data. */
        EQ_API const PixelData& getPixelData( const Frame::Buffer ) const;

        /** @return the compressed pixel data, compressing it if needed. */
        EQ_API const PixelData& compressPixelData( const Frame::Buffer );

        /**
         * @return true if the image has pixel data for the buffer, false if
         * not.
         */
        bool hasPixelData( const Frame::Buffer buffer ) const
            { return _getAttachment( buffer ).memory.state == Memory::VALID; }

        /**
         * Clear (zero-initialize) and validate an image buffer.
         *
         * @param buffer the image buffer to clear.
         */
        EQ_API void clearPixelData( const Frame::Buffer buffer );

        /** Validate an image buffer without initializing its content. */
        EQ_API void validatePixelData( const Frame::Buffer buffer );

        /**
         * Set the pixel data of one of the image buffers.
         *
         * Previous data for the buffer is overwritten. Validates the
         * buffer. Depending on the given PixelData parameters, the pixel data
         * is copied, decompressed or cleared.
         *
         * @param buffer the image buffer to set.
         * @param data the pixel data.
         */
        EQ_API void setPixelData( const Frame::Buffer buffer,
                                     const PixelData& data );

        /** Enable compression and transport of alpha data. */
        EQ_API void enableAlphaUsage();

        /** Disable compression and transport of alpha data. */
        EQ_API void disableAlphaUsage();

        /** Set the minimum quality after a full download-compression path. */
        EQ_API void setQuality( const Frame::Buffer buffer,
                                   const float quality );

        /** @return the minimum quality. */
        EQ_API float getQuality( const Frame::Buffer buffer ) const;

        /** @return true if alpha data can be ignored. */
        bool ignoreAlpha() const { return _ignoreAlpha; }
        //@}

        /** @name Texture access */
        //@{
        /** Get the texture of this image. */
        EQ_API const util::Texture& getTexture( const Frame::Buffer buffer )
            const;

        /**
         * @return true if the image has texture data for the buffer, false if
         * not.
         */
        EQ_API bool hasTextureData( const Frame::Buffer buffer ) const;

        /** 
         * @return the internal format a texture should use for the given
         *         buffer. 
         */
        EQ_API uint32_t getInternalTextureFormat( const Frame::Buffer which )
                               const;
        //@}

        /**
         * @name Operations
         */
        //@{
        /**
         * Read back an image from the frame buffer.
         *
         * @param buffers bit-wise combination of the Frame::Buffer components.
         * @param pvp the area of the frame buffer wrt the drawable.
         * @param zoom the scale factor to apply during readback.
         * @param glObjects the GL object manager for the current GL context.
         * @sa setStorageType()
         */
        EQ_API void readback( const uint32_t buffers,
                                 const PixelViewport& pvp, const Zoom& zoom,
                                 util::ObjectManager< const void* >* glObjects);

        /**
         * Read back an image from a given texture.
         *
         * If no texture is provided, the readback is performed from the
         * framebuffer.
         *
         * @param buffer the buffer type.
         * @param texture the OpenGL texture name.
         * @param glewContext function table for the current GL context.
         * @sa setStorageType()
         */
        void readback( const Frame::Buffer buffer, const util::Texture* texture,
                       const GLEWContext* glewContext );

        /**
         * Upload this image to the frame buffer.
         *
         * @param buffer the buffer type.
         * @param position the destination offset wrt current GL viewport.
         * @param glObjects the OpenGL object manager for the current context.
         */
        EQ_API void upload( const Frame::Buffer buffer,
                               const Vector2i& position,
                               util::ObjectManager< const void* >* glObjects )
            const;

        /** 
         * Upload this image to a texture.
         *
         * The texture will be initialized using the parameters corresponding to
         * the requested buffer.
         *
         * @param buffer the buffer type.
         * @param texture the target texture.
         * @param glObjects the OpenGL object manager for the current context.
         */
        EQ_API void upload( const Frame::Buffer buffer,
                               util::Texture* texture,
                               util::ObjectManager< const void* >* glObjects )
            const;

        /** Writes the pixel data as rgb image files. */
        EQ_API bool writeImage( const std::string& filename,
                                   const Frame::Buffer buffer ) const;

        /** Writes all valid pixel data as separate images. */
        EQ_API bool writeImages( const std::string& filenameTemplate ) const;

        /** Read pixel data from an uncompressed rgb image file. */
        EQ_API bool readImage( const std::string& filename, 
                                  const Frame::Buffer buffer   );

        /** Setting image offset, used after readback to correct position 
            if necessary */
        void setOffset( int32_t x, int32_t y ) { _pvp.x = x; _pvp.y = y; }

        /** Delete all cache data of this image. */
        EQ_API void flush();
        //@}

        /** 
         * @internal
         * @return the list of possible compressors for the given buffer.
         */
        EQ_API std::vector< uint32_t > 
        findCompressors( const Frame::Buffer buffer ) const;

        /** 
         * @internal
         * Assemble a list of possible up/downloaders for the given buffer.
         */
        EQ_API void findTransferers( const Frame::Buffer buffer,
                                        const GLEWContext* glewContext,
                                        std::vector< uint32_t >& names );
        
        /** @internal Re-allocate, if needed, a compressor instance. */
        EQ_API bool allocCompressor( const Frame::Buffer buffer, 
                                        const uint32_t name );

        /** @internal Re-allocate, if needed, a downloader instance. */
        EQ_API bool allocDownloader( const Frame::Buffer buffer, 
                                        const uint32_t name,
                                        const GLEWContext* glewContext );

    private:
        /** All distributed data. */
        struct Data
        {
            Viewport vp;
        } _data;

        /** The rectangle of the current pixel data. */
        PixelViewport _pvp;

        /**
         * Raw image data.
         */
        struct Memory : public PixelData
        {
        public:
            Memory() : state( INVALID ) {}

            void resize( const uint32_t size );
            void flush();            
            void useLocalBuffer();

            enum State
            {
                INVALID,
                VALID
            };

            State state;   //!< The current state of the memory

            /** During the call of setPixelData or writeImage, we have to 
                manage an internal buffer to copy the data */
            eq::base::Bufferb localBuffer;

            bool hasAlpha; //!< The uncompressed pixels contain alpha
        };

        /** @return an appropriate compressor name for the given buffer.*/
        uint32_t _chooseCompressor( const Frame::Buffer buffer ) const;

        /** The storage type for the pixel data. */
        Frame::Type _type;

        /** The individual parameters for a buffer. */
        class Attachment
        {
        public:
            Attachment();
            ~Attachment();

            void flush();
            base::CPUCompressor* const fullCompressor;
            base::CPUCompressor* const lossyCompressor;

            util::GPUCompressor* const fullTransfer;
            util::GPUCompressor* const lossyTransfer;

            base::CPUCompressor* compressor; //!< current CPU (de)compressor
            util::GPUCompressor* transfer;   //!< current up/download engine

            float quality; //!< the minimum quality

            /** The texture name for this image component (texture images). */
            util::Texture texture;

            /** Current pixel data (memory images). */
            Memory memory;
        };
        
        Attachment _color;
        Attachment _depth;

        EQ_API Attachment& _getAttachment( const Frame::Buffer buffer );
        EQ_API const Attachment& _getAttachment( const Frame::Buffer ) const;

        Memory& _getMemory( const Frame::Buffer buffer )
            { return _getAttachment( buffer ).memory; }
        const Memory& _getMemory( const Frame::Buffer buffer ) const
            { return  _getAttachment( buffer ).memory; }

        /** Find and activate a decompression engine */
        bool _allocDecompressor( Attachment& attachment, uint32_t name );

        void _findTransferers( const Frame::Buffer buffer,
                               const GLEWContext* glewContext,
                               base::CompressorInfos& result );

        /** Alpha channel significance. */
        bool _ignoreAlpha;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** @return a unique key for the frame buffer attachment. */
        const void* _getBufferKey( const Frame::Buffer buffer ) const;

        /** @return a unique key for the frame buffer attachment. */
        const void* _getCompressorKey( const Frame::Buffer buffer ) const;

        /**
         * Set the type of the pixel data in main memory for the given buffer.
         *
         * Invalidates the pixel data.
         *
         * @param buffer the buffer type.
         * @param externalFormat the type of the pixel.
         * @param pixelSize the size of one pixel in bytes.
         * @param hasAlpha true if the pixel data contains an alpha channel.
         */
        void _setExternalFormat( const Frame::Buffer buffer,
                                 const uint32_t externalFormat,
                                 const uint32_t pixelSize,
                                 const bool hasAlpha );

        void _readback( const Frame::Buffer buffer, const Zoom& zoom,
                        util::ObjectManager< const void* >* glObjects );
        void _readbackZoom( const Frame::Buffer buffer, const Zoom& zoom,
                            util::ObjectManager< const void* >* glObjects );

        friend std::ostream& operator << ( std::ostream& os, const Image* );
    };

    std::ostream& operator << ( std::ostream& os, const Image* image );
};
#endif // EQ_IMAGE_H
