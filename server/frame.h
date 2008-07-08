
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_FRAME_H
#define EQSERVER_FRAME_H

#include "compound.h"

#include <eq/client/frame.h>
#include <eq/client/packets.h>

namespace eq
{
namespace server
{
    class Compound;
    class FrameData;
    class Node;

    /**
     * A holder for frame data and parameters.
     */
    class Frame : public net::Object
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
        Compound* getCompound() const { return _compound; }
        Channel* getChannel() const 
            { return _compound ? _compound->getChannel() :NULL; }
        Node* getNode() const { return _compound ? _compound->getNode() :NULL; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        FrameData* getMasterData() const { return _masterFrameData; }
        bool       hasData( const eq::Eye eye ) const
            { return ( _frameData[eye] != 0 ); }

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
         * relative data position wrt to the current destination channel of
         * the source.
         */
        void setOffset( const vmml::Vector2i& offset ) { _data.offset = offset;}
        
        /** @return the frame offset. */
        const vmml::Vector2i& getOffset() const { return _data.offset; }

        /** 
         * Set the frame buffers to be read or write by this frame.
         * 
         * @param buffers a bitwise combination of the buffers.
         */
        void setBuffers( const uint32_t buffers )
            { _data.buffers = buffers; }
        
        /** @return the frame buffers used by this frame. */
        uint32_t getBuffers() const { return _data.buffers; }
        uint32_t getInheritBuffers() const { return _inherit.buffers; }
        //*}

        /**
         * @name Operations
         */
        //*{
        /** Commit the frame's data (output frames only) */
        void commitData();

        /** 
         * Update the inherited, absolute data of this frame.
         * 
         * @param compound The compound from which the frame inherits.
         */
        void updateInheritData( const Compound* compound );

        /** 
         * Cycle the current FrameData.
         * 
         * Used for output frames to allocate/recycle frame data. Also
         * clears the list of input frames.
         *
         * @param frameNumber the current frame number.
         * @param eyes the eye passes used.
         */
        void cycleData( const uint32_t frameNumber, const uint32_t eyes );

        /** 
         * Add an input frame to this (output) frame
         * 
         * @param frame the input frame.
         * @param eyes the eye passes used.
         */
        void addInputFrame( Frame* frame, const uint32_t eyes );
        /** @return the vector of current input frames. */
        const FrameVector& getInputFrames( const eq::Eye eye ) const
            { return _inputFrames[eye]; }

        /** Unset the frame data. */
        void unsetData();

        /** Reset the frame and delete all frame datas. */
        void flush();
        //*}

    protected:
        virtual ~Frame();
        virtual ChangeType getChangeType() const { return INSTANCE; }

    private:
        /** The parent compound. */
        Compound* _compound;
        friend class Compound;

        /** The name which associates input to output frames. */
        std::string _name;

        /** Frame-specific data. */
        eq::Frame::Data _data;
        eq::Viewport    _vp;

        /** The current, actual data used by the frame. */
        eq::Frame::Data _inherit;

        /** All framedatas ever allocated, used for recycling old datas. */
        std::deque<FrameData*> _datas;
        
        /** Current frame data. */
        FrameData* _masterFrameData;
        FrameData* _frameData[eq::EYE_ALL];

        /** Vector of current input frames. */
        FrameVector _inputFrames[eq::EYE_ALL];
    };

    std::ostream& operator << ( std::ostream& os, const Frame* frame );
}
}
#endif // EQSERVER_FRAME_H
