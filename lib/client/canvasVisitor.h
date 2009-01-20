
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CANVASVISITOR_H
#define EQ_CANVASVISITOR_H

#include <eq/client/visitorResult.h> // enum

namespace eq
{
    class Canvas;

    /**
     * A visitor to traverse non-const canvases and children.
     */
    class CanvasVisitor
    {
    public:
        /** Constructs a new CanvasVisitor. */
        CanvasVisitor(){}
        
        /** Destruct the CanvasVisitor */
        virtual ~CanvasVisitor(){}

        /** Visit a canvas. */
        virtual VisitorResult visit( Canvas* canvas )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_CANVASVISITOR_H
