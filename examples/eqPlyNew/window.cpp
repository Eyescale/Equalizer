
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"
#include "pipe.h"
#include "node.h"

using namespace std;

namespace eqPly
{

bool Window::configInit( const uint32_t initID )
{
    if( !eq::Window::configInit( initID ))
        return false;

    eq::Pipe*  pipe        = getPipe();
    Window*    firstWindow = static_cast< Window* >( pipe->getWindow( 0 ));

    EQASSERT( !_state );

    const eq::GLFunctions* glFunctions = getGLFunctions();

    if( firstWindow == this )
    {
        _state = new VertexBufferState( glFunctions );

        const Node*     node     = static_cast< const Node* >( getNode( ));
        const InitData& initData = node->getInitData();

        if( initData.useVBOs() )
        {
            // Check if all VBO funcs available, else leave DISPLAY_LIST_MODE on
            if( glFunctions->hasGenBuffers() && glFunctions->hasBindBuffer() &&
                glFunctions->hasBufferData() && glFunctions->hasDeleteBuffers())
            {
                _state->setRenderMode( mesh::BUFFER_OBJECT_MODE );
                EQINFO << "VBO rendering enabled" << endl;
            }
            else
                EQWARN << "VBO function pointers missing, using display lists" 
                       << endl;
        }
        
        if( initData.useShaders() )
        {
            // Check if all shader functions are available
            if( glFunctions->hasCreateShader() && 
                glFunctions->hasDeleteShader() &&
                glFunctions->hasShaderSource() &&
                glFunctions->hasCompileShader() &&
                glFunctions->hasGetShaderiv() &&
                glFunctions->hasCreateProgram() && 
                glFunctions->hasDeleteProgram() &&
                glFunctions->hasAttachShader() &&
                glFunctions->hasLinkProgram() && 
                glFunctions->hasGetProgramiv() &&
                glFunctions->hasUseProgram() )
            {
                EQINFO << "Shaders supported, attempting to load" << endl;
                _loadShaders();
            }
            else
                EQWARN << "Shader function pointers missing, using fixed "
                       << "pipeline" << endl;
        }

        _loadLogo();
    }
    else
    {
        _state       = firstWindow->_state;
        _logoTexture = firstWindow->_logoTexture;
        _logoSize    = firstWindow->_logoSize;
    }

    if( !_state ) // happens if first window failed to initialize
    {
        setErrorMessage( "No state handler object" );
        return false;
    }

    const GLuint program = _state->getProgram( getPipe() );
    if( program != VertexBufferState::FAILED )
        glFunctions->useProgram( program );

    return true;
}

bool Window::configExit()
{
    if( _state.isValid() && _state->getRefCount() == 1 )
        _state->deleteAll();

    _state = 0;
    return eq::Window::configExit();
}

static const char* _logoTextureName = "eqPly_logo";

void Window::_loadLogo()
{
    EQASSERT( _state->getTexture( _logoTextureName ) == 
              VertexBufferState::FAILED );

    eq::Image image;
    if( !image.readImage( "logo.rgb", eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "examples/eqPly/logo.rgb", eq::Frame::BUFFER_COLOR ))
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

static const char* _vertexShader = 
    "/*  Pass-through vertex shader.  */\n"
    "void main()\n"
    "{\n"
    "    gl_Position = ftransform();\n"
    "}";

static const char* _fragmentShader = 
    "/*  Dummy fragment shader.  */\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );\n"
    "}";

void Window::_loadShaders()
{
    const eq::GLFunctions* glFunctions = getGLFunctions();
    GLint status;
    
    const GLuint vShader = 
        _state->newShader( _vertexShader, GL_VERTEX_SHADER );
    EQASSERT( vShader != VertexBufferState::FAILED );
    glFunctions->shaderSource( vShader, 1, &_vertexShader, 0 );
    glFunctions->compileShader( vShader );
    glFunctions->getShaderiv( vShader, GL_COMPILE_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to compile vertex shader" << endl;
        return;
    }
    
    const GLuint fShader = 
        _state->newShader( _fragmentShader, GL_FRAGMENT_SHADER );
    EQASSERT( fShader != VertexBufferState::FAILED );
    glFunctions->shaderSource( fShader, 1, &_fragmentShader, 0 );
    glFunctions->compileShader( fShader );
    glFunctions->getShaderiv( fShader, GL_COMPILE_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to compile fragment shader" << endl;
        return;
    }
    
    const GLuint program = _state->newProgram( getPipe() );
    EQASSERT( program != VertexBufferState::FAILED );
    glFunctions->attachShader( program, vShader );
    glFunctions->attachShader( program, fShader );
    glFunctions->linkProgram( program );
    glFunctions->getProgramiv( program, GL_LINK_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to link shader program" << endl;
        return;
    }
    
    EQINFO << "Shaders loaded successfully" << endl;
}

}
