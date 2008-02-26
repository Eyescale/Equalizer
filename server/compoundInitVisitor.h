
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDINITVISITOR_H
#define EQS_COMPOUNDINITVISITOR_H

#include "compoundVisitor.h" // base class

namespace eqs
{
    /**
     * The compound visitor initializing a compound tree.
     */
    class CompoundInitVisitor : public CompoundVisitor
    {
    public:
        CompoundInitVisitor();
        virtual ~CompoundInitVisitor() {}

        /** Visit a non-leaf compound on the down traversal. */
        virtual Compound::VisitorResult visitPre( Compound* compound )
            { return visitLeaf( compound ); }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound );

    private:
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
