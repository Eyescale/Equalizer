
/* 
 * Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 *
 * The pipe object is responsible for maintaining GPU-specific and
 * frame-specific data. The identifier passed by the application contains the
 * version of the frame data corresponding to the rendered frame. The pipe's
 * start frame callback synchronizes the thread-local instance of the frame data
 * to this version.
 */

#include "pipe.h"
#include "node.h"
#include "config.h"

#include <eq/eq.h>

using namespace eq::base;
using namespace std;

namespace eVolve
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

    const string&  filename  = initData.getFilename();
    const uint32_t precision = initData.getPrecision();
    EQINFO << "Loading model " << filename << endl;

    _renderer = new Renderer( filename.c_str(), precision );
    EQASSERT( _renderer );

    if( !_renderer->loadHeader( initData.getBrightness(), initData.getAlpha( )))
    {
        setErrorMessage( "Can't load model header file" );
        EQWARN << "Can't load model header file: " << filename << ".vhf"
               << endl;
        delete _renderer;
        _renderer = 0;
        return false;
    }

    return mapped;
}

bool Pipe::configExit()
{
    delete _renderer;
    _renderer = 0;

    eq::Config* config = getConfig();
    config->unmapObject( &_frameData );

    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    eq::Pipe::frameStart( frameID, frameNumber );
    _frameData.sync( frameID );

    _renderer->setOrtho( _frameData.useOrtho( ));
}
}
