
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_FRAME_H
#define EQS_FRAME_H

#include <eq/net/object.h>

namespace eqs
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
        Frame();

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
        void setViewport( const eq::Viewport& vp ) { _data.vp = vp; }
        
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
        /** 
         * Update the inherited, absolute data of this frame.
         * 
         * @param compound The compound from which the frame inherits.
         */
        void updateInheritData( const Compound* compound );

        /** 
         * Cycle the current FrameBuffer.
         * 
         * @param frameNumber the current frame number.
         * @param frameNumber the maximum age before frame buffers can be
         *                    recycled.
         */
        void cycleFrameBuffer( const uint32_t frameNumber, 
                               const uint32_t maxAge );
        //*}

    private:

        std::string _name;

        /** All distributed Data */
        // FIXME: move to eq::Frame
        struct Data
        {
            /** The fractional viewport wrt the compound. */
            eq::Viewport vp;
        }
            _data;
    };
};
#endif // EQS_FRAME_H
