
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

#include <eq/client/frame.h>
#include <eq/client/viewport.h>

namespace eq
{
    /**
     * A holder for pixel data
     */
    class Image
    {
    public:
        /** Constructs a new Image. */
        Image( const Frame::Format format ) : _format( format ) {}
        
        virtual ~Image();
        
        /**
         * @name Data Access
         */
        //*{
        /** @return the fractional viewport of the image. */
        const eq::Viewport& getViewport() const { return _data.vp; }

        /** @return the format of the pixel data. */
        uint32_t getFormat() const;

        /** @return the type of the pixel data. */
        uint32_t getType() const;

        /** @return the size of a single image pixel in bytes. */
        size_t getDepth() const;

        /** @return a pointer to the raw pixel data. */
        const std::vector<uint8_t>& getPixelData() const { return _pixels; }

        /** @return the pixel viewport of the image with in the frame buffer. */
        const PixelViewport& getPixelViewport() const { return _pvp; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Start reading back an image from the frame buffer.
         *
         * @param pvp the area of the frame buffer to read back.
         */
        void startReadback( const PixelViewport& pvp );

        /** Make sure that the last readback operation is complete. */
        void syncReadback() {}
        
        /** Writes the pixel data as an .rgb image file. */
        void writeImage( const std::string& filename );
        
        /** 
         * Transmit the frame data to the specified node.
         *
         * Used internally after readback to push the image data to the input
         * frame nodes. Do not use directly.
         * 
         * @param toNode the receiving node.
         */
        void transmit( eqBase::RefPtr<eqNet::Node> toNode );
        //*}

    private:
        /** All distributed data. */
        struct Data
        {
            Viewport             vp;
        }
            _data;

        /** The rectangle of the current pixels data. */
        PixelViewport _pvp;

        /** Raw image data. */
        std::vector<uint8_t> _pixels;

        /** The image frame buffer type. */
        const Frame::Format  _format;

        friend std::ostream& operator << ( std::ostream& os, const Image* );
    };
    std::ostream& operator << ( std::ostream& os, const Image* image );
};
#endif // EQ_IMAGE_H
