
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/frustum.h>        // base class
#include <eq/client/types.h>
#include <eq/client/viewport.h>       // member
#include <eq/client/visitorResult.h>  // enum

namespace eq
{
namespace server
{
    class Segment;
}

    class Canvas;
    class SegmentVisitor;

    /**
     * A segment covers a sub-area of a Canvas. It has a Frustum, and defines
     * one output Channel of the whole projection area, typically a projector or
     * screen.
     */
    class Segment : public eq::Frustum
    {
    public:
        /** Construct a new Segment. */
        EQ_EXPORT Segment();

        /** Destruct this segment. */
        EQ_EXPORT virtual ~Segment();

        /** @name Data Access */
        //@{
        /** @return the config of this view. */
        EQ_EXPORT Config* getConfig();

        /** @return the config of this view. */
        EQ_EXPORT const Config* getConfig() const;

        /** @return the segment's viewport. */
        const eq::Viewport& getViewport() const { return _vp; }
        //@}
        
        /** @name Operations */
        //@{
        /** 
         * Traverse this segment using a segment visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQ_EXPORT VisitorResult accept( SegmentVisitor& visitor );
        //@}

    protected:
        /** @sa Frustum::serialize */
        EQ_EXPORT virtual void serialize( net::DataOStream& os, 
                                          const uint64_t dirtyBits );
        /** @sa Frustum::deserialize */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );

        enum DirtyBits
        {
            DIRTY_VIEWPORT   = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_FILL1      = Frustum::DIRTY_CUSTOM << 1,
            DIRTY_FILL2      = Frustum::DIRTY_CUSTOM << 2,
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 3
        };

    private:
        /** The parent canvas. */
        Canvas* _canvas;
        friend class Canvas;

        /** The 2D area of this segment wrt to the canvas. */
        eq::Viewport _vp;
        friend class server::Segment;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };

}
#endif // EQ_SEGMENT_H
