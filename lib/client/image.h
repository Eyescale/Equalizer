
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_IMAGE_H
#define EQ_IMAGE_H

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
        Image() {}
        
        virtual ~Image() {}
        
        /**
         * @name Data Access
         */
        //*{
        /** 
         * Return this image's viewport.
         *
         * @return the fractional viewport.
         */
        const eq::Viewport& getViewport() const { return _data.vp; }
        //*}

        /**
         * @name Operations
         */
        //*{
        //*}

    private:
        /** All distributed data. */
        struct Data
        {
            Viewport             vp;
        }
            _data;
    };
};
#endif // EQ_IMAGE_H
