
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_FRAMEDATA_H
#define EQSERVER_FRAMEDATA_H

#include <eq/client/frameData.h>

namespace eq
{
namespace server
{
    /**
     * A holder for a Frame Data and parameters.
     */
    class FrameData : public eq::net::Object
    {
    public:
        /** 
         * Constructs a new FrameData.
         */
        FrameData();

        /**
         * @name Data Access
         */
        //*{
        /** Set the number of the frame when this data was last used. */
        void setFrameNumber( const uint32_t number ) { _frameNumber = number; }
        uint32_t getFrameNumber() const { return _frameNumber; }

        /** Set the data's area within the channel */
        void setPixelViewport( const eq::PixelViewport& pvp ) 
            { _data.pvp = pvp; }

        /** Set the position of the data relative to the window. */
        void setOffset( const vmml::Vector2i& offset ) 
            { _data.offset = offset; }
        /** @return the position of the data relative to the window. */
        const vmml::Vector2i& getOffset() const { return _data.offset; }

        /** Set the (color, depth) buffers of the source frame. */
        void setBuffers( const uint32_t buffers ) 
            { _data.buffers = buffers; }
        uint32_t getBuffers() const { return _data.buffers; }

        /** Set the source range wrt dest channel. */
        void setRange( const eq::Range& range )
            { _data.range = range; }

        /** Set the source pixel decomposition wrt dest channel. */
        void setPixel( const eq::Pixel& pixel )
            { _data.pixel = pixel; }
        //*}

    protected:
        virtual ~FrameData(){}
        virtual ChangeType getChangeType() const { return INSTANCE; }

    private:
        friend class Frame;
        eq::FrameData::Data _data;
        
        /** The number of the config frame when this data was last used. */
        uint32_t _frameNumber;
    };
}
}
#endif // EQSERVER_FRAMEDATA_H
