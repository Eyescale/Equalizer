
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/client/frame.h>
#include <eq/client/viewport.h>

namespace eq
{
    /**
     * A holder for pixel data.
     * 
     * An image holds color and depth information for a rectangular region.
     */
    class Image
    {
    public:
        /** Constructs a new Image. */
        Image(){}
        
        virtual ~Image();
        
        /**
         * @name Data Access
         */
        //*{
        /** @return the fractional viewport of the image. */
        const eq::Viewport& getViewport() const { return _data.vp; }

        /** @return the format of the pixel data. */
        uint32_t getFormat( const Frame::Buffer buffer ) const;

        /** @return the type of the pixel data. */
        uint32_t getType( const Frame::Buffer buffer ) const;

        /** @return the size of a single image pixel in bytes. */
        size_t getDepth( const Frame::Buffer buffer ) const;

        /** @return a pointer to the raw pixel data. */
        const std::vector<uint8_t>& getPixelData( const Frame::Buffer b ) const
            { return _pixels[ _getIndexForBuffer( b )]; }
        
        /** 
         * @return true if the image has pixel data for the buffer, false if
         * not.
         */
        bool hasPixelData( const Frame::Buffer buffer ) const 
            { return !(getPixelData( buffer ).empty()); }
            
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
        void setData( const Frame::Buffer buffer, const uint8_t* data );
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start reading back an image from the frame buffer.
         *
         * @param pvp the area of the frame buffer to read back.
         * @param buffers bit-wise combination of the frame buffer components.
         */
        void startReadback( const PixelViewport& pvp, const uint32_t buffers );

        /** Make sure that the last readback operation is complete. */
        void syncReadback() {}
        
        /** 
         * Start assemble the image into the frame buffer.
         *
         * @param offset the x,y offset wrt the current drawable.
         * @param buffers bit-wise combination of the frame buffer components.
         */
        void startAssemble( const vmml::Vector2i& offset,
                            const uint32_t buffers );

        /** Make sure that the last assemble operation is complete. */
        void syncAssemble() {}
        
        /** Writes the pixel data as rgb image files. */
        void writeImage( const std::string& filename, 
                         const Frame::Buffer buffer ) const;
        
        /** Writes all pixel data as separate images. */
        void writeImages( const std::string& filenameTemplate ) const;
        //*}

    private:
        enum BufferIndex
        {
            INDEX_COLOR,
            INDEX_DEPTH,
            INDEX_ALL
        };

        /** All distributed data. */
        struct Data
        {
            Viewport             vp;
        }
            _data;

        /** The rectangle of the current pixels data. */
        PixelViewport _pvp;

        /** Raw image data. */
        std::vector<uint8_t> _pixels[INDEX_ALL];

        static BufferIndex _getIndexForBuffer( const Frame::Buffer buffer );

        void _startReadback( const Frame::Buffer buffer );
        void _startAssemble2D( const vmml::Vector2i& offset );
        void _startAssembleDB( const vmml::Vector2i& offset );

        friend std::ostream& operator << ( std::ostream& os, const Image* );
    };
    std::ostream& operator << ( std::ostream& os, const Image* image );
};
#endif // EQ_IMAGE_H
