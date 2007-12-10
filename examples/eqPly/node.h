
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

        const InitData& getInitData() const { return _initData; }
        const Model*    getModel() const    { return _model; }

    protected:
        virtual ~Node(){}

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();

        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
        virtual void frameDrawFinish( const uint32_t frameID,
                                      const uint32_t frameNumber )
            { /* nop, see frameStart */ }

    private:
        InitData _initData;
        Model*   _model;
    };
}

#endif // EQ_PLY_NODE_H
