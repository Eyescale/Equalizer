
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDVISITOR_H
#define EQSERVER_COMPOUNDVISITOR_H

#include "compound.h"  // nested enum

namespace eq
{
namespace server
{
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
        virtual Compound::VisitorResult visitPre( Compound* compound )
            { return visit( compound ); }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound )
            { return visit( compound ); }
        /** Visit a non-leaf compound on the up traversal. */
        virtual Compound::VisitorResult visitPost( Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }

        /** Visit every compound on the down traversal. */
        virtual Compound::VisitorResult visit( Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_COMPOUNDVISITOR_H
