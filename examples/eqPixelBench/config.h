
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIXELBENCH_CONFIG_H
#define EQ_PIXELBENCH_CONFIG_H

#include <eq/eq.h>

/** The Equalizer Pixel Transfer Benchmark Utility */
namespace eqPixelBench
{
class Config : public eq::Config
{
public:
    Config();

    /** @sa eq::Config::startFrame */
    virtual uint32_t startFrame( const uint32_t frameID );

    /** @sa eq::Config::handleEvent */
    virtual bool handleEvent( const eq::ConfigEvent* event );

    /** @return the clock started by startFrame, or 0 on render clients. */
    const eqBase::Clock* getClock() const { return _clock; }

protected:
    virtual ~Config();

private:
    eqBase::Clock* _clock;
};
}

#endif // EQ_PIXELBENCH_CONFIG_H
