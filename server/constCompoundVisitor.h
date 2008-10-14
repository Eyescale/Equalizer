
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONSTCOMPOUNDVISITOR_H
#define EQSERVER_CONSTCOMPOUNDVISITOR_H

#include "compound.h"  // nested enum

namespace eq
{
namespace server
{
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
        virtual Compound::VisitorResult visitPre( const Compound* compound )
            { return visit( compound ); }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( const Compound* compound )
            { return visit( compound ); }
        /** Visit a non-leaf compound on the up traversal. */
        virtual Compound::VisitorResult visitPost( const Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }

        /** Visit every compound on the down traversal. */
        virtual Compound::VisitorResult visit( const Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
