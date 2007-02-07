
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_CONFIG_H
#define EQ_PLY_CONFIG_H

#include <eq/eq.h>

#include "appInitData.h"
#include "frameData.h"

class Config : public eq::Config
{
public:
    Config();

    bool isRunning() const { return _running; }
    
    /** @sa eq::Config::init. */
    virtual bool init();
    /** @sa eq::Config::exit. */
    virtual bool exit();

    /** @sa eq::Config::startFrame. */
    virtual uint32_t startFrame();

    AppInitData& getInitData() { return _initData; }

protected:
    virtual ~Config();

    /** @sa eq::Config::handleEvent */
    virtual bool handleEvent( const eq::ConfigEvent* event );

    bool       _running;
    int        _spinX, _spinY;

    AppInitData _initData;
    FrameData   _frameData;

private:
    static void _applyRotation( float m[16], const float dx, const float dy );
};

#endif // EQ_PLY_CONFIG_H
