
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELBENCH_CONFIG_H
#define EQ_PIXELBENCH_CONFIG_H

#include <eq/eq.h>

namespace eqPixelBench
{
class Config : public eq::Config
{
public:
    Config();

    /** @sa eq::Config::handleEvent */
    virtual bool handleEvent( const eq::ConfigEvent* event );

protected:
    virtual ~Config();
};
}

#endif // EQ_PIXELBENCH_CONFIG_H
