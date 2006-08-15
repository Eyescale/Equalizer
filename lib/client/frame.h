
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAME_H
#define EQ_FRAME_H

#include <eq/net/object.h>
#include <eq/client/viewport.h>
#include <eq/client/frameBuffer.h>

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
        enum Format
        {
            FORMAT_COLOR = 0x01,
            FORMAT_DEPTH = 0x02
        };

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

        /** Clear the frame by deleting the attached images. */
        void clear() { _getBuffer()->clear(); }

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The images are added to the frame, existing images are retained.
         */
        void startReadback()  { _getBuffer()->startReadback(); }
        //*}

    protected:
        /** @sa eqNet::Object::unpack */
        virtual void unpack( const void* data, const uint64_t size ) 
            { _buffer = NULL; eqNet::Object::unpack( data, size ); }

    private:
        std::string _name;

        /** All distributed Data shared between eq::Frame and eqs::Frame. */
        struct Data
        {
            Viewport             vp;
            eqNet::ObjectVersion buffer;
        }
            _data;
        friend class eqs::Frame;

        FrameBuffer* _buffer;

        FrameBuffer* _getBuffer();
    };
};
#endif // EQ_FRAME_H
