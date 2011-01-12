
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#include "texture.h"

#include <eq/image.h>
#include <co/plugins/compressor.h>

namespace eq
{
namespace util
{
Texture::Texture( const GLenum target, const GLEWContext* const glewContext )
        : _name( 0 )
        , _target( target )
        , _internalFormat( 0 )
        , _format( 0 )
        , _type( 0 )
        , _width( 0 )
        , _height( 0 )
        , _defined( false ) 
        , _glewContext( glewContext )
{}

Texture::~Texture()
{
    if( _name != 0 )
        EQWARN << "OpenGL texture " << _name << " was not freed" << std::endl;

    _name      = 0;
    _defined = false;
}

bool Texture::isValid() const
{ 
    return ( _name != 0 && _defined );
}

void Texture::flush()
{
    if( _name == 0 )
        return;

    EQ_TS_THREAD( _thread );
    glDeleteTextures( 1, &_name );
    _name = 0;
    _defined = false;
}

void Texture::flushNoDelete()
{
    EQ_TS_THREAD( _thread );
    _name = 0;
    _defined = false;
}

uint32_t Texture::getCompressorTarget() const
{
    switch( _target )
    {
        case GL_TEXTURE_RECTANGLE_ARB:
            return EQ_COMPRESSOR_USE_TEXTURE_RECT;

        default:
            EQUNIMPLEMENTED;
        case GL_TEXTURE_2D:
            return EQ_COMPRESSOR_USE_TEXTURE_2D;
    }
}

void Texture::_setInternalFormat( const GLuint internalFormat )
{
    if( _internalFormat == internalFormat )
        return;

    _defined = false;
    _internalFormat = internalFormat;

    switch( internalFormat )
    {
        // depth format
        case GL_DEPTH_COMPONENT:
            setExternalFormat( GL_DEPTH_COMPONENT, GL_UNSIGNED_INT );
            break;
        case GL_RGB10_A2:
            setExternalFormat( GL_RGBA, GL_UNSIGNED_INT_10_10_10_2 );
            break;
        case GL_RGBA:
        case GL_RGBA8:
            setExternalFormat( GL_RGBA, GL_UNSIGNED_BYTE );
            break;
        case GL_RGBA16F:
            setExternalFormat( GL_RGBA, GL_HALF_FLOAT );
            break;
        case GL_RGBA32F:
            setExternalFormat( GL_RGBA, GL_FLOAT );
            break;
        case GL_RGB:
        case GL_RGB8:
            setExternalFormat( GL_RGB, GL_UNSIGNED_BYTE );
            break;
        case GL_RGB16F:
            setExternalFormat( GL_RGB, GL_HALF_FLOAT );
            break;
        case GL_RGB32F:
            setExternalFormat( GL_RGB, GL_FLOAT );
            break;
        case GL_ALPHA32F_ARB:
            setExternalFormat( GL_ALPHA, GL_FLOAT );
            break;
        case GL_DEPTH24_STENCIL8:
            setExternalFormat( GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 );
            break;
        case GL_RGBA32UI:
            EQASSERT( _glewContext );
            if( GLEW_EXT_texture_integer )
                setExternalFormat( GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT );
            else
                EQUNIMPLEMENTED;
            break;

        default:
            EQUNIMPLEMENTED;
            setExternalFormat( internalFormat, GL_UNSIGNED_BYTE );
    }
}

void Texture::setExternalFormat( const uint32_t format, const uint32_t type )
{
     _format = format;
     _type   = type;    
}

void Texture::_generate()
{
    EQ_TS_THREAD( _thread );
    if( _name != 0 )
        return;

    _defined = false;
    EQ_GL_CALL( glGenTextures( 1, &_name ));
}

void Texture::init( const GLuint format, const int32_t width,
                    const int32_t height )
{
    _generate();
    _setInternalFormat( format );
    resize( width, height );
}

void Texture::setGLData( const GLuint id, const GLuint internalFormat,
                         const int32_t width, const int32_t height )
{
    flush();
    _setInternalFormat( internalFormat );
    _name = id;
    _width = width;
    _height = height;
    _defined = true;
}

namespace
{
/* Check if the texture dimensions are power of two. */
static bool _isPOT( const uint32_t width, const uint32_t height )
{
    return ( width > 0 && height > 0 &&
           ( width & ( width - 1 )) == 0 &&
           ( height & ( height - 1 )) == 0 );
}
}

void Texture::_grow( const int32_t width, const int32_t height )
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

void Texture::applyZoomFilter( const ZoomFilter zoomFilter ) const
{
    glTexParameteri( _target, GL_TEXTURE_MAG_FILTER, zoomFilter );
    glTexParameteri( _target, GL_TEXTURE_MIN_FILTER, zoomFilter );
}

void Texture::applyWrap() const
{
    glTexParameteri( _target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( _target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void Texture::copyFromFrameBuffer( const GLuint internalFormat,
                                   const fabric::PixelViewport& pvp )
{
    EQ_GL_ERROR( "before Texture::copyFromFrameBuffer" );
    EQ_TS_THREAD( _thread );

    _generate();
    _setInternalFormat( internalFormat );
    _grow( pvp.w, pvp.h );

    if( _defined )
        glBindTexture( _target, _name );
    else
        resize( _width, _height );

    glCopyTexSubImage2D( _target, 0, 0, 0, pvp.x, pvp.y, pvp.w, pvp.h );
    EQ_GL_ERROR( "after Texture::copyFromFrameBuffer" );
}

void Texture::upload( const int32_t width, const int32_t height,
                      const void* ptr )
{
    _generate();
    _grow( width, height );

    if( _defined )
        glBindTexture( _target, _name );
    else
        resize( _width, _height );

    glTexSubImage2D( _target, 0, 0, 0, width, height, _format, _type, ptr );
}

void Texture::download( void* buffer ) const
{
    EQ_TS_THREAD( _thread );
    EQASSERT( _defined );
    EQ_GL_CALL( glBindTexture( _target, _name ));
    EQ_GL_CALL( glGetTexImage( _target, 0, _format, _type, buffer ));
}

void Texture::bind() const
{
    EQASSERT( _name );
    glBindTexture( _target, _name );
}

void Texture::bindToFBO( const GLenum target, const int32_t width, 
                         const int32_t height )
{
    EQ_TS_THREAD( _thread );
    EQASSERT( _internalFormat );
    EQASSERT( _glewContext );

    _generate();

    glBindTexture( _target, _name );
    glTexImage2D( _target, 0, _internalFormat, width, height, 0,
                  _format, _type, 0 );
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER, target, _target, _name, 0 );

    _width = width;
    _height = height;
    _defined = true;
}

void Texture::resize( const int32_t width, const int32_t height )
{
    EQ_TS_THREAD( _thread );
    EQASSERT( _name );
    EQASSERT( _internalFormat );
    EQASSERT( width > 0 && height > 0 );

    if( _width == width && _height == height && _defined )
        return;

    if( _target == GL_TEXTURE_2D && !_isPOT( width, height ))
    {
        EQASSERT( _glewContext );
        EQASSERT( GLEW_ARB_texture_non_power_of_two );
    }

    EQ_GL_CALL( glBindTexture( _target, _name ));
    EQ_GL_CALL( glTexImage2D( _target, 0, _internalFormat, width, height, 0,
                              _format, _type, 0 ));
    _width  = width;
    _height = height;
    _defined = true;
}

void Texture::writeRGB( const std::string& filename ) const
{
    EQASSERT( _defined );
    if( !_defined )
        return;

    eq::Image image;

    switch( _internalFormat )
    {
        case GL_DEPTH_COMPONENT:
            image.allocDownloader( Frame::BUFFER_COLOR,
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                   _glewContext );
            break;
        case GL_RGB10_A2:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGB10_A2_TO_BGR10_A2,
                                   _glewContext );
            break;
        case GL_RGBA:
        case GL_RGBA8:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA,
                                   _glewContext );
            break;
        case GL_RGBA16F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F,
                                   _glewContext );
            break;
        case GL_RGBA32F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F,
                                   _glewContext );
            break;
        case GL_RGB:
        case GL_RGB8:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGR,
                                   _glewContext );
            break;
        case GL_RGB16F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGR16F,
                                   _glewContext );
            break;
        case GL_RGB32F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR32F,
                                   _glewContext );
            break;
        case GL_DEPTH24_STENCIL8:
            image.allocDownloader( Frame::BUFFER_COLOR, 
                      EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                   _glewContext );
            break;

        default:
            EQUNIMPLEMENTED;
            return;
    }

    image.setPixelViewport( eq::PixelViewport( 0, 0, _width, _height ));
    image.readback( Frame::BUFFER_COLOR, this, _glewContext );
    image.writeImage( filename + ".rgb", Frame::BUFFER_COLOR );
}

}
}
