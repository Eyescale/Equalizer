
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDEXITVISITOR_H
#define EQS_COMPOUNDEXITVISITOR_H

#include "compoundVisitor.h" // base class

namespace eqs
{
    class Channel;
    
    /**
     * The compound visitor exitializing a compound tree.
     */
    class CompoundExitVisitor : public CompoundVisitor
    {
    public:
        CompoundExitVisitor();
        virtual ~CompoundExitVisitor() {}

        /** Visit a non-leaf compound on the down traversal. */
        virtual Compound::VisitorResult visitPre( Compound* compound )
            { return visitLeaf( compound ); }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound );

    private:
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
