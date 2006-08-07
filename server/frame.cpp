
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include <eq/client/packets.h>

using namespace eqs;

Frame::Frame()
        : eqNet::Object( eq::TYPE_FRAME, eqNet::CMD_OBJECT_CUSTOM )
{
}

        /** 
         * Constructs a new deep copy of a frame.
         */
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
        void setViewport( const eq::Viewport& vp );

        /** 
         * Return this frame's viewport.
         * 
         * @return the fractional viewport.
         */
        const eq::Viewport& getViewport() const { return _vp; }
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

        /** The fractional viewport wrt the compound. */
        eq::Viewport      _vp;
    };
};
#endif // EQS_FRAME_H
