
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@equalizergraphics.com>
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

#include "compressor1To1.h"
#include <eq/util/texture.h>
#include <GL/glew.h>
#define glewGetContext() glewContext


// used to address one shader and program per shared context set
namespace eq
{
namespace plugin
{

/** Construct a new compressor Yuv */
Compressor1TO1::Compressor1TO1( ) 
        : _texture( 0 )
{ }

bool Compressor1TO1::isCompatible( const GLEWContext* glewContext )
{
    return ( GL_ARB_texture_non_power_of_two && GL_VERSION_1_2 );
}

Compressor1TO1Color8::Compressor1TO1Color8()
{
    _format = GL_RGBA;
    _type   = GL_UNSIGNED_BYTE;
    _depth  = 4;
    _bufferType = BUFFER_COLOR;
}

Compressor1TO1Color32F::Compressor1TO1Color32F()
{
    _format = GL_RGBA;
    _type   = GL_FLOAT;
    _depth  = 16;
    _bufferType = BUFFER_COLOR;
}

Compressor1TO1Color16F::Compressor1TO1Color16F()
{
    _format = GL_RGBA;
    _type   = GL_HALF_FLOAT;
    _depth  = 8;
    _bufferType = BUFFER_COLOR;
}

Compressor1TO1Color10A2::Compressor1TO1Color10A2()
{
    _format = GL_RGBA;
    _type   = GL_UNSIGNED_INT_10_10_10_2;
    _depth  = 4;
    _bufferType = BUFFER_COLOR;
}

Compressor1TO1Depth8::Compressor1TO1Depth8()
{
    _format = GL_DEPTH_COMPONENT;
    _type   = GL_UNSIGNED_INT; 
    _depth  = 4;
    _bufferType = BUFFER_DEPTH;
}

/** Destruct the compressor Yuv */
Compressor1TO1::~Compressor1TO1( )
{ 
    if( _texture )
        delete _texture;
}

void Compressor1TO1::_init( const eq_uint64_t  inDims[4],
                            eq_uint64_t        outDims[4] )
{
    outDims[0] = inDims[0];
    outDims[1] = inDims[1];
    outDims[2] = inDims[2];
    outDims[3] = inDims[3];
}

void Compressor1TO1::download( GLEWContext*       glewContext,
                               const eq_uint64_t  inDims[4],
                               const unsigned     source,
                               const eq_uint64_t  flags,
                               eq_uint64_t        outDims[4],
                               void**             out )
{
    _buffer.resize( inDims[1] * inDims[3] * _depth );
    _init( inDims, outDims );

    if ( ( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER ) == 
         EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glReadPixels( inDims[0], inDims[2], inDims[1], inDims[3], _format,
                      _type, _buffer.getData() );
    }
    else if( ( flags & EQ_COMPRESSOR_USE_TEXTURE ) == 
             EQ_COMPRESSOR_USE_TEXTURE )
    {
        if ( !_texture )
        {

            _texture = new util::Texture( glewContext );
            _texture->setInternalFormat( _format );
        }
        
        _texture->setGLData( source, outDims[1], outDims[3] );
        _texture->download( _buffer.getData(), _format, _type );
        _texture->flushNoDelete();
    }
    out[0] = _buffer.getData();
}

void Compressor1TO1::upload( GLEWContext*       glewContext, 
                             const void*        buffer,
                             const eq_uint64_t  inDims[4],
                             const eq_uint64_t  flags,
                             const eq_uint64_t  outDims[4],  
                             const unsigned     destination )
{
    _buffer.resize( inDims[1] * inDims[3] * _depth );

    if ( ( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER ) == 
         EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glRasterPos2i( outDims[0], outDims[2] );
        glDrawPixels( outDims[1], outDims[3], _format,
                      _type, buffer );
    }
    else if( ( flags & EQ_COMPRESSOR_USE_TEXTURE ) == 
             EQ_COMPRESSOR_USE_TEXTURE )
    {
        if( !_texture )
        {
            _texture = new util::Texture( glewContext );
            _texture->setInternalFormat( _format );
        }
        _texture->setGLData( destination, outDims[1], outDims[3] );
        _texture->upload( outDims[1] , outDims[3], const_cast<void*>( buffer ) );
        _texture->flushNoDelete();
    }
}
}

}
