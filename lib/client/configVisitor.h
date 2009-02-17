
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIGVISITOR_H
#define EQ_CONFIGVISITOR_H

#include <eq/client/canvasVisitor.h>        // base class
#include <eq/client/layoutVisitor.h>        // base class
#include <eq/client/nodeVisitor.h>          // base class

namespace eq
{
    class Config;

    /**
     * A visitor to traverse non-const configs and children.
     */
    class ConfigVisitor : public NodeVisitor, 
                          public LayoutVisitor,
                          public CanvasVisitor
    {
    public:
        /** Constructs a new ConfigVisitor. */
        ConfigVisitor(){}
        
        /** Destruct the ConfigVisitor */
        virtual ~ConfigVisitor(){}

        /** Visit a config on the down traversal. */
        virtual VisitorResult visitPre( Config* config )
            { return TRAVERSE_CONTINUE; }

        /** Visit a config on the up traversal. */
        virtual VisitorResult visitPost( Config* config )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_CONFIGVISITOR_H
