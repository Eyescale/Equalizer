
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDUPDATEINPUTVISITOR_H
#define EQS_COMPOUNDUPDATEINPUTVISITOR_H

#include "compoundVisitor.h" // base class

namespace eqs
{
    class Channel;
    
    /**
     * The compound visitor updating the inherit input of a compound tree.
     */
    class CompoundUpdateInputVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateInputVisitor( 
            const stde::hash_map<std::string, Frame*>& outputFrames );
        virtual ~CompoundUpdateInputVisitor() {}

        /** Visit a non-leaf compound on the down traversal. */
        virtual Compound::VisitorResult visitPre( Compound* compound )
            { return visitLeaf( compound ); }
        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound );

    private:
        const stde::hash_map<std::string, Frame*>& _outputFrames;
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
