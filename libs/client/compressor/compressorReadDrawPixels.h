
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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
 
#ifndef EQ_PLUGIN_COMPRESSORREADDRAWPIXELS
#define EQ_PLUGIN_COMPRESSORREADDRAWPIXELS

#include "compressor.h"

#include <GL/glew.h>

namespace eq
{
namespace util{ class Texture; }

namespace plugin
{

class CompressorReadDrawPixels : public Compressor
{
public:
    CompressorReadDrawPixels( const unsigned name );
    virtual ~CompressorReadDrawPixels();
    
    static void* getNewCompressor( const unsigned name )
        { return new CompressorReadDrawPixels( name ); }

    static void* getNewDecompressor( const unsigned name )
        { return new CompressorReadDrawPixels( name ); }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool        useAlpha )
        { EQDONTCALL; }
    
    static bool isCompatible( const GLEWContext* glewContext );
    
    virtual void download( const GLEWContext* glewContext,
                           const eq_uint64_t  inDims[4],
                           const unsigned     source,
                           const eq_uint64_t  flags,
                                 eq_uint64_t  outDims[4],
                           void**             out );

    virtual void upload( const GLEWContext* glewContext, 
                         const void*        buffer,
                         const eq_uint64_t  inDims[4],
                         const eq_uint64_t  flags,
                         const eq_uint64_t  outDims[4],
                         const unsigned     destination );

protected:
    co::base::Bufferb _buffer;
    util::Texture*    _texture;
    unsigned    _internalFormat; //!< the GL format
    unsigned    _format;         //!< the GL format
    unsigned    _type;           //!< the GL type 
    const unsigned _depth;       //!< the size of one output token
    void _initTexture( const GLEWContext* glewContext, const eq_uint64_t flags );
    void _init( const eq_uint64_t  inDims[4], eq_uint64_t  outDims[4] );
};

}
}
#endif // EQ_PLUGIN_COMPRESSORREADDRAWPIXELS
