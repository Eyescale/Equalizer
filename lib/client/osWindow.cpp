/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#include "osWindow.h"

#include "global.h"

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
         return false;
    }
    
    // needs glew initialized (see above)
    _fbo = new FrameBufferObject( _glewContext );
    
    const PixelViewport& pvp = _window->getPixelViewport();
    
    int depthSize = getIAttribute( Window::IATTR_PLANES_DEPTH );
    if( depthSize == AUTO )
         depthSize = 24;

    int stencilSize = getIAttribute( Window::IATTR_PLANES_STENCIL );
    if( stencilSize == AUTO )
        stencilSize = 1;

    if( _fbo->init( pvp.w, pvp.h, depthSize, stencilSize ) )
        return true;
    
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
   if( !_glewInitialized )
       return;
    
   if( _fbo )
       _fbo->bind();
   else
       glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}
}
