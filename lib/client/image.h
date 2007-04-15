
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/client/frame.h>        // for Frame::Buffer enum
#include <eq/client/viewport.h>     // member
#include <eq/client/windowSystem.h> // for OpenGL types
#include <eq/base/nonCopyable.h>    // base class of nested class

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
            { return _getPixels( buffer ).data; }
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
         * The image buffers will be resized to match the new pixel viewport.
         * 
         * @param pvp the pixel viewport.
         */
        void setPixelViewport( const PixelViewport& pvp );

        /** 
         * Set the pixel data of one of the image buffers.
         *
         * The data is copied, and previous data for the buffer is overwritten.
         * 
         * @param buffer the image buffer to set
         * @param data the buffer data of size pvp.w * pvp.h * depth
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
         */
        void startReadback( const uint32_t buffers, const PixelViewport& pvp );

        /** Make sure that the last readback operation is complete. */
        void syncReadback() {}
        
        /** 
         * Start assemble the image into the frame buffer.
         *
         * @param buffers bit-wise combination of the frame buffer components.
         * @param offset the x,y offset wrt the current drawable.
         */
        void startAssemble( const uint32_t buffers,
                            const vmml::Vector2i& offset);

        /** Make sure that the last assemble operation is complete. */
        void syncAssemble() {}
        
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
            Pixels() : data(0), maxSize(0), format( GL_FALSE ),
                       type( GL_FALSE ), valid( false )
                {}
            ~Pixels() { delete [] data; }

            void resize( const uint32_t size );

            uint8_t* data;    // allocated (and cached data)
            uint32_t maxSize; // the size of the allocation
            uint32_t format;
            uint32_t type;
            bool     valid;   // data is currently valid
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

        Pixels&           _getPixels( const Frame::Buffer buffer );
        CompressedPixels& _getCompressedPixels( const Frame::Buffer buffer );
        const Pixels&           _getPixels( const Frame::Buffer buffer ) const;
        const CompressedPixels& _getCompressedPixels( const Frame::Buffer
                                                      buffer ) const;

        void _startReadback( const Frame::Buffer buffer );
        void _startAssemble2D( const vmml::Vector2i& offset );
        void _startAssembleDB( const vmml::Vector2i& offset );

        friend std::ostream& operator << ( std::ostream& os, const Image* );
    };
    std::ostream& operator << ( std::ostream& os, const Image* image );
};
#endif // EQ_IMAGE_H
