
/* 
 * Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved.
 */

#include "pipe.h"

#include "config.h"
#include <eq/eq.h>

using namespace eq::base;
using namespace std;

namespace eqPly
{
eq::WindowSystem Pipe::selectWindowSystem() const
{
    const Config*          config   = static_cast<const Config*>( getConfig( ));
    const InitData&        initData = config->getInitData();
    const eq::WindowSystem ws       = initData.getWindowSystem();

    if( ws == eq::WINDOW_SYSTEM_NONE )
        return eq::Pipe::selectWindowSystem();
    if( !supportsWindowSystem( ws ))
    {
        EQWARN << "Window system " << ws 
               << " not supported, using default window system" << endl;
        return eq::Pipe::selectWindowSystem();
    }

    return ws;
}

bool Pipe::configInit( const uint32_t initID )
{
    if( !eq::Pipe::configInit( initID ))
        return false;

    Config*         config      = static_cast<Config*>( getConfig( ));
    const InitData& initData    = config->getInitData();
    const uint32_t  frameDataID = initData.getFrameDataID();

    const bool mapped = config->mapObject( &_frameData, frameDataID );
    EQASSERT( mapped );
    return mapped;
}

bool Pipe::configExit()
{
    eq::Config* config = getConfig();
    config->unmapObject( &_frameData );

    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    eq::Pipe::frameStart( frameID, frameNumber );
    _frameData.sync( frameID );
}
}
