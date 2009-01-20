
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

    private:
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
