
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_LAYOUTVISITOR_H
#define EQSERVER_LAYOUTVISITOR_H

#include "viewVisitor.h"

namespace eq
{
namespace server
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

    /**
     * A visitor to traverse const layouts and children.
     */
    class ConstLayoutVisitor : public ConstViewVisitor
    {
    public:
        /** Constructs a new LayoutVisitor. */
        ConstLayoutVisitor(){}
        
        /** Destruct the LayoutVisitor */
        virtual ~ConstLayoutVisitor(){}

        /** Visit a layout on the down traversal. */
        virtual VisitorResult visitPre( const Layout* layout )
            { return TRAVERSE_CONTINUE; }

        /** Visit a layout on the up traversal. */
        virtual VisitorResult visitPost( const Layout* layout )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_LAYOUTVISITOR_H
