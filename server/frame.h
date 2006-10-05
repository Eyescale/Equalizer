
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_FRAME_H
#define EQS_FRAME_H

#include <eq/client/frame.h>
#include <eq/client/packets.h>

namespace eqs
{
    class Compound;
    class FrameBuffer;

    /**
     * A holder for a FrameBuffer and frame parameters.
     */
    class Frame : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new Frame.
         */
        Frame();
        Frame( const Frame& from );

        /**
         * @name Data Access
         */
        //*{
        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        FrameBuffer* getBuffer() const { return _buffer; }

        /** 
         * Set the frame's viewport wrt the compound (output frames) or wrt the
         * corresponding output frame (input frames).
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const eq::Viewport& vp ) { _vp = vp; }
        
        /** @return the fractional viewport. */
        const eq::Viewport& getViewport() const { return _vp; }

        /** 
         * Set the offset of the frame.
         *
         * The offset is computed during compound update. The offset defines
         * relative buffer position wrt to the current destination channel of
         * the source.
         */
        void setOffset( const vmml::Vector2i& offset ) { _data.offset = offset;}
        
        /** @return the frame offset. */
        const vmml::Vector2i& getOffset() const { return _data.offset; }

        /** 
         * Set the frame buffer types to be read or write by this frame.
         * 
         * @param format a bitwise combination of the frame buffer formats.
         */
        void setFormat( const eq::Frame::Format format )
            { _data.format = format; }
        
        /** @return the frame buffer parts used by this frame. */
        eq::Frame::Format getFormat() const { return _data.format; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /** 
         * Update the inherited, absolute data of this frame.
         * 
         * @param compound The compound from which the frame inherits.
         */
        void updateInheritData( const Compound* compound );

        /** 
         * Cycle the current FrameBuffer.
         * 
         * Used for output frames.
         *
         * @param frameNumber the current frame number.
         * @param frameNumber the maximum age before frame buffers can be
         *                    recycled.
         */
        void cycleBuffer( const uint32_t frameNumber, 
                               const uint32_t maxAge );

        /** 
         * Set the output frame for this (input) frame.
         * 
         * @param frame the corresponding output frame.
         */
        void setOutputFrame( Frame* frame );

        /** Unset the frame buffer. */
        void unsetBuffer() { _buffer = NULL; }

        /** Reset the frame and delete all frame buffers. */
        void flush();
        //*}

    protected:
        virtual ~Frame();

        /** @sa eqNet::Object::pack */
        virtual const void* pack( uint64_t* size );

    private:
        std::string _name;

        /** Frame-specific data. */
        eq::Frame::Data _data;
        eq::Viewport    _vp;

        /** The current, actual data used by the frame. */
        eq::Frame::Data _inherit;

        /** All framebuffers ever allocated, used for recycling old buffers. */
        std::list<FrameBuffer*> _buffers;
        
        /** Current frame buffer. */
        FrameBuffer* _buffer;
    };

    std::ostream& operator << ( std::ostream& os, const Frame* frame );
};
#endif // EQS_FRAME_H
