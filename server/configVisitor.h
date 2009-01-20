
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONFIGVISITOR_H
#define EQSERVER_CONFIGVISITOR_H

#include "canvasVisitor.h"   // base class
#include "compoundVisitor.h" // base class
#include "layoutVisitor.h"   // base class
#include "nodeVisitor.h"     // base class

namespace eq
{
namespace server
{
    class Config;

    /**
     * A visitor to traverse non-const configs and children.
     */
    class ConfigVisitor : public NodeVisitor, 
                          public CompoundVisitor,
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
}
#endif // EQSERVER_CONFIGVISITOR_H
