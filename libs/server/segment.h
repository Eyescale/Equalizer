
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
    class Segment : public fabric::Segment< Canvas, Segment, Channel >
    {
    public:
        /** Construct a new Segment. */
        EQSERVER_EXPORT Segment( Canvas* parent );

        /** Destruct this segment. */
        virtual ~Segment();

        /** @name Data Access */
        //@{
        /** @return the config of this segment. */
        Config* getConfig();

        /** @return the config of this segment. */
        const Config* getConfig() const;

        /** @return the Server of this segment. */
        ServerPtr getServer();

        /** @return the index path to this segment. */
        SegmentPath getPath() const;

        /** Add a destination (View) channel. */
        void addDestinationChannel( Channel* channel );

        /** Remove a destination (View) channel. */
        bool removeDestinationChannel( Channel* channel );

        /** @return the vector of channels resulting from the segment/view
         *          intersection. */
        const Channels& getDestinationChannels() const 
            { return _destinationChannels; }

        /** @return the vector of destination channels for the given layout. */
        void findDestinationChannels( const Layout* layout,
                                      Channels& result ) const;
        //@}

    private:
        /** The resulting destination channels from the view intersections. */
        Channels _destinationChannels;

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
