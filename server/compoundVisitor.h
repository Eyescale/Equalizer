
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDVISITOR_H
#define EQS_COMPOUNDVISITOR_H

#include "compound.h"  // nested enum

namespace eqs
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
            { return Compound::TRAVERSE_CONTINUE; }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
        /** Visit a non-leaf compound on the up traversal. */
        virtual Compound::VisitorResult visitPost( Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
    };
};
#endif // EQS_COMPOUNDVISITOR_H
