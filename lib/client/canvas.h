
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

#include <eq/client/frustum.h>        // base class
#include <eq/client/types.h>
#include <eq/client/visitorResult.h>  // enum

#include <eq/net/object.h>
#include <string>

namespace eq
{
namespace server
{
    class Canvas;
}
    class CanvasVisitor;

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
     * A canvas has one ore more layouts, of which one Layout
     * is the active layout, defining the set of logical views currently used to
     * render on the canvas. The layout can be switched at runtime. A canvas
     * with a NULL layout does not render anything, i.e., it is not active.
     */
    class Canvas : public Frustum
    {
    public:
        /** Construct a new Canvas. @version 1.0 */
        EQ_EXPORT Canvas();

        /** Destruct this canvas. @version 1.0 */
        EQ_EXPORT virtual ~Canvas();

        /** @name Data Access */
        //@{
        /** @return the parent config. @version 1.0 */
        Config*       getConfig()       { return _config; }
        /** @return the parent config. @version 1.0 */
        const Config* getConfig() const { return _config; }

        /** @return the index of the active layout. @version 1.0 */
        uint32_t getActiveLayoutIndex() const { return _activeLayout; }

        /** @return the active layout. @version 1.0 */
        EQ_EXPORT const Layout* getActiveLayout() const;

        /** @return the vector of child segments. @version 1.0 */
        const SegmentVector& getSegments() const { return _segments; }        

        /** @return the vector of possible layouts. @version 1.0 */
        const LayoutVector& getLayouts() const { return _layouts; }        
        //@}

        /** @name Operations */
        //@{
        /** Activate the given layout on this canvas. @version 1.0 */
        EQ_EXPORT virtual void useLayout( const uint32_t index );

        /** 
         * Traverse this canvas and all children using a canvas visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         * @version 1.0
         */
        EQ_EXPORT VisitorResult accept( CanvasVisitor& visitor );

        /** Const-version of accept(). */
        EQ_EXPORT VisitorResult accept( CanvasVisitor& visitor ) const;

        /** @return true if the layout has changed. @internal */
        bool hasDirtyLayout() const { return getDirty() & DIRTY_LAYOUT; }
        //@}

    protected:
        /** @sa Frustum::serialize. @version 1.0 */
        EQ_EXPORT void serialize( net::DataOStream& os, 
                                  const uint64_t dirtyBits );

        /** @sa Frustum::deserialize. @version 1.0 */
        EQ_EXPORT virtual void deserialize( net::DataIStream& is, 
                                            const uint64_t dirtyBits );
        enum DirtyBits
        {
            DIRTY_LAYOUT     = Frustum::DIRTY_CUSTOM << 0,
            DIRTY_CHILDREN   = Frustum::DIRTY_CUSTOM << 1,
            DIRTY_FILL1      = Frustum::DIRTY_CUSTOM << 2,
            DIRTY_FILL2      = Frustum::DIRTY_CUSTOM << 3,
            /** First usable dirty bit for sub-classes of a canvas. */
            DIRTY_CUSTOM     = Frustum::DIRTY_CUSTOM << 4
        };

    private:
        /** The parent config. */
        Config* _config;
        friend class Config;
        friend class server::Canvas;

        /** The currently active layout on this canvas. */
        uint32_t _activeLayout;

        /** Allowed layouts on this canvas. */
        LayoutVector _layouts;

        /** Child segments on this canvas. */
        SegmentVector _segments;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** Deregister this canvas, and all children, from its net::Session.*/
        void _deregister();
    };
}
#endif // EQ_CANVAS_H
