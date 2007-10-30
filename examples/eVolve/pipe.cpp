
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

#ifdef CG_INSTALLED
#  include "vertexShaderComplex_cg.h"
#  include "vertexShaderSimple_cg.h"
#  include "fragmentShader_cg.h"
#endif
#include "fragmentShader_glsl.h"
#include "vertexShader_glsl.h"
    
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


    const string& filename = initData.getFilename();
    EQINFO << "Loading model " << filename << endl;

    _model = new Model( filename.c_str() );
    EQASSERT( _model );

    if( !_model->loadHeader( initData.getBrightness(), initData.getAlpha( )))
    {
        setErrorMessage( "Can't load model header file" );
        EQWARN << "Can't load model header file: " << filename << ".vhf"
               << endl;
        delete _model;
        _model = 0;
        return false;
    }

    return eq::Pipe::configInit( initID );
}

bool Pipe::configExit()
{
    delete _model;
    _model = 0;

    eq::Config* config = getConfig();
    config->unmapObject( &_frameData );

    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    // don't wait for node to start frame, local sync not needed
    // node->waitFrameStarted( frameNumber );
    _frameData.sync( frameID );
    startFrame( frameNumber );
}

void Pipe::LoadShaders()
{
    if( !_shadersLoaded )
    {
        const Node*     node     = static_cast<Node*>( getNode( ));
        const InitData& initData = node->getInitData();
        const bool      useCg    = !initData.useGLSL();

#ifdef CG_INSTALLED
        if( useCg )
        {
            EQLOG( eq::LOG_CUSTOM ) << "using Cg shaders" << endl;
            _cgContext = cgCreateContext();

            if( _shaders.cgVertex )
                delete _shaders.cgVertex; 

            _shaders.cgVertex = new gloo::cg_program( _cgContext );
            try
            {
                _shaders.cgVertex->create_from_string( CG_GL_VERTEX , 
                                                       vertexShaderComplex_cg );
                _useSimpleShader = false;
            }catch(...)
            {
                _shaders.cgVertex->create_from_string( CG_GL_VERTEX, 
                                                       vertexShaderSimple_cg );
                EQWARN  << "Can't load normal vertex shader, using smaller, "
                        << "but slower vertex shader" << std::endl;
                _useSimpleShader = true;
            }

            if( _shaders.cgFragment )
                delete _shaders.cgFragment;

            _shaders.cgFragment = new gloo::cg_program( _cgContext );
            _shaders.cgFragment->create_from_string( CG_GL_FRAGMENT, 
                                                     fragmentShader_cg );
        }
        else
#endif
        {
            EQLOG( eq::LOG_CUSTOM ) << "using glsl shaders" << endl;
            if( !eqShader::loadShaders( vertexShader_glsl, fragmentShader_glsl,
                                        _shader ))
            {
                EQERROR << "Can't load glsl shaders" << endl;
                return;
            }
        }

        _shadersLoaded = true;
        EQLOG( eq::LOG_CUSTOM ) << "shaders loaded" << endl;
    }
}

}
