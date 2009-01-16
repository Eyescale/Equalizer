
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONFIGVISITOR_H
#define EQSERVER_CONFIGVISITOR_H

#include "compoundVisitor.h" // base class
#include "nodeVisitor.h"     // base class

namespace eq
{
namespace server
{
    class Config;

    /**
     * A visitor to traverse a non-const configs and children.
     */
    class ConfigVisitor : public NodeVisitor, public CompoundVisitor
    {
    public:
        /** Constructs a new ConfigVisitor. */
        ConfigVisitor(){}
        
        /** Destruct the ConfigVisitor */
        virtual ~ConfigVisitor(){}

        /** Visit a config on the down traversal. */
        virtual Result visitPre( Config* config )
            { return TRAVERSE_CONTINUE; }

        /** Visit a config on the up traversal. */
        virtual Result visitPost( Config* config )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CONFIGVISITOR_H
