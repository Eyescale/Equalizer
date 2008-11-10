
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_NODE_H
#define EVOLVE_NODE_H

#include "eVolve.h"
#include "initData.h"

#include <eq/eq.h>

namespace eVolve
{
    class Node : public eq::Node
    {
    public:
        Node( eq::Config* parent ) : eq::Node( parent ) {}

        const InitData& getInitData() const { return _initData; }

    protected:
        virtual ~Node(){}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

    private:
        InitData _initData;
    };
}

#endif // EVOLVE_NODE_H
