
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIGVISITOR_H
#define EQ_CONFIGVISITOR_H

#include <eq/client/nodeVisitor.h>

namespace eq
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
#endif // EQ_CONFIGVISITOR_H
