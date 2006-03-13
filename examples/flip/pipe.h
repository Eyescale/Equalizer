
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FLIP_PIPE_H
#define EQ_FLIP_PIPE_H

#include <eq/eq.h>

#include "initData.h"

class Pipe : public eq::Pipe
{
public:
    Pipe() : _initData(NULL) {}

protected:
    bool init( const uint32_t initID );

private:
    InitData* _initData;
};

#endif // EQ_FLIP_PIPE_H
