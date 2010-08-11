
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@equalizergraphics.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "compressorReadDrawPixels.h"

#include <eq/util/texture.h>
#include <eq/base/buffer.h>

#define glewGetContext() glewContext
    
namespace eq
{
namespace plugin
{

namespace
{
#define REGISTER_TRANSFER( in, out, size, quality_, ratio_, speed_, alpha ) \
    static void _getInfo ## in ## out( EqCompressorInfo* const info )   \
    {                                                                   \
        info->version = EQ_COMPRESSOR_VERSION;                          \
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D | \
            EQ_COMPRESSOR_USE_TEXTURE | EQ_COMPRESSOR_USE_FRAMEBUFFER;  \
        if( alpha )                                                     \
            info->capabilities |= EQ_COMPRESSOR_IGNORE_ALPHA;           \
        info->quality = quality_ ## f;                                  \
        info->ratio   = ratio_ ## f;                                    \
        info->speed   = speed_ ## f;                                    \
        info->name = EQ_COMPRESSOR_TRANSFER_ ## in ## _TO_ ## out;      \
        info->tokenType = EQ_COMPRESSOR_DATATYPE_ ## in;                \
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_ ## out;         \
        info->outputTokenSize = size;                                   \
    }                                                                   \
                                                                        \
    static bool _register ## in ## out()                                \
    {                                                                   \
        Compressor::registerEngine(                                     \
            Compressor::Functions(                                      \
                _getInfo ## in ## out,                                  \
                CompressorReadDrawPixels::getNewCompressor,             \
                CompressorReadDrawPixels::getNewDecompressor,           \
                0,                                                      \
                CompressorReadDrawPixels::isCompatible ));              \
        return true;                                                    \
    }                                                                   \
                                                                        \
    static bool _initialized ## in ## out = _register ## in ## out();

REGISTER_TRANSFER( RGBA, RGBA, 4, 1., 1., 1., false );
REGISTER_TRANSFER( RGBA, BGRA, 4, 1., 1., 2., false );
REGISTER_TRANSFER( RGBA, RGBA_UINT_8_8_8_8_REV, 4, 1., 1., 1., false );
REGISTER_TRANSFER( RGBA, BGRA_UINT_8_8_8_8_REV, 4, 1., 1., 2., false );
REGISTER_TRANSFER( RGBA, RGB, 3, 1., 1., .7, true );
REGISTER_TRANSFER( RGBA, BGR, 3, 1., 1., .7, true );

REGISTER_TRANSFER( RGB10_A2, BGR10_A2, 4, 1., 1., 1., false );

REGISTER_TRANSFER( RGBA16F, RGBA16F, 8, 1., 1., 1., false );
REGISTER_TRANSFER( RGBA16F, BGRA16F, 8, 1., 1., 1.1, false );
REGISTER_TRANSFER( RGBA16F, RGB16F, 6, 1., .75, 1., true );
REGISTER_TRANSFER( RGBA16F, BGR16F, 6, 1., .75, 1.1, true );
REGISTER_TRANSFER( RGBA16F, RGBA, 4, 1., .5, 1., false );
REGISTER_TRANSFER( RGBA16F, BGRA, 4, 1., .5, 1.1, false );
REGISTER_TRANSFER( RGBA16F, RGB, 3, 1., .375, 1., true );
REGISTER_TRANSFER( RGBA16F, BGR, 3, 1., .375, 1.1, true );

REGISTER_TRANSFER( RGBA32F, RGBA32F, 16, 1., 1., 1., false );
REGISTER_TRANSFER( RGBA32F, BGRA32F, 16, 1., 1., 1.1, false );
REGISTER_TRANSFER( RGBA32F, RGB32F, 12, 1., .75, 1., true );
REGISTER_TRANSFER( RGBA32F, BGR32F, 12, 1., .75, 1.1, true );
REGISTER_TRANSFER( RGBA32F, RGBA16F, 8, 1., .5, 1., false );
REGISTER_TRANSFER( RGBA32F, BGRA16F, 8, 1., .5, 1.1, false );
REGISTER_TRANSFER( RGBA32F, RGB16F, 6, 1., .375, 1., true );
REGISTER_TRANSFER( RGBA32F, BGR16F, 6, 1., .375, 1.1, true );
REGISTER_TRANSFER( RGBA32F, RGBA, 4, 1., .25, 1., false );
REGISTER_TRANSFER( RGBA32F, BGRA, 4, 1., .25, 1.1, false );
REGISTER_TRANSFER( RGBA32F, RGB, 3, 1., .1875, 1., true );
REGISTER_TRANSFER( RGBA32F, BGR, 3, 1., .1875, 1.1, true );

REGISTER_TRANSFER( DEPTH, DEPTH_UNSIGNED_INT, 4, 1., 1., 1., false );
}

CompressorReadDrawPixels::CompressorReadDrawPixels(const EqCompressorInfo* info)
        : Compressor( info )
        , _texture( 0 )
        , _internalFormat( 0 )
        , _format( 0 )
        , _type( 0 )
        , _depth( info->outputTokenSize )
{ 
    switch( info->outputTokenType )
    {
        case EQ_COMPRESSOR_DATATYPE_RGBA:
        case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
        case EQ_COMPRESSOR_DATATYPE_RGBA32F:
        case EQ_COMPRESSOR_DATATYPE_RGBA16F:
        case EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV:
            _format = GL_RGBA;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGRA:
        case EQ_COMPRESSOR_DATATYPE_BGR10_A2:
        case EQ_COMPRESSOR_DATATYPE_BGRA32F:
        case EQ_COMPRESSOR_DATATYPE_BGRA16F:
        case EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV:
            _format = GL_BGRA;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGB:
        case EQ_COMPRESSOR_DATATYPE_RGB32F:
        case EQ_COMPRESSOR_DATATYPE_RGB16F:
            _format = GL_RGB;
            break;
        case EQ_COMPRESSOR_DATATYPE_BGR:
        case EQ_COMPRESSOR_DATATYPE_BGR32F:
        case EQ_COMPRESSOR_DATATYPE_BGR16F:
            _format = GL_BGR;
            break;
        case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT:
            _format = GL_DEPTH_COMPONENT;
            break;
        default: EQASSERT( false );
    }

    switch( info->outputTokenType )
    {
        case EQ_COMPRESSOR_DATATYPE_RGBA:        
        case EQ_COMPRESSOR_DATATYPE_BGRA:
        case EQ_COMPRESSOR_DATATYPE_RGB:
        case EQ_COMPRESSOR_DATATYPE_BGR:
        case EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV:
        case EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV:
            _type = GL_UNSIGNED_BYTE;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
        case EQ_COMPRESSOR_DATATYPE_BGR10_A2:
            _type = GL_UNSIGNED_INT_10_10_10_2;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGBA32F:
        case EQ_COMPRESSOR_DATATYPE_BGRA32F:
        case EQ_COMPRESSOR_DATATYPE_RGB32F:
        case EQ_COMPRESSOR_DATATYPE_BGR32F:
            _type = GL_FLOAT;
            break;        
        case EQ_COMPRESSOR_DATATYPE_RGBA16F:
        case EQ_COMPRESSOR_DATATYPE_BGRA16F:
        case EQ_COMPRESSOR_DATATYPE_RGB16F:
        case EQ_COMPRESSOR_DATATYPE_BGR16F:
            _type = GL_HALF_FLOAT;
            break;
        case EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT:
            _type = GL_UNSIGNED_INT;
            break;
        default: EQASSERT( false );
    }

    switch( info->tokenType )
    {
        case EQ_COMPRESSOR_DATATYPE_RGBA:              
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
            _internalFormat = GL_RGB10_A2;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGBA32F:
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_DATATYPE_RGBA16F:
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_DATATYPE_DEPTH:
            _internalFormat = GL_DEPTH_COMPONENT;
            break;
        default: EQASSERT( false );
    }
}

CompressorReadDrawPixels::~CompressorReadDrawPixels( )
{
    delete _texture;
    _texture = 0;
}

bool CompressorReadDrawPixels::isCompatible( const GLEWContext* glewContext )
{
    return true;
}

void CompressorReadDrawPixels::_init( const uint64_t inDims[4],
                                      uint64_t outDims[4] )
{
    outDims[0] = inDims[0];
    outDims[1] = inDims[1];
    outDims[2] = inDims[2];
    outDims[3] = inDims[3];

    const size_t size = inDims[1] * inDims[3] * _depth;

    // eile: The code path using realloc creates visual artefacts on my MacBook
    //       The other crashes occasionally with KERN_BAD_ADDRESS
    //       Seems to be only with GL_RGB. Radar #8261726.
#if 1
    _buffer.reserve( size );
    _buffer.setSize( size );
#else
    // eile: This code path using realloc creates visual artefacts on my MacBook
    _buffer.resize( size );
#endif
}

void CompressorReadDrawPixels::download( const GLEWContext* glewContext,
                                         const uint64_t  inDims[4],
                                         const unsigned  source,
                                         const uint64_t  flags,
                                         uint64_t        outDims[4],
                                         void**          out )
{
    _init( inDims, outDims );

    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glReadPixels( inDims[0], inDims[2], inDims[1], inDims[3], _format,
                      _type, _buffer.getData() );
    }
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE )
    {
        if ( !_texture )
        {
            _texture = new util::Texture(GL_TEXTURE_RECTANGLE_ARB, glewContext);
            _texture->setInternalFormat( _internalFormat );
        }
        
        _texture->setGLData( source, inDims[1], inDims[3] );
        _texture->download( _buffer.getData(), _format, _type );
        _texture->flushNoDelete();
    }
    else
    {
        EQUNREACHABLE;
    }
    *out = _buffer.getData();
}

void CompressorReadDrawPixels::upload( const GLEWContext* glewContext, 
                                       const void*        buffer,
                                       const uint64_t     inDims[4],
                                       const uint64_t     flags,
                                       const uint64_t     outDims[4],  
                                       const unsigned     destination )
{
    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glRasterPos2i( outDims[0], outDims[2] );
        glDrawPixels( outDims[1], outDims[3], _format, _type, buffer );
    }
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE )
    {
        if( !_texture )
        {
            _texture = new util::Texture(GL_TEXTURE_RECTANGLE_ARB, glewContext);
            _texture->setInternalFormat( _internalFormat );
        }
        _texture->setExternalFormat( _format, _type );
        _texture->setGLData( destination, outDims[1], outDims[3] );
        _texture->upload( outDims[1] , outDims[3], buffer );
        _texture->flushNoDelete();
    }
}
}

}
