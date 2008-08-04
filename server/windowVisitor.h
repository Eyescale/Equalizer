
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
     * A visitor to traverse a non-const windows and children.
     */
    class WindowVisitor : public ChannelVisitor
    {
    public:
        /** Constructs a new WindowVisitor. */
        WindowVisitor(){}
        
        /** Destruct the WindowVisitor */
        virtual ~WindowVisitor(){}

        /** Visit a window on the down traversal. */
        virtual Result visitPre( Window* window )
            { return TRAVERSE_CONTINUE; }

        /** Visit a window on the up traversal. */
        virtual Result visitPost( Window* window )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_WINDOWVISITOR_H
