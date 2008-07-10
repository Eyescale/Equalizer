
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIPEVISITOR_H
#define EQ_PIPEVISITOR_H

#include <eq/client/windowVisitor.h>

namespace eq
{
    class Pipe;

    /**
     * A visitor to traverse a non-const pipes and children.
     */
    class PipeVisitor : public WindowVisitor
    {
    public:
        /** Constructs a new PipeVisitor. */
        PipeVisitor(){}
        
        /** Destruct the PipeVisitor */
        virtual ~PipeVisitor(){}

        /** Visit a pipe. */
        virtual Result visit( Pipe* pipe )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_PIPEVISITOR_H
