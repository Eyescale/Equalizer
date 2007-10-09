
/* 
 * Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved.
 *
 * The pipe object is responsible for maintaining GPU-specific and
 * frame-specific data. The identifier passed by the application contains the
 * version of the frame data corresponding to the rendered frame. The pipe's
 * start frame callback synchronizes the thread-local instance of the frame data
 * to this version.
 */

#include "pipe.h"

#include "node.h"
#include <eq/eq.h>

using namespace eqBase;
using namespace std;

namespace eVolve
{
    
eq::WindowSystem Pipe::selectWindowSystem() const
{
    const Node*            node     = static_cast<Node*>( getNode( ));
    const InitData&        initData = node->getInitData();
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
    const Node*     node        = static_cast<Node*>( getNode( ));
    const InitData& initData    = node->getInitData();
    const uint32_t  frameDataID = initData.getFrameDataID();
    eq::Config*     config      = getConfig();

    const bool mapped = config->mapObject( &_frameData, frameDataID );
    EQASSERT( mapped );

    return eq::Pipe::configInit( initID );
}

bool Pipe::configExit()
{
    eq::Config* config = getConfig();
    config->unmapObject( &_frameData );

    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    _frameData.sync( frameID );
    startFrame( frameNumber );
}

void Pipe::LoadShaders()
{
    if( !_shadersLoaded )
    {
        const Node*     node     = static_cast<Node*>( getNode( ));
        const InitData& initData = node->getInitData();
        const bool      useCg    = initData.useCg();

#ifdef CG_INSTALLED
        if( useCg )
        {
            _cgContext = cgCreateContext();

            if( _shaders.cgVertex )
                delete _shaders.cgVertex; 

            _shaders.cgVertex = new gloo::cg_program( _cgContext );
            _shaders.cgVertex->create_from_file(   CG_GL_VERTEX , 
                                        "./examples/eVolve/vshader.cg" );

            if( _shaders.cgFragment )
                delete _shaders.cgFragment;

            _shaders.cgFragment = new gloo::cg_program( _cgContext );
            _shaders.cgFragment->create_from_file( CG_GL_FRAGMENT, 
                                        "./examples/eVolve/fshader.cg" );
        }
        else
#endif
        {
            if( !eqShader::loadShaders("./examples/eVolve/vshader.oglsl", 
                                       "./examples/eVolve/fshader.oglsl", 
                                        _shader) )
            {
                EQERROR << "Can't load glsl shaders" << endl;
                return;
            }
        
            glUseProgramObjectARB( 0 );
        }    

        _shadersLoaded = true;
        EQLOG( eq::LOG_CUSTOM ) << "shaders loaded" << endl;
    }
}

}
