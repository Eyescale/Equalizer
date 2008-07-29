
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELBENCH_WINDOW_H
#define EQ_PIXELBENCH_WINDOW_H

#include <eq/eq.h>

namespace eqPixelBench
{
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent ) : eq::Window( parent ) {}

    protected:
        virtual bool configInit( const uint32_t initID );
    };
}

#endif // EQ_PIXELBENCH_WINDOW_H
