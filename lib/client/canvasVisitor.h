
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CANVASVISITOR_H
#define EQ_CANVASVISITOR_H

#include <eq/client/segmentVisitor.h> // base class

namespace eq
{
    class Canvas;

    /**
     * A visitor to traverse non-const canvases and children.
     */
    class CanvasVisitor : public SegmentVisitor
    {
    public:
        /** Constructs a new CanvasVisitor. */
        CanvasVisitor(){}
        
        /** Destruct the CanvasVisitor */
        virtual ~CanvasVisitor(){}

        /** Visit a canvas on the down traversal. */
        virtual VisitorResult visitPre( Canvas* canvas )
            { return TRAVERSE_CONTINUE; }

        /** Visit a canvas on the up traversal. */
        virtual VisitorResult visitPost( Canvas* canvas )
            { return TRAVERSE_CONTINUE; }
    };

}
#endif // EQ_CANVASVISITOR_H
