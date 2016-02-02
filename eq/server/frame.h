
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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
#include "types.h"

#include <eq/fabric/frame.h> // base class
#include <lunchbox/bitOperation.h> // function getIndexOfLastBit

namespace eq
{
namespace server
{
/** A holder for frame data and parameters. */
class Frame : public fabric::Frame
{
public:
    /** Construct a new Frame. */
    EQSERVER_API Frame();
    EQSERVER_API virtual ~Frame();
    Frame( const Frame& from );

    /** @name Data Access */
    //@{
    void setCompound( Compound* compound )
    { LBASSERT( !_compound ); _compound = compound; }
    Compound* getCompound() const { return _compound; }
    Channel* getChannel() const
    { return _compound ? _compound->getChannel() : 0; }
    Node* getNode() const { return _compound ? _compound->getNode() : 0; }

    FrameData* getMasterData() const { return _masterFrameData; }
    bool hasData( const Eye eye ) const
        { return ( _frameData[ lunchbox::getIndexOfLastBit(eye) ] != 0 ); }

    /**
     * Set the frame's viewport wrt the compound (output frames) or wrt the
     * corresponding output frame (input frames).
     *
     * @param vp the fractional viewport.
     */
    void setViewport( const Viewport& vp ) { _vp = vp; }

    /** @return the fractional viewport. */
    const Viewport& getViewport() const { return _vp; }

    /**
     * Set the frame buffers to be read or write by this frame.
     *
     * @param buffers a bitwise combination of the buffers.
     */
    void setBuffers( const uint32_t buffers ) { _buffers = buffers; }

    /** return the frame storage type. */
    Type getType() const { return _type; }

    /**
     * Set the frame storage type.
     *
     * @param type frame storage type.
     */
    void setType( const Type type ) { _type = type; }

    /** @return the frame buffers used by this frame. */
    uint32_t getBuffers() const { return _buffers; }
    //@}

    /** @name Operations */
    //@{
    /** Commit the frame's data (used for output frames only) */
    void commitData();

    /** Commit the frame */
    EQSERVER_API virtual uint128_t commit( const uint32_t incarnation =
                                           CO_COMMIT_NEXT );

    /**
     * Cycle the current FrameData.
     *
     * Used for output frames to allocate/recycle frame data. Also
     * clears the list of input frames.
     *
     * @param frameNumber the current frame number.
     * @param compound the compound holding the output frame.
     */
    void cycleData( const uint32_t frameNumber, const Compound* compound );

    /**
     * Add an input frame to this (output) frame
     *
     * @param frame the input frame.
     * @param compound the compound holding the input frame.
     */
    void addInputFrame( Frame* frame, const Compound* compound );

    /** @return the vector of current input frames. */
    const Frames& getInputFrames( const Eye eye ) const
    { return _inputFrames[ lunchbox::getIndexOfLastBit( eye ) ]; }

    /** Unset the frame data. */
    void unsetData();

    /** Reset the frame and delete all frame datas. */
    void flush();
    //@}

    /**
     * @name Native data access.
     *
     * Native data is the raw data based on which the actual data used is
     * computed each frame and set using the non-native methods.
     */
    //@{
    /**
     * Set the offset of the frame.
     *
     * The offset defines relative data position wrt to the current
     * destination channel of the source.
     */
    void setNativeOffset( const Vector2i& offset )
    { _native.setOffset( offset ); }

    /** @return the frame's effective offset. */
    const Vector2i& getNativeOffset() const { return _native.getOffset(); }

    /** Set the native frame zoom factor. */
    void setNativeZoom( const Zoom& zoom ) { _native.setZoom( zoom ); }

    /** @return the native zoom factor. */
    const Zoom& getNativeZoom() const { return _native.getZoom(); }
    //@}

private:
    /** The parent compound. */
    Compound* _compound;

    /** Frame-specific data. */
    Viewport _vp;
    uint32_t _buffers;
    Type _type;

    /** The configured frame data (base class contains inherit values). */
    fabric::Frame _native;

    /** All framedatas ever allocated, used for recycling old datas. */
    std::deque<FrameData*> _datas;

    /** Current frame data. */
    FrameData* _masterFrameData;
    FrameData* _frameData[ fabric::NUM_EYES ];

    /** Vector of current input frames. */
    Frames _inputFrames[ fabric::NUM_EYES ];
};

EQSERVER_API std::ostream& operator << ( std::ostream&, const Frame& );
}
}
#endif // EQSERVER_FRAME_H
