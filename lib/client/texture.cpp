
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "texture.h"

#include "image.h"
#include "window.h"

namespace eq
{
Texture::Texture() 
        : _id( Window::ObjectManager::INVALID )
        , _format( 0 )
        , _width(0)
        , _height(0)
        , _defined( false ) 
{
}

Texture::~Texture()
{
    if( _id != Window::ObjectManager::INVALID )
        EQWARN << "OpenGL texture was not freed" << std::endl;
    _id      = Window::ObjectManager::INVALID;
    _defined = false;
}

void Texture::flush()
{
    if( _id == Window::ObjectManager::INVALID )
        return;

    CHECK_THREAD( _thread );
    glDeleteTextures( 1, &_id );
    _id = Window::ObjectManager::INVALID;
    _defined = false;
}

void Texture::setFormat( const GLuint format )
{
    if( _format == format )
        return;

    _defined = false;
    _format  = format;
}

void Texture::_generate()
{
    CHECK_THREAD( _thread );
    if( _id != Window::ObjectManager::INVALID )
        return;

    _defined = false;
    glGenTextures( 1, &_id );
}

void Texture::_resize( const int32_t width, const int32_t height )
{
    if( _width < width ) 
    {
        _width   = width;
        _defined = false;
    }
    
    if( _height < height )
    {
        _height  = height;
        _defined = false;
    }
}

void Texture::copyFromFrameBuffer( const PixelViewport& pvp )
{
    CHECK_THREAD( _thread );
    EQASSERT( _format != 0 );

    _generate();
    _resize( pvp.w, pvp.h );  
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _id );

    if( _defined )
    {
        EQ_GL_CALL( glCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0,
                                         pvp.x, pvp.y, pvp.w, pvp.h ));
    }
    else
    {
        EQ_GL_CALL( glCopyTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, _format, 
                                      pvp.x, pvp.y, pvp.w, pvp.h, 0 ));
        _defined = true;
    }
}

void Texture::upload( const Image* image, const Frame::Buffer which )
{
    EQASSERT( _format != 0 );

    const eq::PixelViewport& pvp = image->getPixelViewport();

    _generate();
    _resize( pvp.w, pvp.h );  
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _id );

    if( _defined )
    {
        EQ_GL_CALL( glTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0,
                                     pvp.w, pvp.h,
                                     image->getFormat( which ), 
                                     image->getType( which ),
                                     image->getPixelPointer( which )));
    }
    else
    {
        EQ_GL_CALL( glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 
                                  _format, pvp.w, pvp.h, 0,
                                  image->getFormat( which ), 
                                  image->getType( which ),
                                  image->getPixelPointer( which )));
        _defined = true;
    }
}

}
