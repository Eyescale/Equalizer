
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_VIEWVISITOR_H
#define EQSERVER_VIEWVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class View;

    /**
     * A visitor to traverse a non-const views.
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
}
#endif // EQSERVER_VIEWVISITOR_H
