
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"
#include "pipe.h"

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
        _objects = new ObjectManager;
        _loadLogo();
    }
    else
    {
        _objects     = firstWindow->_objects;
        _logoTexture = firstWindow->_logoTexture;
        _logoSize    = firstWindow->_logoSize;
    }

    EQASSERT( _objects );

    return true;
}

bool Window::configExit()
{
    if( _objects.isValid() && _objects->getRefCount() == 1 )
        _objects->deleteAll();

    _objects = 0;
    return eq::Window::configExit();
}

void Window::_loadLogo()
{
    EQASSERT( !_objects->getTexture( "eqPly_logo" ));

    eq::Image image;
    if( !image.readImage( "logo.rgb", eq::Frame::BUFFER_COLOR ) &&
        !image.readImage( "examples/eqPly/logo.rgb", eq::Frame::BUFFER_COLOR ))
    {
        EQWARN << "Can't load overlay logo 'logo.rgb'" << endl;
        return;
    }

    _logoTexture = _objects->newTexture( "eqPly_logo" );
    EQASSERT( _logoTexture );

    const eq::PixelViewport& pvp = image.getPixelViewport();
    _logoSize.x = pvp.w;
    _logoSize.y = pvp.h;

    glBindTexture( GL_TEXTURE_RECTANGLE_NV, _logoTexture );
    glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, 
                  image.getFormat( eq::Frame::BUFFER_COLOR ),
                  _logoSize.x, _logoSize.y, 0,
                  image.getFormat( eq::Frame::BUFFER_COLOR ), 
                  image.getType( eq::Frame::BUFFER_COLOR ),
                  image.getPixelData( eq::Frame::BUFFER_COLOR ));

    EQINFO << "Created logo texture of size " << _logoSize << endl;
}

}
