
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SEGMENTVISITOR_H
#define EQ_SEGMENTVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
    class Segment;

    /**
     * A visitor to traverse non-const segments.
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
#endif // EQ_SEGMENTVISITOR_H
