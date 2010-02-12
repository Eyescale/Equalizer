
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

#ifndef EQ_CANVASVISITOR_H
#define EQ_CANVASVISITOR_H

#include <eq/client/segmentVisitor.h> // base class

namespace eq
{
    class Canvas;

    /**
     * A visitor to traverse canvases and children.
     * @sa Canvas::accept()
     */
    class CanvasVisitor : public SegmentVisitor
    {
    public:
        /** Constructs a new CanvasVisitor. @version 1.0 */
        CanvasVisitor(){}
        
        /** Destruct the CanvasVisitor @version 1.0 */
        virtual ~CanvasVisitor(){}

        /** Visit a canvas on the down traversal. @version 1.0 */
        virtual VisitorResult visitPre( Canvas* canvas )
            { return visitPre( static_cast< const Canvas* >( canvas )); }

        /** Visit a canvas on the up traversal. @version 1.0 */
        virtual VisitorResult visitPost( Canvas* canvas )
            { return visitPost( static_cast< const Canvas* >( canvas )); }

        /** Visit a canvas on the down traversal. @version 1.0 */
        virtual VisitorResult visitPre( const Canvas* canvas )
            { return TRAVERSE_CONTINUE; }

        /** Visit a canvas on the up traversal. @version 1.0 */
        virtual VisitorResult visitPost( const Canvas* canvas )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_CANVASVISITOR_H
