
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_VIEWVISITOR_H
#define EQ_VIEWVISITOR_H

#include <eq/client/visitorResult.h>  // enum

namespace eq
{
    class View;

    /**
     * A visitor to traverse non-const views.
     */
    class ViewVisitor
    {
    public:
        /** Constructs a new ViewVisitor. */
        ViewVisitor(){}
        
        /** Destruct the ViewVisitor */
        virtual ~ViewVisitor(){}

        /** Visit a view. */
        virtual VisitorResult visit( View* view )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_VIEWVISITOR_H
