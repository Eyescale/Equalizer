
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_PIPEVISITOR_H
#define EQSERVER_PIPEVISITOR_H

#include "windowVisitor.h"

namespace eq
{
namespace server
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
}
#endif // EQSERVER_PIPEVISITOR_H
