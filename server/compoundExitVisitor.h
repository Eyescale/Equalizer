
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDEXITVISITOR_H
#define EQSERVER_COMPOUNDEXITVISITOR_H

#include "compoundVisitor.h" // base class

namespace eq
{
namespace server
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
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
