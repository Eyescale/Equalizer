
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_PLUGIN_COMPRESSORYUV
#define EQ_PLUGIN_COMPRESSORYUV

#include "compressor.h"

#include <eq/util/texture.h>
#include <eq/client/gl.h>
#include <lunchbox/buffer.h>

namespace eq
{
namespace util
{
    class FrameBufferObject;
}
namespace plugin
{

class CompressorYUV : public Compressor
{
public:
    CompressorYUV();
    virtual ~CompressorYUV();

    static void* getNewCompressor( const unsigned name  )
        { return new CompressorYUV(); }
    static void* getNewDecompressor( const unsigned name  )
        { return new CompressorYUV(); }

    virtual void compress( const void* const inData, 
                           const uint64_t    nPixels, 
                           const bool        useAlpha )
        { LBDONTCALL; }

    static bool isCompatible( const GLEWContext* glewContext );

    virtual void download( const GLEWContext* glewContext,
                           const eq_uint64_t  inDims[4],
                           const unsigned     source,
                           const eq_uint64_t  flags,
                                 eq_uint64_t  outDims[4],
                                 void**       out );

    virtual void upload( const GLEWContext* glewContext, 
                         const void*        data,
                         const eq_uint64_t  inDims[4],
                         const eq_uint64_t  flags,
                         const eq_uint64_t  outDims[4],  
                         const unsigned     destination );

protected:
    GLuint   _program;
    lunchbox::Bufferb buffer;

private:
    void _initShader( const GLEWContext* glewContext, const char* fShaderPtr );
    void _compress( const GLEWContext* glewContext,
                    const eq_uint64_t inDims[4],
                    eq_uint64_t       outDims[4] );
    void _download( void* data );

    void _decompress( const GLEWContext* glewContext,
                      const eq_uint64_t  inDims[4] );
    
    util::FrameBufferObject* _fbo;
    util::Texture* _texture;

};

}
}
#endif //EQ_PLUGIN_COMPRESSORYUV
