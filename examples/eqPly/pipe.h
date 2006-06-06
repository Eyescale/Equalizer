
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_PIPE_H
#define EQ_PLY_PIPE_H

#include <eq/eq.h>

#include "initData.h"

class Pipe : public eq::Pipe
{
public:
    Pipe() : _initData(NULL) {}

protected:
    bool init( const uint32_t initID );
    bool exit();

private:
    eqBase::RefPtr<InitData>  _initData;
};

#endif // EQ_PLY_PIPE_H
