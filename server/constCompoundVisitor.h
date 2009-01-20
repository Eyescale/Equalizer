
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONSTCOMPOUNDVISITOR_H
#define EQSERVER_CONSTCOMPOUNDVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Compound;

    /**
     * A visitor to traverse a const compound tree.
     */
    class ConstCompoundVisitor
    {
    public:
        /** Constructs a new CompoundVisitor. */
        ConstCompoundVisitor(){}
        
        /** Destruct the CompoundVisitor */
        virtual ~ConstCompoundVisitor(){}

        /** Visit a non-leaf compound on the down traversal. */
        virtual VisitorResult visitPre( const Compound* compound )
            { return visit( compound ); }
        /** Visit a leaf compound. */
        virtual VisitorResult visitLeaf( const Compound* compound )
            { return visit( compound ); }
        /** Visit a non-leaf compound on the up traversal. */
        virtual VisitorResult visitPost( const Compound* compound )
            { return TRAVERSE_CONTINUE; }

        /** Visit every compound on the down traversal. */
        virtual VisitorResult visit( const Compound* compound )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
