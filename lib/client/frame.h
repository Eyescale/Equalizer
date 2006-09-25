
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
    class FrameBuffer;

    /**
     * A holder for a FrameBuffer and frame parameters.
     */
    class Frame : public eqNet::Object
    {
    public:
        /** 
         * The frame format defines which components of the framebuffer are to
         * be used during recomposition.
         */
        enum Format
        {
            FORMAT_NONE      = 0,
            FORMAT_UNDEFINED = 0x1,      //!< Inherit, only if no others are set
            FORMAT_COLOR     = 0x10,     //!< Use color images
            FORMAT_DEPTH     = 0x10000,  //!< Use depth images
            FORMAT_ALL       = 0xfffffff
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
        void clear();

        /** 
         * Read back a set of images according to the current frame data.
         * 
         * The images are added to the frame, existing images are retained.
         */
        void startReadback();
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
            Format               format;
            eqNet::ObjectVersion buffer;
        }
            _data;
        friend class eqs::Frame;

        FrameBuffer* _buffer;

        FrameBuffer* _getBuffer();
    };
};
#endif // EQ_FRAME_H
