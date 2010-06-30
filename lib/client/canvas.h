
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

#ifndef EQ_CANVAS_H
#define EQ_CANVAS_H

#include <eq/client/types.h>
#include <eq/fabric/canvas.h> // base class

namespace eq
{
    class Segment;

    /**
     * A canvas represents a logical 2D projection surface.
     *
     * A canvas consists of one or more Segment, which represent the physical
     * output channels. Segments have a viewport, which defines which part of
     * the logical 2D projection surface they occupy. Segments overlap each
     * other when edge-blending is used, and have gaps for display
     * walls. Passive stereo systems use one segment for each eye pass, so that
     * two segments have the same viewport. Application windows typically use
     * one canvas per Window.
     *
     * A canvas has a Frustum, which is used to compute a sub-frustum for
     * segments which have no frustum specified. This is useful for planar
     * projection systems.
     *
     * A canvas has one ore more layouts, of which one Layout is the active
     * layout, defining the set of logical views currently used to render on the
     * canvas. The layout can be switched at runtime. A canvas with a
     * <code>0</code> layout does not render anything, i.e., it is not active.
     *
     * @sa fabric::Canvas for public methods
     */
    class Canvas : public fabric::Canvas< Config, Canvas, Segment, Layout >
    {
    public:
        /** Construct a new canvas. @version 1.0 */
        EQ_EXPORT Canvas( Config* parent );

        /** Destruct this canvas. @version 1.0 */
        EQ_EXPORT virtual ~Canvas();

        /** @name Data Access */
        //@{
        /** @return the Server of this canvas. @version 1.0 */
        EQ_EXPORT ServerPtr getServer();
        //@}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
#endif // EQ_CANVAS_H
