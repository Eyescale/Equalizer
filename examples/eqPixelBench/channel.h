
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELBENCH_CHANNEL_H
#define EQ_PIXELBENCH_CHANNEL_H

#include <eq/eq.h>

namespace eqPixelBench
{
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent ) : eq::Channel( parent ) {}

protected:
    virtual void frameDraw( const uint32_t frameID );
};
}

#endif // EQ_PIXELBENCH_CHANNEL_H

