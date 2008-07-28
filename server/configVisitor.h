
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONFIGVISITOR_H
#define EQSERVER_CONFIGVISITOR_H

#include "nodeVisitor.h"

namespace eq
{
namespace server
{
    class Config;

    /**
     * A visitor to traverse a non-const configs and children.
     */
    class ConfigVisitor : public NodeVisitor
    {
    public:
        /** Constructs a new ConfigVisitor. */
        ConfigVisitor(){}
        
        /** Destruct the ConfigVisitor */
        virtual ~ConfigVisitor(){}

        /** Visit a config. */
        virtual Result visit( Config* config )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CONFIGVISITOR_H
