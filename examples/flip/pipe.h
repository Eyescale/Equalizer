
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FLIP_PIPE_H
#define EQ_FLIP_PIPE_H

#include <eq/eq.h>

#include "frameData.h"

class Pipe : public eq::Pipe
{
public:
    Pipe() : _frameData(NULL) {}

    FrameData* getFrameData() const { return _frameData; }
    
protected:
    bool init( const uint32_t initID );

private:
    FrameData* _frameData;
};

#endif // EQ_FLIP_PIPE_H
