
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_LAYOUTVISITOR_H
#define EQ_LAYOUTVISITOR_H

#include <eq/client/viewVisitor.h> // base class

namespace eq
{
    class Layout;

    /**
     * A visitor to traverse non-const layouts and children.
     */
    class LayoutVisitor : public ViewVisitor
    {
    public:
        /** Constructs a new LayoutVisitor. */
        LayoutVisitor(){}
        
        /** Destruct the LayoutVisitor */
        virtual ~LayoutVisitor(){}

        /** Visit a layout on the down traversal. */
        virtual VisitorResult visitPre( Layout* layout )
            { return TRAVERSE_CONTINUE; }

        /** Visit a layout on the up traversal. */
        virtual VisitorResult visitPost( Layout* layout )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_LAYOUTVISITOR_H
