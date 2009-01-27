
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
   All rights reserved. */

#include "window.h"
#include "pipe.h"
#include "config.h"

#include "fragmentShader_glsl.h"
#include "vertexShader_glsl.h"
    
#include <fstream>
#include <sstream>

using namespace std;

namespace eqPly
{

bool Window::configInitGL( const uint32_t initID )
{
    if( !eq::Window::configInitGL( initID ))
        return false;

    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, 1 );
    glEnable( GL_CULL_FACE ); // OPT - produces sparser images in DB mode
    glCullFace( GL_BACK );

    EQASSERT( !_state );
    _state = new VertexBufferState( getObjectManager( ));
    _loadLogo();

    const Config*   config   = static_cast< const Config* >( getConfig( ));
    const InitData& initData = config->getInitData();
    if( initData.useGLSL() )
        _loadShaders();

    return true;
}

bool Window::configExitGL()
{
    if( _state )
        _state->deleteAll();

    delete _state;
    _state = 0;

    return eq::Window::configExitGL();
}

static const char* _logoTextureName = "eqPly_logo";

void Window::_loadLogo()
{
    if( _state->getTexture( _logoTextureName ) != VertexBufferState::INVALID )
    {
        // Already loaded by first window
        const eq::Pipe* pipe        = getPipe();
        const Window*   firstWindow = static_cast< Window* >
                                          ( pipe->getWindows()[0] );
        
        _logoTexture = firstWindow->_logoTexture;
        _logoSize    = firstWindow->_logoSize;
        return;
    }

    eq::Image image;
    if( !image.readImage( "logo.rgb", eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "./examples/eqPly/logo.rgb", 
                          eq::Frame::BUFFER_COLOR ) )
    {
        EQWARN << "Can't load overlay logo 'logo.rgb'" << endl;
        return;
    }

    _logoTexture = _state->newTexture( _logoTextureName );
    EQASSERT( _logoTexture != VertexBufferState::INVALID );

    const eq::PixelViewport& pvp = image.getPixelViewport();
    _logoSize.x = pvp.w;
    _logoSize.y = pvp.h;

    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _logoTexture );
    glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                  image.getFormat( eq::Frame::BUFFER_COLOR ),
                  _logoSize.x, _logoSize.y, 0,
                  image.getFormat( eq::Frame::BUFFER_COLOR ), 
                  image.getType( eq::Frame::BUFFER_COLOR ),
                  image.getPixelPointer( eq::Frame::BUFFER_COLOR ));

    EQINFO << "Created logo texture of size " << _logoSize << endl;
}

void Window::_loadShaders()
{
    if( _state->getShader( vertexShader_glsl.c_str( )) !=
        VertexBufferState::INVALID )

        // already loaded
        return;

    // Check if functions are available
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "Shader function pointers missing, using fixed function "
               << "pipeline" << endl;
        return;
    }

    const GLuint vShader = _state->newShader( vertexShader_glsl.c_str(), 
                                              GL_VERTEX_SHADER );
    EQASSERT( vShader != VertexBufferState::INVALID );
    const GLchar* vShaderPtr = vertexShader_glsl.c_str();
    glShaderSource( vShader, 1, &vShaderPtr, 0 );
    glCompileShader( vShader );

    GLint status;
    glGetShaderiv( vShader, GL_COMPILE_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to compile vertex shader" << endl;
        return;
    }
    
    const GLuint fShader = 
        _state->newShader( fragmentShader_glsl.c_str(), GL_FRAGMENT_SHADER );
    EQASSERT( fShader != VertexBufferState::INVALID );
    const GLchar* fShaderPtr = fragmentShader_glsl.c_str();
    glShaderSource( fShader, 1, &fShaderPtr, 0 );
    glCompileShader( fShader );
    glGetShaderiv( fShader, GL_COMPILE_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to compile fragment shader" << endl;
        return;
    }
    
    const GLuint program = _state->newProgram( getPipe() );
    EQASSERT( program != VertexBufferState::INVALID );
    glAttachShader( program, vShader );
    glAttachShader( program, fShader );
    glLinkProgram( program );
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to link shader program" << endl;
        return;
    }
    
    // turn off OpenGL lighting if we are using our own shaders
    glDisable( GL_LIGHTING );

    EQINFO << "Shaders loaded successfully" << endl;
}

void Window::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    const Pipe*            pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData::Data& frameData = pipe->getFrameData();

    _state->setRenderMode( frameData.renderMode );
    eq::Window::frameStart( frameID, frameNumber );
}

void Window::swapBuffers()
{
    const Pipe*              pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData::Data&   frameData = pipe->getFrameData();
    const eq::ChannelVector& channels  = getChannels();

    if( frameData.statistics && !channels.empty( ))
        EQ_GL_CALL( channels.back()->drawStatistics( ));

    eq::Window::swapBuffers();
}
}
