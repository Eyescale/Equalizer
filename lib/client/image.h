
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/client/frame.h>         // for Frame::Buffer enum
#include <eq/client/pixelViewport.h> // member
#include <eq/client/viewport.h>      // member
#include <eq/client/windowSystem.h>  // for OpenGL types
#include <eq/base/nonCopyable.h>     // base class of nested class

namespace eq
{
    /**
     * A holder for pixel data.
     * 
     * An image holds color and depth information for a rectangular region.
     */
    class EQ_EXPORT Image
    {
    public:
        /** Constructs a new Image. */
        Image();
        virtual ~Image();
        
        /**
         * @name Data Access
         */
        //*{
        /** @return the fractional viewport of the image. */
        const eq::Viewport& getViewport() const { return _data.vp; }

		/** @return the GL function table, valid during readback. */
		GLEWContext* glewGetContext() { return _glObjects->glewGetContext(); }

        /** Reset the image to its default state. */
        void reset();

        /** 
         * Set the (OpenGL) format of the pixel data for a buffer.
         * Invalidates the pixel data.
         *
         * @param buffer the buffer type.
         * @param format the format.
         */
        void setFormat( const Frame::Buffer buffer, const uint32_t format );

        /** 
         * Set the (OpenGL) type of the pixel data for a buffer.
         * Invalidates the pixel data.
         *
         * @param buffer the buffer type.
         * @param type the type.
         */
        void setType( const Frame::Buffer buffer, const uint32_t type );

        /** @return the (OpenGL) format of the pixel data. */
        uint32_t getFormat( const Frame::Buffer buffer ) const;

        /** @return the (OpenGL) type of the pixel data. */
        uint32_t getType( const Frame::Buffer buffer ) const;

        /** @return the size of a single image pixel in bytes. */
        uint32_t getDepth( const Frame::Buffer buffer ) const;

        /** @return a pointer to the raw pixel data. */
        const uint8_t* getPixelData( const Frame::Buffer buffer ) const
            { EQASSERT(hasPixelData(buffer)); return _getPixels( buffer ).data;}
        uint8_t* getPixelData( const Frame::Buffer buffer )
            { EQASSERT(hasPixelData(buffer)); return _getPixels( buffer ).data;}

        /** @return the size of the raw pixel data in bytes */
        uint32_t getPixelDataSize( const Frame::Buffer buffer ) const
            { return (_pvp.w * _pvp.h * getDepth( buffer )); }
            
        /** @return a pointer to compressed pixel data. */
        const uint8_t* compressPixelData( const Frame::Buffer buffer,
                                          uint32_t& size );

        /** 
         * @return true if the image has pixel data for the buffer, false if
         * not.
         */
        bool hasPixelData( const Frame::Buffer buffer ) const 
            { return _getPixels( buffer ).valid; }
            
        /** @return the pixel viewport of the image with in the frame buffer. */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * Set the pixel viewport of the image buffers.
         *
         * The image buffers will be invalidated.
         * 
         * @param pvp the pixel viewport.
         */
        void setPixelViewport( const PixelViewport& pvp );

        /** 
         * Clear (zero-initialize) and validate an image buffer.
         * 
         * @param buffer the image buffer to clear.
         */
        void clearPixelData( const Frame::Buffer buffer );

        /** 
         * Set the pixel data of one of the image buffers.
         *
         * The data is copied, and previous data for the buffer is overwritten.
         * 
         * @param buffer the image buffer to set.
         * @param data the buffer data of size pvp.w * pvp.h * depth.
         */
        void setPixelData( const Frame::Buffer buffer, const uint8_t* data );

        /** 
         * Decompress and set the pixel data of one of the image buffers.
         *
         * Previous data for the buffer is overwritten.
         * 
         * @param buffer the image buffer to set
         * @param data the buffer data decompressing to getPixelDataSize()
         * @return the number of bytes read from the input data.
         */
        uint32_t decompressPixelData( const Frame::Buffer buffer, 
                                      const uint8_t* data );
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start reading back an image from the frame buffer.
         *
         * @param buffers bit-wise combination of the frame buffer components.
         * @param pvp the area of the frame buffer wrt the drawable.
		 * @param glObjects the GL object manager for the current GL context.
         */
        void startReadback( const uint32_t buffers, const PixelViewport& pvp,
			                Window::ObjectManager* glObjects );

        /** Make sure that the last readback operation is complete. */
        void syncReadback();
        
        /** Writes the pixel data as rgb image files. */
        void writeImage( const std::string& filename, 
                         const Frame::Buffer buffer ) const;
        
        /** Writes all pixel data as separate images. */
        void writeImages( const std::string& filenameTemplate ) const;

        /** Read pixel data from an uncompressed rgb image file. */
        bool readImage(const std::string& filename, const Frame::Buffer buffer);
        //*}
        
    private:
        /** All distributed data. */
        struct Data
        {
            Viewport vp;
        }
        _data;
        
        /** The rectangle of the current pixels data. */
        PixelViewport _pvp;

        /** 
         * Raw image data.
         * Previous implementations used a std::vector, but resizing it took
         * about 20ms for typical image sizes.
         */
        class Pixels : public eqBase::NonCopyable
        {
        public:
            Pixels() : data(0), format( GL_FALSE ), type( GL_FALSE ),
                       maxSize(0), valid( false ), reading( false )
                {}
            ~Pixels();

            void resize( uint32_t size );

            uint8_t* data;    // allocated (and cached data)
            uint32_t format;
            uint32_t type;
            uint32_t maxSize; // the size of the allocation
            bool     valid;   // data is currently valid
            bool     reading; // data is currently read
        };

        Pixels _colorPixels;
        Pixels _depthPixels;

        class CompressedPixels : public Pixels
        {
        public:
            uint32_t size; // current size of the compressed data
        };

        CompressedPixels _compressedColorPixels;
        CompressedPixels _compressedDepthPixels;

		/** The GL object manager, valid during a readback operation. */
		Window::ObjectManager* _glObjects;

        Pixels&           _getPixels( const Frame::Buffer buffer );
        CompressedPixels& _getCompressedPixels( const Frame::Buffer buffer );
        const Pixels&           _getPixels( const Frame::Buffer buffer ) const;
        const CompressedPixels& _getCompressedPixels( const Frame::Buffer
                                                      buffer ) const;

        uint32_t _compressPixelData( const uint64_t* data, const uint32_t size, 
                                     const uint64_t marker, uint64_t* out );

        /** @return a unique key for the frame buffer attachment. */
        const void* _getPBOKey( const Frame::Buffer buffer ) const;

        void _startReadback( const Frame::Buffer buffer );
        void _syncReadback( const Frame::Buffer buffer );

        void _setupAssemble( const vmml::Vector2i& offset );
        void _startAssemble2D( const vmml::Vector2i& offset );
        void _startAssembleDB( const vmml::Vector2i& offset );

        friend std::ostream& operator << ( std::ostream& os, const Image* );
    };
    std::ostream& operator << ( std::ostream& os, const Image* image );
};
#endif // EQ_IMAGE_H
