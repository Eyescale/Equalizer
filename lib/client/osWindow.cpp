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
{
    EQASSERT( _window ); 
}

OSWindow::~OSWindow()
{
    delete _glewContext;
    _glewContext = 0;
}
    
void OSWindow::_initializeGLData()
{
    makeCurrent();
    _queryDrawableConfig();
    const GLenum result = glewInit();
    if( result != GLEW_OK )
        EQWARN << "GLEW initialization failed with error " << result <<endl;

    _setupObjectManager();
}

void OSWindow::_clearGLData()
{
    _releaseObjectManager();
}


void OSWindow::_queryDrawableConfig()
{
    // GL version
    const char* glVersion = (const char*)glGetString( GL_VERSION );
    if( !glVersion ) // most likely no context - fail
    {
        EQWARN << "glGetString(GL_VERSION) returned 0, assuming GL version 1.1" 
            << endl;
        _drawableConfig.glVersion = 1.1f;
    }
    else
        _drawableConfig.glVersion = static_cast<float>( atof( glVersion ));

    // Framebuffer capabilities
    GLboolean result;
    glGetBooleanv( GL_STEREO,       &result );
    _drawableConfig.stereo = result;

    glGetBooleanv( GL_DOUBLEBUFFER, &result );
    _drawableConfig.doublebuffered = result;

    GLint stencilBits;
    glGetIntegerv( GL_STENCIL_BITS, &stencilBits );
    _drawableConfig.stencilBits = stencilBits;

    GLint alphaBits;
    glGetIntegerv( GL_ALPHA_BITS, &alphaBits );
    _drawableConfig.alphaBits = alphaBits;

    EQINFO << "Window drawable config: " << _drawableConfig << endl;
}

void OSWindow::_setupObjectManager()
{
    _releaseObjectManager();

    Window* sharedWindow = _window->getSharedContextWindow();
    if( sharedWindow )
        _objectManager = sharedWindow->getObjectManager();

    if( !_objectManager.isValid( ))
        _objectManager = new Window::ObjectManager( _glewContext );
}

void OSWindow::_releaseObjectManager()
{
    if( _objectManager.isValid() && _objectManager->getRefCount() == 1 )
        _objectManager->deleteAll();

    _objectManager = 0;
}

}
