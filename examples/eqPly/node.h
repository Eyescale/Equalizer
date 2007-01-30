
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_NODE_H
#define EQ_PLY_NODE_H

#include "eqPly.h"

#include <eq/eq.h>

class Node : public eq::Node
{
public:
    const Model* getModel() const { return _model; }

protected:
    virtual bool init( const uint32_t initID );
    virtual bool exit();

private:
    Model* _model;    
};

#endif // EQ_PLY_NODE_H
