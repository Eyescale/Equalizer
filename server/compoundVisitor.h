
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDVISITOR_H
#define EQSERVER_COMPOUNDVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Compound;

    /**
     * A visitor to traverse a non-const compound tree.
     */
    class CompoundVisitor
    {
    public:
        /** Constructs a new CompoundVisitor. */
        CompoundVisitor(){}
        
        /** Destruct the CompoundVisitor */
        virtual ~CompoundVisitor(){}

        /** Visit a non-leaf compound on the down traversal. */
        virtual VisitorResult visitPre( Compound* compound )
            { return visit( compound ); }
        /** Visit a leaf compound. */
        virtual VisitorResult visitLeaf( Compound* compound )
            { return visit( compound ); }
        /** Visit a non-leaf compound on the up traversal. */
        virtual VisitorResult visitPost( Compound* compound )
            { return TRAVERSE_CONTINUE; }

        /** Visit every compound on the down traversal. */
        virtual VisitorResult visit( Compound* compound )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_COMPOUNDVISITOR_H
