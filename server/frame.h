
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

        /** 
         * Set the frame's viewport wrt the compound.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const eq::Viewport& vp ) { _inherit.vp = vp; }
        
        /** 
         * Return this frame's viewport.
         * 
         * @return the fractional viewport.
         */
        const eq::Viewport& getViewport() const { return _inherit.vp; }
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
        void cycleFrameBuffer( const uint32_t frameNumber, 
                               const uint32_t maxAge );

        /** 
         * Set the output frame for this (input) frame.
         * 
         * @param frame the corresponding output frame.
         */
        void setOutputFrame( Frame* frame );

        /** Unset the frame buffer. */
        void unsetFrameBuffer() { _setFrameBuffer( NULL ); }

        /** Reset the frame and delete all frame buffers. */
        void flush();
        //*}

    protected:
        virtual ~Frame();

    private:
        std::string _name;

        /** All distributed data */
        eq::Frame::Data _inherit;

        /** All framebuffers ever allocated, used for recycling old buffers. */
        std::list<FrameBuffer*> _buffers;
        
        /** Current frame buffer. */
        FrameBuffer* _buffer;

        void _setFrameBuffer( FrameBuffer* buffer );
    };

    std::ostream& operator << ( std::ostream& os, const Frame* frame );
};
#endif // EQS_FRAME_H
