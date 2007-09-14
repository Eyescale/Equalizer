
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"
#include "pipe.h"

using namespace std;

namespace eqPly
{

bool Window::configInit( const uint32_t initID )
{
    if( !eq::Window::configInit( initID ))
        return false;

    eq::Pipe*  pipe        = getPipe();
    Window*    firstWindow = static_cast< Window* >( pipe->getWindow( 0 ));

    EQASSERT( !_objects );

    if( firstWindow == this )
    {
        _objects = new ObjectManager( getGLFunctions( ));
        _state = new VertexBufferStateSimple;
        _loadLogo();
    }
    else
    {
        _objects     = firstWindow->_objects;
        _state       = firstWindow->_state;
        _logoTexture = firstWindow->_logoTexture;
        _logoSize    = firstWindow->_logoSize;
    }

    if( !_objects ) // happens if first window failed to initialize
        return false;

    return true;
}

bool Window::configExit()
{
    if( _objects.isValid() && _objects->getRefCount() == 1 )
        _objects->deleteAll();

    _objects = 0;
    _state = 0;
    return eq::Window::configExit();
}

static const char* _logoTextureName = "eqPly_logo";

void Window::_loadLogo()
{
    EQASSERT( _objects->getTexture( _logoTextureName ) == 
              ObjectManager::FAILED );

    eq::Image image;
    if( !image.readImage( "logo.rgb", eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "examples/eqPly/logo.rgb", eq::Frame::BUFFER_COLOR ))
    {
        EQWARN << "Can't load overlay logo 'logo.rgb'" << endl;
        return;
    }

    _logoTexture = _objects->newTexture( _logoTextureName );
    EQASSERT( _logoTexture != ObjectManager::FAILED );

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

}
