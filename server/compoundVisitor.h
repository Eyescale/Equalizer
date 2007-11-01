
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDVISITOR_H
#define EQS_COMPOUNDVISITOR_H

namespace eqs
{
    class Compound;
    
    /**
     * A visitor to traverse a compound tree.
     */
    class CompoundVisitor
    {
    public:
        /** Constructs a new CompoundVisitor. */
        CompoundVisitor(){}
        
        /** Destruct the CompoundVisitor */
        virtual ~CompoundVisitor(){}

        /** Return value of the visit methods */
        enum Result
        {
            CONTINUE,  //!< Keep traversing
            TERMINATE  //!< Abort traversal immediately
        };

        // non-const methods used by 'Compound::apply()'
        /** Visit a non-leaf compound on the down traversal. */
        virtual Result visitPre( Compound* compound ){ return CONTINUE; }
        /** Visit a leaf compound. */
        virtual Result visitLeaf( Compound* compound ){ return CONTINUE; }
        /** Visit a non-leaf compound on the up traversal. */
        virtual Result visitPost( Compound* compound ){ return CONTINUE; }

        // const methods used by 'Compound::apply() const'
        /** Visit a non-leaf compound on the down traversal. */
        virtual Result visitPre( const Compound* compound ){ return CONTINUE; }
        /** Visit a leaf compound. */
        virtual Result visitLeaf( const Compound* compound ){ return CONTINUE; }
        /** Visit a non-leaf compound on the up traversal. */
        virtual Result visitPost( const Compound* compound ){ return CONTINUE; }
    };
};
#endif // EQS_COMPOUNDVISITOR_H
