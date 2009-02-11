
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CANVASVISITOR_H
#define EQSERVER_CANVASVISITOR_H

#include "segmentVisitor.h" // base class

namespace eq
{
namespace server
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

    /**
     * A visitor to traverse const canvases and children.
     */
    class ConstCanvasVisitor : public ConstSegmentVisitor
    {
    public:
        /** Constructs a new CanvasVisitor. */
        ConstCanvasVisitor(){}
        
        /** Destruct the CanvasVisitor */
        virtual ~ConstCanvasVisitor(){}

        /** Visit a canvas on the down traversal. */
        virtual VisitorResult visitPre( const Canvas* canvas )
            { return TRAVERSE_CONTINUE; }

        /** Visit a canvas on the up traversal. */
        virtual VisitorResult visitPost( const Canvas* canvas )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CANVASVISITOR_H
