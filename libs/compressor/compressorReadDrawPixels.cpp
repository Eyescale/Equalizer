
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
#include <co/base/buffer.h>

#define glewGetContext() glewContext
    
namespace eq
{
namespace plugin
{

namespace
{
static stde::hash_map< unsigned, unsigned > _depths;

#define REGISTER_TRANSFER( in, out, size, quality_, ratio_, speed_, alpha ) \
    static void _getInfo ## in ## out( EqCompressorInfo* const info )   \
    {                                                                   \
        info->version = EQ_COMPRESSOR_VERSION;                          \
        info->capabilities = EQ_COMPRESSOR_TRANSFER |                   \
                             EQ_COMPRESSOR_DATA_2D |                    \
                             EQ_COMPRESSOR_USE_TEXTURE_RECT |           \
                             EQ_COMPRESSOR_USE_TEXTURE_2D |             \
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;             \
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
        const unsigned name = EQ_COMPRESSOR_TRANSFER_ ## in ## _TO_ ## out; \
        Compressor::registerEngine(                                     \
            Compressor::Functions(                                      \
                name,                                                   \
                _getInfo ## in ## out,                                  \
                CompressorReadDrawPixels::getNewCompressor,             \
                CompressorReadDrawPixels::getNewDecompressor,           \
                0,                                                      \
                CompressorReadDrawPixels::isCompatible ));              \
        _depths[ name ] = size;                                         \
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

REGISTER_TRANSFER( RGB10_A2, BGR10_A2, 4, 1., 1., 1.1, false );
REGISTER_TRANSFER( RGB10_A2, RGB10_A2, 4, 1., 1., 1., false );

REGISTER_TRANSFER( RGBA16F, RGBA16F, 8, 1., 1., 1., false );
REGISTER_TRANSFER( RGBA16F, BGRA16F, 8, 1., 1., 1.1, false );
REGISTER_TRANSFER( RGBA16F, RGB16F, 6, 1., 1., 1., true );
REGISTER_TRANSFER( RGBA16F, BGR16F, 6, 1., 1., 1.1, true );
REGISTER_TRANSFER( RGBA16F, RGBA, 4, .5, .5, 1., false );
REGISTER_TRANSFER( RGBA16F, BGRA, 4, .5, .5, 1.1, false );
REGISTER_TRANSFER( RGBA16F, RGB, 3, .5, .5, 1., true );
REGISTER_TRANSFER( RGBA16F, BGR, 3, .5, .5, 1.1, true );

REGISTER_TRANSFER( RGBA32F, RGBA32F, 16, 1., 1., 1.1, false );
REGISTER_TRANSFER( RGBA32F, BGRA32F, 16, 1., 1., 1., false );
REGISTER_TRANSFER( RGBA32F, RGB32F, 12, 1., 1., 1., true );
REGISTER_TRANSFER( RGBA32F, BGR32F, 12, 1., 1., 1.1, true );
REGISTER_TRANSFER( RGBA32F, RGBA16F, 8, .5, .5, 1., false );
REGISTER_TRANSFER( RGBA32F, BGRA16F, 8,.5, .5, 1.1, false );
REGISTER_TRANSFER( RGBA32F, RGB16F, 6, .5, .5, 1., true );
REGISTER_TRANSFER( RGBA32F, BGR16F, 6, .5, .5, 1.1, true );
REGISTER_TRANSFER( RGBA32F, RGBA, 4, .25, .25, 1., false );
REGISTER_TRANSFER( RGBA32F, BGRA, 4, .25, .25, 1.1, false );
REGISTER_TRANSFER( RGBA32F, RGB, 3, .25, .25, 1., true );
REGISTER_TRANSFER( RGBA32F, BGR, 3, .25, .25, 1.1, true );

REGISTER_TRANSFER( DEPTH, DEPTH_UNSIGNED_INT, 4, 1., 1., 1., false );
}

CompressorReadDrawPixels::CompressorReadDrawPixels( const unsigned name )
        : Compressor()
        , _texture( 0 )
        , _internalFormat( 0 )
        , _format( 0 )
        , _type( 0 )
        , _depth( _depths[ name ] )
{
    EQASSERT( _depth > 0 );
    switch( name )
    {
        case EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA:
            _format = GL_RGBA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA_UINT_8_8_8_8_REV:
            _format = GL_RGBA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA:
            _format = GL_RGBA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA16F:
            _format = GL_RGBA;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA:
            _format = GL_RGBA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA16F:
            _format = GL_RGBA;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA32F:
            _format = GL_RGBA;
            _type = GL_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;

        case EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA:
            _format = GL_BGRA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA_UINT_8_8_8_8_REV:
            _format = GL_BGRA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGB10_A2_TO_BGR10_A2:
            _format = GL_BGRA;
            _type = GL_UNSIGNED_INT_10_10_10_2;
            _internalFormat = GL_RGB10_A2;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGB10_A2_TO_RGB10_A2:
            _format = GL_RGBA;
            _type = GL_UNSIGNED_INT_10_10_10_2;
            _internalFormat = GL_RGB10_A2;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA:
            _format = GL_BGRA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F:
            _format = GL_BGRA;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA:
            _format = GL_BGRA;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA16F:
            _format = GL_BGRA;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F:
            _format = GL_BGRA;
            _type = GL_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGB:
            _format = GL_RGB;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGB:
            _format = GL_RGB;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGB16F:
            _format = GL_RGB;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGB:
            _format = GL_RGB;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGB32F:
            _format = GL_RGB;
            _type = GL_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGB16F:
            _format = GL_RGB;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGR:
            _format = GL_BGR;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGR:
            _format = GL_BGR;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGR16F:
            _format = GL_BGR;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA16F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR:
            _format = GL_BGR;
            _type = GL_UNSIGNED_BYTE;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR32F:
            _format = GL_BGR;
            _type = GL_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGR16F:
            _format = GL_BGR;
            _type = GL_HALF_FLOAT;
            _internalFormat = GL_RGBA32F;
            break;
        case EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT:
            _format = GL_DEPTH_COMPONENT;
            _type = GL_UNSIGNED_INT;
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

void CompressorReadDrawPixels::_init( const eq_uint64_t inDims[4],
                                            eq_uint64_t outDims[4] )
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

void CompressorReadDrawPixels::_initTexture( const GLEWContext* glewContext,
                                             const eq_uint64_t flags )
{
    GLenum target = 0;
    if( flags & EQ_COMPRESSOR_USE_TEXTURE_2D )
        target = GL_TEXTURE_2D;
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE_RECT )
        target = GL_TEXTURE_RECTANGLE_ARB;
    else
    {
        EQUNREACHABLE;
    }

    if ( !_texture || _texture->getTarget( ) != target )
    {
        if( _texture )
            delete _texture;
        _texture = new util::Texture( target, glewContext );
    }
}

void CompressorReadDrawPixels::download( const GLEWContext* glewContext,
                                         const eq_uint64_t  inDims[4],
                                         const unsigned     source,
                                         const eq_uint64_t  flags,
                                               eq_uint64_t  outDims[4],
                                         void**             out )
{
    _init( inDims, outDims );

    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glReadPixels( inDims[0], inDims[2], inDims[1], inDims[3], _format,
                      _type, _buffer.getData() );
    }
    else 
    {
        _initTexture( glewContext, flags );
        _texture->setGLData( source, _internalFormat, inDims[1], inDims[3] );
        _texture->setExternalFormat( _format, _type );
        _texture->download( _buffer.getData( ));
        _texture->flushNoDelete();
    }

    *out = _buffer.getData();
}

void CompressorReadDrawPixels::upload( const GLEWContext* glewContext, 
                                       const void*        buffer,
                                       const eq_uint64_t  inDims[4],
                                       const eq_uint64_t  flags,
                                       const eq_uint64_t  outDims[4],  
                                       const unsigned     destination )
{
    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glRasterPos2i( outDims[0], outDims[2] );
        glDrawPixels( outDims[1], outDims[3], _format, _type, buffer );
    }
    else
    {
        EQASSERT( outDims[0] == 0 && outDims[2]==0 ); // Implement me
        _initTexture( glewContext, flags );
        _texture->setGLData( destination, _internalFormat,
                             outDims[1], outDims[3] );
        _texture->setExternalFormat( _format, _type );
        _texture->upload( outDims[1] , outDims[3], buffer );
        _texture->flushNoDelete();
    }
}
}

}
