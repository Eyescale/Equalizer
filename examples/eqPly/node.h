
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_NODE_H
#define EQ_PLY_NODE_H

#include "eqPly.h"
#include "initData.h"

#include <eq/eq.h>

namespace eqPly
{
    /**
     * Representation of a node in the cluster
     * 
     * Manages node-specific data, namely requesting the mapping of the
     * initialization data by the local Config instance.
     */
    class Node : public eq::Node
    {
    public:
        Node( eq::Config* parent ) : eq::Node( parent ) {}

    protected:
        virtual ~Node(){}

        virtual bool configInit( const uint32_t initID );

    private:
    };
}

#endif // EQ_PLY_NODE_H
