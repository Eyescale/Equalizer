
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_SEGMENT_H
#define EQSERVER_SEGMENT_H

#include "types.h"
#include "visitorResult.h"  // enum

#include <eq/fabric/segment.h>      // base class

namespace eq
{
namespace server
{
    class Canvas;
    
    /**
     * The segment. @sa eq::Segment
     */
    class Segment : public fabric::Segment< Canvas, Segment >
    {
    public:
        /** Construct a new Segment. */
        EQSERVER_EXPORT Segment( Canvas* parent );

        /** Creates a new, deep copy of a segment. */
        Segment( const Segment& from, Canvas* parent );

        /** Destruct this segment. */
        virtual ~Segment();

        /** @name Data Access */
        //@{
        /** @return the config of this segment. */
        Config* getConfig();

        /** @return the config of this segment. */
        const Config* getConfig() const;

        /** @return the index path to this segment. */
        SegmentPath getPath() const;

        /** 
         * Set the channel of this segment.
         *
         * The channel defines the output area for this segment, typically a
         * rendering area covering a graphics card output.
         * 
         * @param channel the channel.
         */
        void setChannel( Channel* channel ) { _channel = channel; }

        /** Return the output channel of this segment. */
        Channel* getChannel()               { return _channel; }
        const Channel* getChannel() const   { return _channel; }

        /** Add a destination (View) channel. */
        void addDestinationChannel( Channel* channel );

        /** Remove a destination (View) channel. */
        bool removeDestinationChannel( Channel* channel );

        /** @return the vector of channels resulting from the segment/view
         *          intersection. */
        const ChannelVector& getDestinationChannels() const 
            { return _destinationChannels; }
        //@}

    private:
        /** The output channel of this segment. */
        Channel* _channel;

        /** The resulting destination channels from the view intersections. */
        ChannelVector _destinationChannels;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** Update the view (wall/projection). */
        void _updateView();
    };
}
}
#endif // EQSERVER_SEGMENT_H
