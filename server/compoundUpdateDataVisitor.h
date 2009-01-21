
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COMPOUNDUPDATEDATAVISITOR_H
#define EQSERVER_COMPOUNDUPDATEDATAVISITOR_H

#include "compoundVisitor.h" // base class
#include "types.h"

namespace eq
{
namespace server
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

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

    private:
        const uint32_t _frameNumber;

        void _updateDrawFinish( Compound* compound );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
