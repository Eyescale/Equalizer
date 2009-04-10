
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "osWindow.h"

#include "frameBufferObject.h"
#include "global.h"
#include "pipe.h"

using namespace std;

namespace eq
{

OSWindow::OSWindow( Window* parent )
    : _window( parent )
    , _glewContext( new GLEWContext )
    , _glewInitialized( false )
    , _fbo( 0 )
{
    EQASSERT( _window ); 
}

OSWindow::~OSWindow()
{
    delete _glewContext;
    _glewContext = 0;
    _glewInitialized = false;
}

const Pipe* OSWindow::getPipe() const
{
    EQASSERT( _window );
    return _window->getPipe();
}
Pipe* OSWindow::getPipe()
{
    EQASSERT( _window );
    return _window->getPipe();
}

const Node* OSWindow::getNode() const
{
    EQASSERT( _window );
    return _window->getNode();
}
Node* OSWindow::getNode()
{
    EQASSERT( _window );
    return _window->getNode();
}

const Config* OSWindow::getConfig() const
{
    EQASSERT( _window );
    return _window->getConfig();
}
Config* OSWindow::getConfig()
{
    EQASSERT( _window );
    return _window->getConfig();
}

int32_t OSWindow::getIAttribute( const Window::IAttribute attr ) const
{
    EQASSERT( _window );
    return _window->getIAttribute( attr );
}

WGLEWContext* OSWindow::wglewGetContext()
{
    EQASSERT( _window );
    return _window->wglewGetContext();
}

void OSWindow::_initGlew()
{
    const GLenum result = glewInit();
    if( result != GLEW_OK )
        _window->setErrorMessage( "GLEW initialization failed: " + result );
    else
        _glewInitialized = true;
}
    
bool OSWindow::configInitFBO()
{
    if( !_glewInitialized ||
        !GLEW_ARB_texture_non_power_of_two ||
        !GLEW_EXT_framebuffer_object )
    {
        _window->setErrorMessage( "Framebuffer objects unsupported" );
         return false;
    }
    
    // needs glew initialized (see above)
    _fbo = new FrameBufferObject( _glewContext, _window->getColorType());
    
    const PixelViewport& pvp = _window->getPixelViewport();
    
    int depthSize = getIAttribute( Window::IATTR_PLANES_DEPTH );
    if( depthSize == AUTO )
         depthSize = 24;

    int stencilSize = getIAttribute( Window::IATTR_PLANES_STENCIL );
    if( stencilSize == AUTO )
        stencilSize = 1;

    if( _fbo->init( pvp.w, pvp.h, depthSize, stencilSize ) )
        return true;
    
    _window->setErrorMessage( "FBO initialization failed: " + 
                              _fbo->getErrorMessage( ));
    delete _fbo;
    _fbo = 0;
    return false;
}

void OSWindow::configExitFBO()
{   
    if( _fbo )
        _fbo->exit();

    delete _fbo;
    _fbo = 0;
}

void OSWindow::makeCurrent() const 
{
    bindFrameBuffer();
    getPipe()->setCurrent( _window );
}

void OSWindow::bindFrameBuffer() const 
{
   if( !_glewInitialized )
       return;
    
   if( _fbo )
       _fbo->bind();
   else if( GLEW_EXT_framebuffer_object )
       glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

}
