
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAME_H
#define EQ_FRAME_H

#include <eq/net/object.h>
#include <eq/client/viewport.h>

namespace eqs
{
    class Frame;
}

namespace eq
{
    /**
     * A holder for a FrameBuffer and frame parameters.
     */
    class Frame : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new Frame.
         */
        Frame( const void* data, uint64_t dataSize );

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Return this frame's viewport.
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

        std::string _name;

        /** All distributed Data */
        friend class eqs::Frame;
        struct Data
        {
            eq::Viewport vp;
            uint32_t     bufferID;
            uint32_t     bufferVersion;
        }
            _data;
    };
};
#endif // EQS_FRAME_H
