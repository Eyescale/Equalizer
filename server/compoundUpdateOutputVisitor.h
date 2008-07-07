
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUNDUPDATEOUTPUTVISITOR_H
#define EQS_COMPOUNDUPDATEOUTPUTVISITOR_H

#include "compoundVisitor.h" // base class

namespace eqs
{
    class Channel;
    
    /**
     * The compound visitor updating the output data (frames, swapbarriers) of a
     * compound tree.
     */
    class CompoundUpdateOutputVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateOutputVisitor( const uint32_t frameNumber );
        virtual ~CompoundUpdateOutputVisitor() {}

        /** Visit a leaf compound. */
        virtual Compound::VisitorResult visitLeaf( Compound* compound );
        /** Visit a non-leaf compound on the up traversal. */
        virtual Compound::VisitorResult visitPost( Compound* compound )
            { return visitLeaf( compound ); }

        const stde::hash_map<std::string, eq::net::Barrier*>& getSwapBarriers()
            const { return _swapBarriers; }
        const stde::hash_map<std::string, Frame*>& getOutputFrames() const
            { return _outputFrames; }

    private:
        const uint32_t _frameNumber;
 
        stde::hash_map<std::string, eq::net::Barrier*> _swapBarriers;
        stde::hash_map<std::string, Frame*>          _outputFrames;

        void _updateOutput( Compound* compound );
        void _updateSwapBarriers( Compound* compound );
    };
};
#endif // EQS_CONSTCOMPOUNDVISITOR_H
