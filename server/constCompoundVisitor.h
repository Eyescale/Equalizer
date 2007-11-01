
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CONSTCOMPOUNDVISITOR_H
#define EQS_CONSTCOMPOUNDVISITOR_H

#include "compound.h"  // nested enum

namespace eqs
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

        /** Return value of the visit methods */
        enum Result
        {
            CONTINUE,  //!< Keep traversing
            TERMINATE  //!< Abort traversal immediately
        };

        /** Visit a non-leaf compound on the down traversal. */
        virtual Compound::VisitorResult visitPre( const Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( const Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
        /** Visit a non-leaf compound on the up traversal. */
        virtual Compound::VisitorResult visitPost( const Compound* compound )
            { return Compound::TRAVERSE_CONTINUE; }
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
