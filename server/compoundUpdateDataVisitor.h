
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDUPDATEDATAVISITOR_H
#define EQS_COMPOUNDUPDATEDATAVISITOR_H

#include "compoundVisitor.h" // base class

namespace eqs
{
    class Channel;
    
    /**
     * The compound visitor updating the inherit data of a compound tree.
     */
    class CompoundUpdateDataVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateDataVisitor( const uint32_t frameNumber );
        virtual ~CompoundUpdateDataVisitor() {}

        /** Visit a non-leaf compound on the down traversal. */
        virtual Compound::VisitorResult visitPre( Compound* compound )
            { return visitLeaf( compound ); }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound );

    private:
        const uint32_t _frameNumber;

        void _updateDrawFinish( Compound* compound );
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
