
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
     * A visitor to traverse non-const pipes and children.
     */
    class PipeVisitor : public WindowVisitor
    {
    public:
        /** Constructs a new PipeVisitor. */
        PipeVisitor(){}
        
        /** Destruct the PipeVisitor */
        virtual ~PipeVisitor(){}

        /** Visit a pipe on the down traversal. */
        virtual VisitorResult visitPre( Pipe* pipe )
            { return TRAVERSE_CONTINUE; }

        /** Visit a pipe on the up traversal. */
        virtual VisitorResult visitPost( Pipe* pipe )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const pipes and children.
     */
    class ConstPipeVisitor : public ConstWindowVisitor
    {
    public:
        /** Constructs a new PipeVisitor. */
        ConstPipeVisitor(){}
        
        /** Destruct the PipeVisitor */
        virtual ~ConstPipeVisitor(){}

        /** Visit a pipe on the down traversal. */
        virtual VisitorResult visitPre( const Pipe* pipe )
            { return TRAVERSE_CONTINUE; }

        /** Visit a pipe on the up traversal. */
        virtual VisitorResult visitPost( const Pipe* pipe )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_PIPEVISITOR_H
