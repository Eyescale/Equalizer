
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_FRAMEDATA_H
#define EQS_FRAMEDATA_H

#include <eq/client/frameData.h>

namespace eqs
{
    /**
     * A holder for a Frame Data and parameters.
     */
    class FrameData : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new FrameData.
         */
        FrameData();

        virtual uint32_t getTypeID() const { return eq::Object::TYPE_FRAMEDATA;}

        /**
         * @name Data Access
         */
        //*{
        /** Set the number of the frame when this data was last used. */
        void setFrameNumber( const uint32_t number ) { _frameNumber = number; }
        uint32_t getFrameNumber() const { return _frameNumber; }

        /** Set the datas area within the channel */
        void setPixelViewport( const eq::PixelViewport& pvp ) 
            { _data.pvp = pvp; }
        /** Set the position of the data relative to the window. */
        void setOffset( const vmml::Vector2i& offset ) 
            { _data.offset = offset; }

        /** Set the buffers of the source frame. */
        void setBuffers( const uint32_t buffers ) 
            { _data.buffers = buffers; }
        uint32_t getBuffers() const { return _data.buffers; }
        //*}

    protected:
        virtual ~FrameData(){}
        virtual bool isStatic() const { return false; }

    private:
        eq::FrameData::Data _data;
        
        /** The number of the config frame when this data was last used. */
        uint32_t _frameNumber;
    };
};
#endif // EQS_FRAMEDATA_H
