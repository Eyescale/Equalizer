
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

#define glewGetContext() glewContext


// used to address one shader and program per shared context set
namespace eq
{
namespace plugin
{

Compressor1TO1::Compressor1TO1( uint32_t format, uint32_t type, 
                                uint32_t depth ) 
        : Compressor()
        , _texture( 0 )
        , _format( format )
        , _type( type )
        , _depth( depth )
{ }

bool Compressor1TO1::isCompatible( const GLEWContext* glewContext )
{
    return ( GL_VERSION_1_2 );
}

Compressor1TO1::~Compressor1TO1( )
{ 
    delete _texture;
    _texture = 0;
}

void Compressor1TO1::_init( const uint64_t  inDims[4],
                                  uint64_t  outDims[4] )
{
    outDims[0] = inDims[0];
    outDims[1] = inDims[1];
    outDims[2] = inDims[2];
    outDims[3] = inDims[3];
}

void Compressor1TO1::download( GLEWContext*       glewContext,
                               const uint64_t  inDims[4],
                               const unsigned     source,
                               const uint64_t  flags,
                               uint64_t        outDims[4],
                               void**             out )
{
    _buffer.resize( inDims[1] * inDims[3] * _depth );
    _init( inDims, outDims );

    if ( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glReadPixels( inDims[0], inDims[2], inDims[1], inDims[3], _format,
                      _type, _buffer.getData() );
    }
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE )
    {
        if ( !_texture )
        {
            _texture = new util::Texture( glewContext );
            _texture->setInternalFormat( _format );
        }
        
        _texture->setGLData( source, inDims[1], inDims[3] );
        _texture->download( _buffer.getData(), _format, _type );
        _texture->flushNoDelete();
    }
    out[0] = _buffer.getData();
}

void Compressor1TO1::upload( GLEWContext*       glewContext, 
                             const void*        buffer,
                             const uint64_t     inDims[4],
                             const uint64_t     flags,
                             const uint64_t     outDims[4],  
                             const unsigned     destination )
{
    _buffer.resize( inDims[1] * inDims[3] * _depth );

    if ( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        glRasterPos2i( outDims[0], outDims[2] );
        glDrawPixels( outDims[1], outDims[3], _format, _type, buffer );
    }
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE )
    {
        if( !_texture )
        {
            _texture = new util::Texture( glewContext );
            _texture->setInternalFormat( _format );
        }
        _texture->setGLData( destination, outDims[1], outDims[3] );
        _texture->upload( outDims[1] , outDims[3], buffer );
        _texture->flushNoDelete();
    }
}
}

}
