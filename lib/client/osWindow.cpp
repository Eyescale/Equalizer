/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#include "osWindow.h"

using namespace std;

namespace eq
{

OSWindow::OSWindow( Window* parent )
    : _window( parent )
    , _glewContext( new GLEWContext )
    , _fbo( 0 )
    , _glewInitialized( false )
{
    EQASSERT( _window ); 
}

OSWindow::~OSWindow()
{
    delete _glewContext;
    _glewContext = 0;
}

void OSWindow::_initGlew()
{
	const GLenum result = glewInit();
    if( result != GLEW_OK )
		EQWARN << "GLEW initialization failed with error " << result <<endl;
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
    
	if( _fbo->init( pvp.w, pvp.h, getIAttribute( Window::IATTR_PLANES_DEPTH ),
                    getIAttribute( Window::IATTR_PLANES_STENCIL ) ) )
    {
		return true;
	}
	
	delete _fbo;
	_fbo = 0;
	return false;
}

bool OSWindow::configExitFBO()
{	
    if( !_glewInitialized ||
       !GLEW_ARB_texture_non_power_of_two ||
       !GLEW_EXT_framebuffer_object )
    {
        return false;
    }
    
    if ( _fbo )
    {
        delete _fbo;
    }
    
    return (_fbo == 0);
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
