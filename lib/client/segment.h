
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

#ifndef EQ_SEGMENT_H
#define EQ_SEGMENT_H

#include <eq/client/types.h>
#include <eq/fabric/segment.h>        // base class

namespace eq
{
    class Canvas;
    class Channel;

    /**
     * A segment covers a sub-area of a Canvas. It has a Frustum, and defines
     * one output Channel of the whole projection area, typically a projector or
     * screen.
     */
    class Segment : public fabric::Segment< Canvas, Segment, Channel >
    {
    public:
        /** Construct a new segment. @version 1.0 */
        EQ_EXPORT Segment( Canvas* parent );

        /** Destruct a segment. @version 1.0 */
        EQ_EXPORT virtual ~Segment();

        /** @name Data Access */
        //@{
        /** @return the config of this segment. */
        EQ_EXPORT Config* getConfig();

        /** @return the config of this segment. */
        EQ_EXPORT const Config* getConfig() const;

        /** @return the Server of this segment. */
        EQ_EXPORT ServerPtr getServer();
        //@}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };

}
#endif // EQ_SEGMENT_H
