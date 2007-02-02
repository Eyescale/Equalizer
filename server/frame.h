
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_FRAME_H
#define EQS_FRAME_H

#include "compound.h"

#include <eq/client/frame.h>
#include <eq/client/packets.h>

namespace eqs
{
    class Compound;
    class FrameData;
    class Node;

    /**
     * A holder for frame data and parameters.
     */
    class Frame : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new Frame.
         */
        Frame();
        Frame( const Frame& from );

        virtual uint32_t getTypeID() const { return eq::Object::TYPE_FRAME; }

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

        FrameData* getData() const { return _frameData; }
        bool       hasData() const { return ( _frameData != 0 ); }

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
         */
        void cycleData( const uint32_t frameNumber );

        /** 
         * Add an input frame to this (output) frame
         * 
         * @param frame the input frame.
         */
        void addInputFrame( Frame* frame );
        /** @return the vector of current input frames. */
        const std::vector<Frame*> getInputFrames() const { return _inputFrames;}

        /** Unset the frame data. */
        void unsetData() { _frameData = NULL; }

        /** Reset the frame and delete all frame datas. */
        void flush();
        //*}

    protected:
        virtual ~Frame();
        virtual bool isStatic() const { return false; }

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
        std::list<FrameData*> _datas;
        
        /** Current frame data. */
        FrameData* _frameData;

        /** Vector of current input frames. */
        std::vector<Frame*> _inputFrames;
    };

    std::ostream& operator << ( std::ostream& os, const Frame* frame );
};
#endif // EQS_FRAME_H
