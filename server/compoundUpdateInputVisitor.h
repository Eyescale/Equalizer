
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDUPDATEINPUTVISITOR_H
#define EQSERVER_COMPOUNDUPDATEINPUTVISITOR_H

#include "compoundVisitor.h" // base class

namespace eq
{
namespace server
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
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
