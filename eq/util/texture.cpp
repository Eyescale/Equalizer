
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/fabric/pixelViewport.h>

#include <eq/image.h>
#include <eq/gl.h>
#include <pression/plugins/compressor.h>

namespace eq
{
namespace util
{
namespace detail
{
class Texture
{
public:
    Texture( const GLenum tgt, const GLEWContext* const gl )
        : name( 0 )
        , target( tgt )
        , internalFormat( 0 )
        , format( 0 )
        , type( 0 )
        , width( 0 )
        , height( 0 )
        , defined( false )
        , glewContext( gl )
        {}

    ~Texture()
    {
        if( name != 0 )
            LBWARN << "OpenGL texture " << name << " not freed" << std::endl;

        name = 0;
        defined = false;
    }

    GLuint name;
    const GLenum target;
    GLuint internalFormat;
    GLuint format;
    GLuint type;
    int32_t width;
    int32_t height;
    bool defined;
    const GLEWContext* glewContext;
};
}

Texture::Texture( const unsigned target, const GLEWContext* const glewContext )
    : _impl( new detail::Texture( target, glewContext ))
{}

Texture::~Texture()
{
    delete _impl;
}
bool Texture::isValid() const
{
    return ( _impl->name != 0 && _impl->defined );
}

void Texture::flush()
{
    if( _impl->name == 0 )
        return;

    LB_TS_THREAD( _thread );
    EQ_GL_CALL( glDeleteTextures( 1, &_impl->name ));
    _impl->name = 0;
    _impl->defined = false;
}

void Texture::flushNoDelete()
{
    LB_TS_THREAD( _thread );
    _impl->name = 0;
    _impl->defined = false;
}

uint32_t Texture::getCompressorTarget() const
{
    switch( _impl->target )
    {
        case GL_TEXTURE_RECTANGLE_ARB:
            return EQ_COMPRESSOR_USE_TEXTURE_RECT;

        default:
            LBUNIMPLEMENTED;
        case GL_TEXTURE_2D:
            return EQ_COMPRESSOR_USE_TEXTURE_2D;
    }
}

void Texture::_setInternalFormat( const GLuint internalFormat )
{
    if( _impl->internalFormat == internalFormat )
        return;

    _impl->defined = false;
    _impl->internalFormat = internalFormat;

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
            LBASSERT( _impl->glewContext );
            if( GLEW_EXT_texture_integer )
                setExternalFormat( GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT );
            else
                LBUNIMPLEMENTED;
            break;

        default:
            LBUNIMPLEMENTED;
            setExternalFormat( internalFormat, GL_UNSIGNED_BYTE );
    }
}

void Texture::setExternalFormat( const uint32_t format, const uint32_t type )
{
     _impl->format = format;
     _impl->type   = type;
}

void Texture::_generate()
{
    LB_TS_THREAD( _thread );
    if( _impl->name != 0 )
        return;

    _impl->defined = false;
    EQ_GL_CALL( glGenTextures( 1, &_impl->name ));
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
    _impl->name = id;
    _impl->width = width;
    _impl->height = height;
    _impl->defined = true;
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
    if( _impl->width < width )
    {
        _impl->width   = width;
        _impl->defined = false;
    }

    if( _impl->height < height )
    {
        _impl->height  = height;
        _impl->defined = false;
    }
}

void Texture::applyZoomFilter( const ZoomFilter zoomFilter ) const
{
    EQ_GL_CALL( glTexParameteri( _impl->target, GL_TEXTURE_MAG_FILTER,
                                 zoomFilter ));
    EQ_GL_CALL( glTexParameteri( _impl->target, GL_TEXTURE_MIN_FILTER,
                                 zoomFilter ));
}

void Texture::applyWrap() const
{
    EQ_GL_CALL( glTexParameteri( _impl->target, GL_TEXTURE_WRAP_S,
                                 GL_CLAMP_TO_EDGE ));
    EQ_GL_CALL( glTexParameteri( _impl->target, GL_TEXTURE_WRAP_T,
                                 GL_CLAMP_TO_EDGE ));
}

void Texture::copyFromFrameBuffer( const GLuint internalFormat,
                                   const fabric::PixelViewport& pvp )
{
    EQ_GL_ERROR( "before Texture::copyFromFrameBuffer" );
    LB_TS_THREAD( _thread );

    _generate();
    _setInternalFormat( internalFormat );
    _grow( pvp.w, pvp.h );

    if( _impl->defined )
        glBindTexture( _impl->target, _impl->name );
    else
        resize( _impl->width, _impl->height );

    EQ_GL_CALL(  glCopyTexSubImage2D( _impl->target, 0, 0, 0, pvp.x, pvp.y,
                                      pvp.w, pvp.h ));
    EQ_GL_ERROR( "after Texture::copyFromFrameBuffer" );
}

void Texture::upload( const int32_t width, const int32_t height,
                      const void* ptr )
{
    _generate();
    _grow( width, height );

    if( _impl->defined )
        glBindTexture( _impl->target, _impl->name );
    else
        resize( _impl->width, _impl->height );

    EQ_GL_CALL( glTexSubImage2D( _impl->target, 0, 0, 0, width, height,
                                 _impl->format, _impl->type, ptr ));
}

void Texture::download( void* buffer ) const
{
    LBASSERT( isValid( ));
    EQ_GL_CALL( glBindTexture( _impl->target, _impl->name ));
    EQ_GL_CALL( glGetTexImage( _impl->target, 0, _impl->format, _impl->type,
                               buffer ));
}

void Texture::bind() const
{
    LBASSERT( _impl->name );
    EQ_GL_CALL( glBindTexture( _impl->target, _impl->name ));
}

void Texture::bindToFBO( const GLenum target, const int32_t width,
                         const int32_t height, const int32_t samples )
{
    LB_TS_THREAD( _thread );
    LBASSERT( _impl->internalFormat );
    LBASSERT( _impl->glewContext );

    _generate();

    EQ_GL_CALL( glBindTexture( _impl->target, _impl->name ));
    EQ_GL_CALL( glTexImage2D( _impl->target, 0, _impl->internalFormat, width,
                              height, 0, _impl->format, _impl->type, 0 ));
    EQ_GL_CALL( glFramebufferTexture2DEXT( GL_FRAMEBUFFER, target,
                                           _impl->target, _impl->name, 0 ));

    if( samples > 1 )
    {
        EQ_GL_CALL( glTexImage2DMultisample( _impl->target, samples,
                                             _impl->internalFormat, width,
                                             height, false ));
    }

    _impl->width = width;
    _impl->height = height;
    _impl->defined = true;
}

void Texture::resize( const int32_t width, const int32_t height )
{
    LB_TS_THREAD( _thread );
    LBASSERT( _impl->name );
    LBASSERT( _impl->internalFormat );
    LBASSERT( width > 0 && height > 0 );

    if( _impl->width == width && _impl->height == height && _impl->defined )
        return;

    if( _impl->target == GL_TEXTURE_2D && !_isPOT( width, height ))
    {
        LBASSERT( _impl->glewContext );
        LBASSERT( GLEW_ARB_texture_non_power_of_two );
    }

    EQ_GL_CALL( glBindTexture( _impl->target, _impl->name ));
    EQ_GL_CALL( glTexImage2D( _impl->target, 0, _impl->internalFormat, width,
                              height, 0, _impl->format, _impl->type, 0 ));
    _impl->width  = width;
    _impl->height = height;
    _impl->defined = true;
}

void Texture::writeRGB( const std::string& filename ) const
{
    LBASSERT( _impl->defined );
    if( !_impl->defined )
        return;

    eq::Image image;

    switch( _impl->internalFormat )
    {
        case GL_DEPTH_COMPONENT:
            image.allocDownloader( Frame::BUFFER_COLOR,
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                   _impl->glewContext );
            break;
        case GL_RGB10_A2:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGB10_A2_TO_BGR10_A2,
                                   _impl->glewContext );
            break;
        case GL_RGBA:
        case GL_RGBA8:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA,
                                   _impl->glewContext );
            break;
        case GL_RGBA16F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F,
                                   _impl->glewContext );
            break;
        case GL_RGBA32F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F,
                                   _impl->glewContext );
            break;
        case GL_RGB:
        case GL_RGB8:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGR,
                                   _impl->glewContext );
            break;
        case GL_RGB16F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGR16F,
                                   _impl->glewContext );
            break;
        case GL_RGB32F:
            image.allocDownloader( Frame::BUFFER_COLOR,
                                   EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR32F,
                                   _impl->glewContext );
            break;
        case GL_DEPTH24_STENCIL8:
            image.allocDownloader( Frame::BUFFER_COLOR,
                             EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT,
                                   _impl->glewContext );
            break;

        default:
            LBUNIMPLEMENTED;
            return;
    }

    image.setPixelViewport( eq::PixelViewport( 0, 0,
                                               _impl->width, _impl->height ));
    if( image.startReadback( Frame::BUFFER_COLOR, this, _impl->glewContext ))
        image.finishReadback( _impl->glewContext );
    image.writeImage( filename + ".rgb", Frame::BUFFER_COLOR );
    image.resetPlugins();
}

GLenum Texture::getTarget() const { return _impl->target; }
GLuint Texture::getInternalFormat() const { return _impl->internalFormat; }
GLuint Texture::getFormat() const { return _impl->format; }
GLuint Texture::getType() const { return _impl->type; }
GLuint Texture::getName() const { return _impl->name; }
int32_t Texture::getWidth() const { return _impl->width; }
int32_t Texture::getHeight() const { return _impl->height; }
const GLEWContext* Texture::glewGetContext() const{ return _impl->glewContext; }

void Texture::setGLEWContext( const GLEWContext* context )
{
    _impl->glewContext = context;
}

std::ostream& operator << ( std::ostream& os, const Texture& texture )
{
    return os
        << "Texture " << texture.getWidth() << "x" << texture.getHeight()
        << " id " << texture.getName() << std::hex << " format "
        << texture.getFormat() << " type " << texture.getType() << std::dec
        << (texture.isValid() ? "" : " invalid");
}

}
}
