
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_NODE_H
#define EQ_PLY_NODE_H

#include "eqPly.h"
#include "initData.h"

#include <eq/eq.h>

class Node : public eq::Node
{
public:
    const InitData& getInitData() const { return _initData; }
    const Model*    getModel() const    { return _model; }

protected:
    virtual bool configInit( const uint32_t initID );
    virtual bool configExit();

private:
    InitData _initData;
    Model*   _model;
};

#endif // EQ_PLY_NODE_H
