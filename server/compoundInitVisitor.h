
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

    private:
        void _updateFrustum( Compound* compound );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
