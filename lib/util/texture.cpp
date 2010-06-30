
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

#include <eq/client/image.h>

namespace eq
{
namespace util
{
Texture::Texture( GLEWContext* const glewContext )
        : _id( 0 )
        , _target( GL_TEXTURE_RECTANGLE_ARB )
        , _internalFormat( 0 )
        , _format( 0 )
        , _type( 0 )
        , _width( 0 )
        , _height( 0 )
        , _defined( false ) 
        , _glewContext( glewContext ){}

Texture::~Texture()
{
    if( _id != 0 )
        EQWARN << "OpenGL texture " << _id << " was not freed" << std::endl;

    _id      = 0;
    _defined = false;
}

bool Texture::isValid() const
{ 
    return ( _id != 0 && _defined );
}

void Texture::flush()
{
    if( _id == 0 )
        return;

    CHECK_THREAD( _thread );
    glDeleteTextures( 1, &_id );
    _id = 0;
    _downloaderName = 0;
    _defined = false;
}

void Texture::setTarget( const GLenum target )
{               
    _target = target;
}

void Texture::setInternalFormat( const GLuint internalFormat )
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
            setDownloader( EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT );
            break;
        case GL_RGB10_A2:
            setExternalFormat( GL_RGBA, GL_UNSIGNED_INT_10_10_10_2 );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_BGR10A2 );
            break;
        case GL_RGBA:
        case GL_RGBA8:
            setExternalFormat( GL_RGBA, GL_UNSIGNED_BYTE );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA );
            break;
        case GL_RGBA16F:
            setExternalFormat( GL_RGBA, GL_HALF_FLOAT );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F );
            break;
        case GL_RGBA32F:
            setExternalFormat( GL_RGBA, GL_FLOAT );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F );
            break;
        case GL_RGB:
        case GL_RGB8:
            setExternalFormat( GL_RGB, GL_UNSIGNED_BYTE );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGB_TO_BGR );
            break;
        case GL_RGB16F:
            setExternalFormat( GL_RGB, GL_HALF_FLOAT );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR16F );
            break;
        case GL_RGB32F:
            setExternalFormat( GL_RGB, GL_FLOAT );
            setDownloader( EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR32F );
            break;
        case GL_ALPHA32F_ARB:
            setExternalFormat( GL_ALPHA, GL_FLOAT );
            //setDownloader( !!! );
            break;
        case GL_DEPTH24_STENCIL8:
            setExternalFormat( GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_EXT );
            setDownloader( EQ_COMPRESSOR_TRANSFER_DEPTH_STENCIL_TO_UNSIGNED_INT_24_8 );
            break;
        case GL_RGBA32UI:
            EQASSERT( _glewContext );
            if( GLEW_EXT_texture_integer )
            {
                setExternalFormat( GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT );
            }
            else
                EQUNIMPLEMENTED;
            break;

        default:
            EQUNIMPLEMENTED;
            setExternalFormat( _internalFormat, GL_UNSIGNED_BYTE );
    }
}

void Texture::setExternalFormat( const uint32_t format,
                                 const uint32_t type )
{
     _format = format;
     _type   = type;    
}

void Texture::_generate()
{
    CHECK_THREAD( _thread );
    if( _id != 0 )
        return;

    _defined = false;
    glGenTextures( 1, &_id );
}

void Texture::flushNoDelete()
{
    if( _id == 0 )
        return;

    CHECK_THREAD( _thread );
    _id = 0;
    _defined = false;
}

void Texture::init( const GLuint format, const int width, const int height )
{
    setInternalFormat( format );
    _generate();
    resize( width, height );
}

void Texture::setGLData( const GLuint id, const int width, const int height )
{
    _id = id;
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

void Texture::copyFromFrameBuffer( const uint64_t  inDims[4] )
{
    _copyFromFrameBuffer( inDims[0], inDims[1], inDims[2], inDims[3] );
}

void Texture::copyFromFrameBuffer( const PixelViewport& pvp )
{
    _copyFromFrameBuffer( pvp.x, pvp.w, pvp.y, pvp.h );
}

void Texture::_copyFromFrameBuffer( uint32_t x, uint32_t w, 
                                    uint32_t y, uint32_t h )
{
    EQ_GL_ERROR( "before Texture::copyFromFrameBuffer" );
    CHECK_THREAD( _thread );
    EQASSERT( _internalFormat != 0 );

    _generate();
    _grow( w, h );

    if( _defined )
        glBindTexture( _target, _id );
    else
        resize( _width, _height );

    EQ_GL_CALL( glCopyTexSubImage2D( _target, 0, 0, 0, x, y, w, h ));
    EQ_GL_ERROR( "after Texture::copyFromFrameBuffer" );
}

void Texture::upload( const Image* image, const Frame::Buffer which )
{
    CHECK_THREAD( _thread );

    setInternalFormat( image->getInternalFormat( which ));
    EQASSERT( _internalFormat != 0 );

    image->uploadToTexture( which, _id, _glewContext );
}

void Texture::upload( const int width, const int height, const void* ptr )
{
    _generate();
    _grow( width, height );

    if( _defined )
        glBindTexture( _target, _id );
    else
        resize( _width, _height );

    glTexSubImage2D( _target, 0, 0, 0, width, height, _format, _type, ptr );
}

void Texture::download( void* buffer, const uint32_t format,
                        const uint32_t type ) const
{
    CHECK_THREAD( _thread );
    EQASSERT( _defined );
    EQ_GL_CALL( glBindTexture( _target, _id ));
    if ( format == 0 || type == 0 )
    {
        EQ_GL_CALL( glGetTexImage( _target, 0, _format, _type, buffer ));
    }
    else
    {
        EQ_GL_CALL( glGetTexImage( _target, 0, format, type, buffer ));
    }
}

void Texture::download( void* buffer ) const
{
    download( buffer, _format, _type );
}

void Texture::bind() const
{
    EQASSERT( _id );
    glBindTexture( _target, _id );
}

void Texture::bindToFBO( const GLenum target, const int width, 
                         const int height )
{
    CHECK_THREAD( _thread );
    EQASSERT( _internalFormat );
    EQASSERT( _glewContext );

    _generate();

    glBindTexture( _target, _id );
    glTexImage2D( _target, 0, _internalFormat, width, height, 0,
                  _format, _type, 0 );
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER, target, _target, _id, 0 );

    _width = width;
    _height = height;
    _defined = true;
}

void Texture::resize( const int width, const int height )
{
    CHECK_THREAD( _thread );
    EQASSERT( _id );
    EQASSERT( _internalFormat );
    EQASSERT( width > 0 && height > 0 );

    if( _width == width && _height == height && _defined )
        return;

    if( _target == GL_TEXTURE_2D && !_isPOT( width, height ))
    {
        EQASSERT( _glewContext );
        EQASSERT( GLEW_ARB_texture_non_power_of_two );
    }

    glBindTexture( _target, _id );
    EQ_GL_CALL( glTexImage2D( _target, 0, _internalFormat, width, height, 0,
                  _format, _type, 0 ));

    _width  = width;
    _height = height;
    _defined = true;
}

void Texture::writeRGB( const std::string& filename, 
                        const Frame::Buffer buffer,
                        const PixelViewport& pvp ) const
{
    eq::Image image;

    image.allocDownloader( buffer, _downloaderName, _glewContext );
    image.readbackFromTexture( buffer, pvp, _id, _glewContext );
    image.writeImage( filename + ".rgb", buffer );
}

}
}
