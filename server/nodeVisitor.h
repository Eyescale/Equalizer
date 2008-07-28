
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_NODEVISITOR_H
#define EQSERVER_NODEVISITOR_H

#include "pipeVisitor.h"

namespace eq
{
namespace server
{
    class Node;

    /**
     * A visitor to traverse a non-const nodes and children.
     */
    class NodeVisitor : public PipeVisitor
    {
    public:
        /** Constructs a new NodeVisitor. */
        NodeVisitor(){}
        
        /** Destruct the NodeVisitor */
        virtual ~NodeVisitor(){}

        /** Visit a node. */
        virtual Result visit( Node* node )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_NODEVISITOR_H
