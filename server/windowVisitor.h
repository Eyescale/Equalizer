
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_WINDOWVISITOR_H
#define EQSERVER_WINDOWVISITOR_H

#include "channelVisitor.h"

namespace eq
{
namespace server
{
    class Window;

    /**
     * A visitor to traverse non-const windows and children.
     */
    class WindowVisitor : public ChannelVisitor
    {
    public:
        /** Constructs a new WindowVisitor. */
        WindowVisitor(){}
        
        /** Destruct the WindowVisitor */
        virtual ~WindowVisitor(){}

        /** Visit a window on the down traversal. */
        virtual VisitorResult visitPre( Window* window )
            { return TRAVERSE_CONTINUE; }

        /** Visit a window on the up traversal. */
        virtual VisitorResult visitPost( Window* window )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const windows and children.
     */
    class ConstWindowVisitor : public ConstChannelVisitor
    {
    public:
        /** Constructs a new WindowVisitor. */
        ConstWindowVisitor(){}
        
        /** Destruct the WindowVisitor */
        virtual ~ConstWindowVisitor(){}

        /** Visit a window on the down traversal. */
        virtual VisitorResult visitPre( const Window* window )
            { return TRAVERSE_CONTINUE; }

        /** Visit a window on the up traversal. */
        virtual VisitorResult visitPost( const Window* window )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_WINDOWVISITOR_H
