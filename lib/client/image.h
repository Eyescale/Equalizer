
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
        //*}

        /**
         * @name Operations
         */
        //*{
        void startReadback( const PixelViewport& pvp );
        
        /** Writes the pixel data as an .rgb image file. */
        void writeImage( const std::string& filename );
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
