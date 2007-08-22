
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VOL_IMAGE_H
#define EQ_VOL_IMAGE_H

#include <eq/eq.h>     // base class of nested class

namespace eqVol
{
    /**
     * A holder for pixel data.
     * 
     * An image holds color and depth information for a rectangular region.
     */
    class EQ_EXPORT Image : public eq::Image
    {
    public:
        /** 
         * Start assemble the image into the frame buffer.
         *
         * @param buffers bit-wise combination of the frame buffer components.
         * @param offset the x,y offset wrt the current drawable.
         */
        void startAssemble( const uint32_t buffers,
                            const vmml::Vector2i& offset);

};
#endif // EQ_VOL_IMAGE_H
