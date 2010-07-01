
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include <eq/util/compressorDataGPU.h>
using namespace std;

namespace eVolve
{

bool Window::configInit( const uint32_t initID )
{
    // Enforce alpha channel, since we need one for rendering
    setIAttribute( IATTR_PLANES_ALPHA, 8 );

    return eq::Window::configInit( initID );
}

bool Window::configInitGL( const uint32_t initID )
{
    Pipe*     pipe     = static_cast<Pipe*>( getPipe() );
    Renderer* renderer = pipe->getRenderer();

    if( !renderer )
        return false;

    if( !GLEW_ARB_shader_objects )
    {
        setErrorMessage( "eVolve needs GL_ARB_shader_objects extension" );
        return false;
    }
    if( !GLEW_EXT_blend_func_separate )
    {
        setErrorMessage( "eVolve needs GL_EXT_blend_func_separate extension" );
        return false;
    }
    if( !GLEW_ARB_multitexture )
    {
        setErrorMessage( "eVolve needs GLEW_ARB_multitexture extension" );
        return false;
    }

    glEnable( GL_SCISSOR_TEST ); // needed to constrain channel viewport

    glClear( GL_COLOR_BUFFER_BIT );
    swapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    renderer->glewSetContext( glewGetContext( ));

    if( !renderer->loadShaders( ))
    {
        setErrorMessage( "Can't load shaders" );
        return false;
    }

    _loadLogo();
    return true;
}

static const char* _logoTextureName = "eVolve_logo";

void Window::_loadLogo()
{
    eq::Window::ObjectManager* objects = getObjectManager();

    if( objects->getTexture( _logoTextureName ) != 
        eq::Window::ObjectManager::INVALID )
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
        !image.readImage( "../examples/eqVolve/logo.rgb",
                          eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "./examples/eqVolve/logo.rgb", 
                          eq::Frame::BUFFER_COLOR ))
    {
        EQWARN << "Can't load overlay logo 'logo.rgb'" << endl;
        return;
    }

    _logoTexture = objects->newTexture( _logoTextureName );
    EQASSERT( _logoTexture != eq::Window::ObjectManager::INVALID );

    const eq::PixelViewport& pvp = image.getPixelViewport();
    _logoSize.x() = pvp.w;
    _logoSize.y() = pvp.h;
    const uint32_t externalFormat = 
        image.getExternalFormat( eq::Frame::BUFFER_COLOR );

    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _logoTexture );
    glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0,
                  image.getInternalFormat( eq::Frame::BUFFER_COLOR ),
                  _logoSize.x(), _logoSize.y(), 0,
                  eq::util::CompressorDataGPU::getGLFormat( externalFormat ), 
                  eq::util::CompressorDataGPU::getGLType( externalFormat ),
                  image.getPixelPointer( eq::Frame::BUFFER_COLOR ));

    EQINFO << "Created logo texture of size " << _logoSize << endl;
}


void Window::swapBuffers()
{
    const Pipe*         pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData&    frameData = pipe->getFrameData();
    const eq::Channels& channels  = getChannels();

    if( frameData.useStatistics() && !channels.empty( ))
        EQ_GL_CALL( channels.back()->drawStatistics( ));

    eq::Window::swapBuffers();
}
}
