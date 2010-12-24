
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQSERVER_FRAME_H
#define EQSERVER_FRAME_H

#include "compound.h"

#include <eq/frame.h>
#include <co/base/bitOperation.h> // function getIndexOfLastBit

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
    class Frame : public co::Object
    {
    public:
        /** 
         * Constructs a new Frame.
         */
        EQSERVER_EXPORT Frame();
        Frame( const Frame& from );

        /**
         * @name Data Access
         */
        //@{
        Compound* getCompound() const { return _compound; }
        Channel* getChannel() const 
            { return _compound ? _compound->getChannel() :0; }
        Node* getNode() const { return _compound ? _compound->getNode() :0; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        FrameData* getMasterData() const { return _masterFrameData; }
        bool       hasData( const eq::Eye eye ) const
            { return ( _frameData[ co::base::getIndexOfLastBit( eye ) ] != 0 ); }

        co::ObjectVersion getDataVersion( const Eye eye ) const;

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
         * Set the frame's zoom factor.
         *
         * Output frames downscale pixel data during readback, and upscale it on
         * the input frame by setting the input frame's inherit zoom. Input
         * frames zoom pixel data during compositing.
         *
         * @param zoom the zoom factor.
         */
        void setZoom( const eq::Zoom& zoom ) { _data.zoom = zoom; }
        
        /** @return the zoom factor. */
        const eq::Zoom& getZoom() const { return _data.zoom; }

        /** 
         * Set the frame buffers to be read or write by this frame.
         * 
         * @param buffers a bitwise combination of the buffers.
         */
        void setBuffers( const uint32_t buffers ) { _buffers = buffers; }

        /** return the frame storage type. */    
        eq::Frame::Type getType() const { return _type; }

        /** 
         * Set the frame storage type.
         * 
         * @param type frame storage type.
         */
        void setType( const eq::Frame::Type type ) { _type = type; }

        /** @return the frame buffers used by this frame. */
        uint32_t getBuffers() const { return _buffers; }
        //@}

        /**
         * @name Operations
         */
        //@{
        /** Commit the frame's data (used for output frames only) */
        void commitData();

        /** Commit the frame */
        EQSERVER_EXPORT virtual uint32_t commitNB();

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
        const Frames& getInputFrames( const eq::Eye eye ) const
            { return _inputFrames[ co::base::getIndexOfLastBit( eye ) ]; }

        /** Unset the frame data. */
        void unsetData();

        /** Reset the frame and delete all frame datas. */
        void flush();
        //@}

        /**
         * @name Inherit data access.
         * 
         * The inherit data are the actual derived (from parent compound)
         * parameters, set each frame during compound update. The inherit data
         * is the data used by client-side frame class for readback and
         * assembly.
         */
        //@{
        /** 
         * Set the offset of the frame.
         *
         * The offset defines relative data position wrt to the current
         * destination channel of the source.
         */
        void setInheritOffset( const Vector2i& offset )
            { _inherit.offset = offset; }
        
        /** @return the frame offset. */
        const Vector2i& getOffset() const { return _data.offset; }

        /** 
         * Set the offset of the frame.
         */
        void setOffset( const Vector2i& offset )
           { _data.offset = offset; }
        
        /** Set the inherit frame zoom factor. */
        void setInheritZoom( const eq::Zoom& zoom )
            { _inherit.zoom = zoom; }

        /** @return the inherit zoom factor. */
        const eq::Zoom& getInheritZoom() const { return _inherit.zoom; }
        //@}
        
    protected:
        EQSERVER_EXPORT virtual ~Frame();
        EQSERVER_EXPORT virtual ChangeType getChangeType() const 
                                                            { return INSTANCE; }
        EQSERVER_EXPORT virtual void getInstanceData( co::DataOStream& os );
        EQSERVER_EXPORT virtual void applyInstanceData( co::DataIStream& is );

    private:
        /** The parent compound. */
        Compound* _compound;
        friend class Compound;

        /** The name which associates input to output frames. */
        std::string _name;

        /** Frame-specific data. */
        eq::Frame::Data _data;
        eq::Viewport    _vp;
        uint32_t        _buffers;
        eq::Frame::Type _type;

        /** The current, actual data used by the frame. */
        eq::Frame::Data _inherit;

        /** All framedatas ever allocated, used for recycling old datas. */
        std::deque<FrameData*> _datas;
        
        /** Current frame data. */
        FrameData* _masterFrameData;
        FrameData* _frameData[ eq::NUM_EYES ];

        /** Vector of current input frames. */
        Frames _inputFrames[ eq::NUM_EYES ];
    };

    std::ostream& operator << ( std::ostream& os, const Frame* frame );
}
}
#endif // EQSERVER_FRAME_H
