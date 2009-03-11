
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDUPDATEOUTPUTVISITOR_H
#define EQSERVER_COMPOUNDUPDATEOUTPUTVISITOR_H

#include "compoundVisitor.h" // base class
#include "compound.h"        // nested type

namespace eq
{
namespace server
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

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

        const Compound::BarrierMap& getSwapBarriers()
            const { return _swapBarriers; }
        const Compound::FrameMap& getOutputFrames() const
            { return _outputFrames; }

    private:
        const uint32_t _frameNumber;
 
        Compound::BarrierMap _swapBarriers;
        Compound::FrameMap   _outputFrames;

        void _updateOutput( Compound* compound );
        void _updateZoom( const Compound* compound, Frame* frame );
        void _updateSwapBarriers( Compound* compound );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
