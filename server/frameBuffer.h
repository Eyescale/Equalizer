
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_FRAMEBUFFER_H
#define EQS_FRAMEBUFFER_H

#include <eq/client/frameBuffer.h>

namespace eqs
{
    /**
     * A holder for a FrameBufferBuffer and frameBuffer parameters.
     */
    class FrameBuffer : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new FrameBuffer.
         */
        FrameBuffer();

        /**
         * @name Data Access
         */
        //*{
        /** Set the number of the frame when this buffer was last used. */
        void setFrameNumber( const uint32_t number ) { _frameNumber = number; }
        uint32_t getFrameNumber() const { return _frameNumber; }
        //*}

        /**
         * @name Operations
         */
        //*{
        //*}

    private:
        eq::FrameBuffer::Data _data;
        
        /** The number of the config frame when this buffer was last used. */
        uint32_t _frameNumber;
    };
};
#endif // EQS_FRAMEBUFFER_H
