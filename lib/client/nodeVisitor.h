
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODEVISITOR_H
#define EQ_NODEVISITOR_H

#include <eq/client/pipeVisitor.h>

namespace eq
{
    class Node;

    /**
     * A visitor to traverse non-const nodes and children.
     */
    class NodeVisitor : public PipeVisitor
    {
    public:
        /** Constructs a new NodeVisitor. */
        NodeVisitor(){}
        
        /** Destruct the NodeVisitor */
        virtual ~NodeVisitor(){}

        /** Visit a node on the down traversal. */
        virtual VisitorResult visitPre( Node* node )
            { return TRAVERSE_CONTINUE; }

        /** Visit a node on the up traversal. */
        virtual VisitorResult visitPost( Node* node )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_NODEVISITOR_H
