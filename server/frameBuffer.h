
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

        /** Set the buffers area within the channel */
        void setPixelViewport( const eq::PixelViewport& pvp ) 
            { _data.pvp = pvp; }
        /** Set the position of the buffer relative to the window. */
        void setOffset( const vmml::Vector2i& offset ) 
            { _data.offset = offset; }
        /** Set the format of the source frame. */
        void setFormat( const eq::Frame::Format format ) 
            { _data.format = format; }
        //*}

    private:
        eq::FrameBuffer::Data _data;
        
        /** The number of the config frame when this buffer was last used. */
        uint32_t _frameNumber;
    };
};
#endif // EQS_FRAMEBUFFER_H
