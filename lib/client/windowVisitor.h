
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWVISITOR_H
#define EQ_WINDOWVISITOR_H

#include <eq/client/channelVisitor.h>

namespace eq
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

        /** Visit a window. */
        virtual Result visit( Window* window )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_WINDOWVISITOR_H
