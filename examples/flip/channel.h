
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EX_CHANNEL_H
#define EQ_EX_CHANNEL_H

#include "flip.h"

#include <eq/eq.h>

class FrameData;

class Channel : public eq::Channel
{
public:
    Channel() : _frameData(NULL) {}

    virtual bool init( const uint32_t initID );
    virtual void exit();
    virtual void draw( const uint32_t frameID );

private:
    FrameData* _frameData;
};




#endif // EQ_EX_CHANNEL_H

