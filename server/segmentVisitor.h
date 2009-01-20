
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_SEGMENTVISITOR_H
#define EQSERVER_SEGMENTVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Segment;

    /**
     * A visitor to traverse a non-const segments.
     */
    class SegmentVisitor
    {
    public:
        /** Constructs a new SegmentVisitor. */
        SegmentVisitor(){}
        
        /** Destruct the SegmentVisitor */
        virtual ~SegmentVisitor(){}

        /** Visit a segment. */
        virtual VisitorResult visit( Segment* segment )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_SEGMENTVISITOR_H
