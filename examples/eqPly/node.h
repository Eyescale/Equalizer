
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_NODE_H
#define EQ_PLY_NODE_H

#include "eqPly.h"
#include "initData.h"

#include <eq/eq.h>

namespace eqPly
{
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
