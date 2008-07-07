
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDINITVISITOR_H
#define EQSERVER_COMPOUNDINITVISITOR_H

#include "compoundVisitor.h" // base class

namespace eq
{
namespace server
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
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
