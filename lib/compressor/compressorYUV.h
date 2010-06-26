
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
#include <eq/base/buffer.h>
#include <eq/util/texture.h>
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
    CompressorYUV( const EqCompressorInfo* info );
    virtual ~CompressorYUV();

    static void* getNewCompressor( const EqCompressorInfo* info  ){ 
                     return new CompressorYUV( info ); }

    virtual void compress( const void* const inData, 
                           const uint64_t    nPixels, 
                           const bool        useAlpha )
        { EQDONTCALL; }

    static bool isCompatible( const GLEWContext* glewContext );
    
    virtual void download( GLEWContext*    glewContext,
                           const uint64_t  inDims[4],
                           const unsigned  source,
                           const uint64_t  flags,
                           uint64_t        outDims[4],
                           void**          out );

    virtual void upload( GLEWContext*    glewContext, 
                         const void*     datas,
                         const uint64_t  inDims[4],
                         const uint64_t  flags,
                         const uint64_t  outDims[4],  
                         const unsigned  destination );

    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_TRANSFER_RGBA_TO_YUV_50P;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_USE_TEXTURE | 
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGBA;

        info->quality      = 0.5f;
        info->ratio        = 0.5f;
        info->speed        = 0.9f;
        
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_YUV;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.newCompressor  = getNewCompressor;  
        functions.decompress     = 0;
        functions.getInfo        = getInfo;
        functions.isCompatible = (IsCompatible_t)CompressorYUV::isCompatible;
        return functions;
    }
protected:
    GLuint   _program;
    eq::base::Bufferb buffer;

private:
    void _init( GLEWContext* glewContext, const char* fShaderPtr );
    void _compress( GLEWContext*   glewContext,
                    const uint64_t inDims[4],
                    uint64_t       outDims[4] );
    void _download( void* datas );

    void _uncompress( GLEWContext* glewContext,
                      const uint64_t  inDims[4],
                      const uint64_t  outDims[4]);
    
    util::FrameBufferObject* _fbo;
    util::Texture* _texture;

};

}
}
#endif
