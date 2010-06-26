
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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
 
#ifndef EQ_PLUGIN_TRANSFERINTERNALTOEXTERAL
#define EQ_PLUGIN_TRANSFERINTERNALTOEXTERAL
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
    CompressorReadDrawPixels( const EqCompressorInfo* info );
    
    static void* getNewCompressor( const EqCompressorInfo* info )
        { return new CompressorReadDrawPixels( info ); }
    virtual ~CompressorReadDrawPixels();

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool        useAlpha )
        { EQDONTCALL; }
    
    static bool isCompatible( const GLEWContext* glewContext );
    
    void download( GLEWContext*       glewContext,
                   const uint64_t     inDims[4],
                   const unsigned     source,
                   const uint64_t     flags,
                   uint64_t           outDims[4],
                   void**             out );

    void upload( GLEWContext*       glewContext, 
                 const void*        buffer,
                 const uint64_t     inDims[4],
                 const uint64_t     flags,
                 const uint64_t     outDims[4],  
                 const unsigned     destination );
static void getInfo0( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenSize = 4;
    }

    static void getInfo1( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA;
        info->outputTokenSize = 4;
    }

    static void getInfo2( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB_TO_RGB;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB;
        info->outputTokenSize = 3;
    }
    
    static void getInfo3( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB_TO_BGR;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR;
        info->outputTokenSize = 3;
    }

    static void getInfo4( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA_UINT_8_8_8_8_REV;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV;
        info->outputTokenSize = 4;
    }
    
    static void getInfo5( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGRA_UINT_8_8_8_8_REV;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV;
        info->outputTokenSize = 4;
    }

    static void getInfo6( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_RGB10A2;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB10_A2;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB10_A2;
        info->outputTokenSize = 4;
    }

    static void getInfo7( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB10A2_TO_BGR10A2;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB10_A2;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR10_A2;
        info->outputTokenSize = 4;
    }
    
    static void getInfo8( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA32F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenSize = 16;
    }

    static void getInfo9( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA32F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA32F;
        info->outputTokenSize = 16;
    }

    static void getInfo10( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB32F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenSize = 12;
    }

    static void getInfo11( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR32F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR32F;
        info->outputTokenSize = 12;
    }

    static void getInfo12( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA16F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA16F;
        info->outputTokenSize = 8;
    }

    static void getInfo13( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA16F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA16F;
        info->outputTokenSize = 8;
    }

    static void getInfo14( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB16F_TO_RGB16F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB16F;
        info->outputTokenSize = 6;
    }

    static void getInfo15( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR16F;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR16F;
        info->outputTokenSize = 6;
    }

    static void getInfo16( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 1.0, 1.0, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_DEPTH_TO_DEPTH_UNSIGNED_INT;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
        info->outputTokenSize = 4;
    }

    static void getInfo17( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.25, 0.25, 1.0 );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA_25P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA;
        info->outputTokenSize = 4;
    }

    static void getInfo18( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info,  0.25f, 0.25f, 0.9f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA_25P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenSize = 4;
    }

    static void getInfo19( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info,  0.5f, 0.5f, 0.9f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_RGBA16F_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA16F;
        info->outputTokenSize = 8;
    }

    static void getInfo20( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info,  0.5f, 0.5f, 1.0f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA32F_TO_BGRA16F_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA16F;
        info->outputTokenSize = 8;
    }

    static void getInfo21( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.5f, 0.5f, 1.0f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_BGRA_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA;
        info->outputTokenSize = 4;
    }

    static void getInfo22( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.5f, 0.5f, 0.9f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGBA16F_TO_RGBA_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA;
        info->outputTokenSize = 4;
    }

    static void getInfo23( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.25f, 0.25f, 1.0f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR_25P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR;
        info->outputTokenSize = 3;
    }

    static void getInfo24( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.25f, 0.25f, 0.9f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB_25P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB;
        info->outputTokenSize = 3;
    }

    static void getInfo25( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.5f, 0.5f, 0.9f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB32F_TO_RGB16F_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB16F;
        info->outputTokenSize = 6;
    }

    static void getInfo26( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.5f, 0.5f, 1.0f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB32F_TO_BGR16F_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB32F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR16F;
        info->outputTokenSize = 6;
    }

    static void getInfo27( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.5f, 0.5f, 1.0f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB16F_TO_BGR_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR;
        info->outputTokenSize = 3;
    }

    static void getInfo28( EqCompressorInfo* const info )
    {
        CompressorReadDrawPixels::getInfo( info, 0.5f, 0.5f, 0.9f );
        info->name            = EQ_COMPRESSOR_TRANSFER_RGB16F_TO_RGB_50P;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGB16F;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB;
        info->outputTokenSize = 3;
    }

    static Functions getFunctions( uint32_t index );

protected:
    eq::base::Bufferb _buffer;
    util::Texture*    _texture;
    uint32_t    _internalFormat;         //!< the GL format
    uint32_t    _format;         //!< the GL format
    uint32_t    _type;           //!< the GL type 
    uint32_t    _depth;          //!< the size of one output token

    void _init( const uint64_t  inDims[4],
                      uint64_t  outDims[4] );


    static void getInfo( EqCompressorInfo* const info, 
                         float quality, float ratio, float speed )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->quality      = quality;
        info->ratio        = ratio;
        info->speed        = speed;
    }

    
};

}
}
#endif
