
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2007, Tobias Wolf <twolf@access.unizh.ch>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "window.h"
#include "pipe.h"
#include "config.h"

#include "fragmentShader.glsl.h"
#include "vertexShader.glsl.h"
    
#include <fstream>
#include <sstream>

namespace eqPly
{

bool Window::configInitSystemWindow( const eq::uint128_t& initID )
{
#ifndef Darwin
    if( !eq::Window::configInitSystemWindow( initID ))
        return false;

    // OpenGL version is less than 2.0.
    if( !GLEW_EXT_framebuffer_object)
    {
        if( getDrawableConfig().accumBits )
            return true;

        configExitSystemWindow();
#endif

        // try with 64 bit accum buffer
        setIAttribute( IATTR_PLANES_ACCUM, 16 );
        if( eq::Window::configInitSystemWindow( initID ))
            return true;

        // no anti-aliasing possible
        setIAttribute( IATTR_PLANES_ACCUM, eq::AUTO );

        return eq::Window::configInitSystemWindow( initID );

#ifndef Darwin
    }

    return true;
#endif
}

bool Window::configInitGL( const eq::uint128_t& initID )
{
    if( !eq::Window::configInitGL( initID ))
        return false;

    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, 1 );
    glEnable( GL_CULL_FACE ); // OPT - produces sparser images in DB mode
    glCullFace( GL_BACK );

    EQASSERT( !_state );
    _state = new VertexBufferState( getObjectManager( ));

    const Config*   config   = static_cast< const Config* >( getConfig( ));
    const InitData& initData = config->getInitData();

    if( initData.showLogo( ))
        _loadLogo();

    if( initData.useGLSL() )
        _loadShaders();

    return true;
}

bool Window::configExitGL()
{
    if( _state && !_state->isShared( ))
        _state->deleteAll();

    delete _state;
    _state = 0;

    return eq::Window::configExitGL();
}

static const char* _logoTextureName = "eqPly_logo";

void Window::_loadLogo()
{
    eq::Window::ObjectManager* om = getObjectManager();
    _logoTexture = om->getEqTexture( _logoTextureName );
    if( _logoTexture )
        return;

    eq::Image image;
    if( !image.readImage( "logo.rgb", eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "../examples/eqPly/logo.rgb",
                          eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "./examples/eqPly/logo.rgb", 
                          eq::Frame::BUFFER_COLOR ))
    {
        EQWARN << "Can't load overlay logo 'logo.rgb'" << std::endl;
        return;
    }

    _logoTexture = om->newEqTexture(_logoTextureName, GL_TEXTURE_RECTANGLE_ARB);
    EQASSERT( _logoTexture );
    
    image.upload(eq::Frame::BUFFER_COLOR, _logoTexture, eq::Vector2i::ZERO, om);
    EQVERB << "Created logo texture of size " << _logoTexture->getWidth() << "x"
           << _logoTexture->getHeight() << std::endl;
}

void Window::_loadShaders()
{
    if( _state->getShader( vertexShader_glsl ) !=
        VertexBufferState::INVALID )
    {
        // already loaded
        return;
    }

    // Check if functions are available
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "Shader function pointers missing, using fixed function "
               << "pipeline" << std::endl;
        return;
    }

    const GLuint vShader = _state->newShader( vertexShader_glsl,
                                              GL_VERTEX_SHADER );
    EQASSERT( vShader != VertexBufferState::INVALID );
    const GLchar* vShaderPtr = vertexShader_glsl;
    glShaderSource( vShader, 1, &vShaderPtr, 0 );
    glCompileShader( vShader );

    GLint status;
    glGetShaderiv( vShader, GL_COMPILE_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to compile vertex shader" << std::endl;
        return;
    }
    
    const GLuint fShader = 
        _state->newShader( fragmentShader_glsl, GL_FRAGMENT_SHADER );
    EQASSERT( fShader != VertexBufferState::INVALID );
    const GLchar* fShaderPtr = fragmentShader_glsl;
    glShaderSource( fShader, 1, &fShaderPtr, 0 );
    glCompileShader( fShader );
    glGetShaderiv( fShader, GL_COMPILE_STATUS, &status );
    if( !status )
    {
        EQWARN << "Failed to compile fragment shader" << std::endl;
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
        EQWARN << "Failed to link shader program" << std::endl;
        return;
    }
    
    // turn off OpenGL lighting if we are using our own shaders
    glDisable( GL_LIGHTING );

    EQINFO << "Shaders loaded successfully" << std::endl;
}

void Window::frameStart( const eq::uint128_t& frameID, const uint32_t frameNumber )
{
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    _state->setRenderMode( frameData.getRenderMode( ));
    eq::Window::frameStart( frameID, frameNumber );
}

}
