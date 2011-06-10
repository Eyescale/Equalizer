
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2011, Daniel Nachbaur<danielnachbaur@googlemail.com>
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

#ifndef EQSERVER_TILEQUEUE_H
#define EQSERVER_TILEQUEUE_H

#include "compound.h"

#include <co/base/bitOperation.h> // function getIndexOfLastBit

namespace eq
{
namespace server
{
    class Compound;
    class FrameData;
    class Node;

    /** A holder for frame data and parameters. */
    class TileQueue : public co::Object
    {
    public:
        /** 
         * Constructs a new TileQueue.
         */
        EQSERVER_API TileQueue();
        EQSERVER_API virtual ~TileQueue();
        TileQueue( const TileQueue& from );

        /**
         * @name Data Access
         */
        //@{
        void setCompound( Compound* compound )
            { EQASSERT( !_compound ); _compound = compound; }
        Compound* getCompound() const { return _compound; }
        Channel* getChannel() const 
            { return _compound ? _compound->getChannel() :0; }
        Node* getNode() const { return _compound ? _compound->getNode() :0; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        //FrameData* getMasterData() const { return _masterFrameData; }
        //bool hasData( const eq::Eye eye ) const
        //    { return ( _frameData[ co::base::getIndexOfLastBit(eye) ] != 0 ); }

        co::ObjectVersion getDataVersion( const Eye eye ) const;

        /** Set the size of the tiles. */
        void setSize( const Vector2i& size ) { _size = size; }

        /** @return the frame offset. */
        const Vector2i& getSize() const { return _size; }

        /**
         * @name Operations
         */
        //@{
        /** Commit the frame's data (used for output frames only) */
        void commitData();

        /** Commit the frame */
        EQSERVER_API virtual uint32_t commitNB( const uint32_t incarnation );

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
        //void addInputFrame( Frame* frame, const Compound* compound );

        /** @return the vector of current input frames. */
        //const Frames& getInputFrames( const eq::Eye eye ) const
        //    { return _inputFrames[ co::base::getIndexOfLastBit( eye ) ]; }

        /** Unset the frame data. */
        void unsetData();

        /** Reset the frame and delete all frame datas. */
        void flush();
        //@}

    protected:
        EQSERVER_API virtual ChangeType getChangeType() const 
                                                            { return INSTANCE; }
        EQSERVER_API virtual void getInstanceData( co::DataOStream& os );
        EQSERVER_API virtual void applyInstanceData( co::DataIStream& is );

    private:
        /** The parent compound. */
        Compound* _compound;

        /** The name which associates input to output frames. */
        std::string _name;

        Vector2i _size;

        /** Frame-specific data. */
        //eq::Frame::Data _data;

        /** The current, actual data used by the frame. */
        //eq::Frame::Data _inherit;

        /** All framedatas ever allocated, used for recycling old datas. */
        //std::deque<FrameData*> _datas;
        
        /** Current frame data. */
        //FrameData* _masterFrameData;
        //FrameData* _frameData[ eq::NUM_EYES ];

        /** Vector of current input frames. */
        //Frames _inputFrames[ eq::NUM_EYES ];
    };

    std::ostream& operator << ( std::ostream& os, const TileQueue* frame );
}
}
#endif // EQSERVER_TILEQUEUE_H
