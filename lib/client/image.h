
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
        //*}

    private:
        /** All distributed data. */
        struct Data
        {
            Viewport             vp;
        }
            _data;

        /** Raw image data. */
        std::vector<uint8_t> _pixels;

        /** The image frame buffer type. */
        const Frame::Format  _format;
    };
};
#endif // EQ_IMAGE_H
