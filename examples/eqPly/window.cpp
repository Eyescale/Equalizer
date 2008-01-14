
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
   All rights reserved. */

#include "window.h"
#include "pipe.h"
#include "node.h"

#include "fragmentShader_glsl.h"
#include "vertexShader_glsl.h"
    
#include <fstream>
#include <sstream>

using namespace std;

namespace eqPly
{

bool Window::configInit( const uint32_t initID )
{
    if( !eq::Window::configInit( initID ))
        return false;

    EQASSERT( !_state );
    _state = new VertexBufferState( getObjectManager( ));

    const Node*     node     = static_cast< const Node* >( getNode( ));
    const InitData& initData = node->getInitData();

    if( initData.useVBOs() )
    {
        // Check if VBO funcs available, else leave DISPLAY_LIST_MODE on
        if( GLEW_VERSION_1_5 )
        {
            _state->setRenderMode( mesh::BUFFER_OBJECT_MODE );
            EQINFO << "VBO rendering enabled" << endl;
        }
        else
            EQWARN << "VBO function pointers missing, using display lists" 
                   << endl;
    }

    _loadLogo();

    if( initData.useGLSL() )
        _loadShaders();

    return true;
}

bool Window::configExit()
{
    if( _state )
        _state->deleteAll();

    delete _state;
    _state = 0;

    return eq::Window::configExit();
}

static const char* _logoTextureName = "eqPly_logo";

void Window::_loadLogo()
{
    if( _state->getTexture( _logoTextureName ) != VertexBufferState::FAILED )
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
    EQASSERT( _logoTexture != VertexBufferState::FAILED );

    const eq::PixelViewport& pvp = image.getPixelViewport();
    _logoSize.x = pvp.w;
    _logoSize.y = pvp.h;

    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _logoTexture );
    glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                  image.getFormat( eq::Frame::BUFFER_COLOR ),
                  _logoSize.x, _logoSize.y, 0,
                  image.getFormat( eq::Frame::BUFFER_COLOR ), 
                  image.getType( eq::Frame::BUFFER_COLOR ),
                  image.getPixelData( eq::Frame::BUFFER_COLOR ));

    EQINFO << "Created logo texture of size " << _logoSize << endl;
}

void Window::_loadShaders()
{
    if( _state->getShader( vertexShader_glsl.c_str( )) !=
        VertexBufferState::FAILED )

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
    EQASSERT( vShader != VertexBufferState::FAILED );
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
    EQASSERT( fShader != VertexBufferState::FAILED );
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
    EQASSERT( program != VertexBufferState::FAILED );
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

}
