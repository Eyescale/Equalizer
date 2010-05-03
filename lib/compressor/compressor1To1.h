
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
 
#ifndef EQ_PLUGIN_COMPRESSOR1TO1
#define EQ_PLUGIN_COMPRESSOR1TO1
#include "compressor.h"
#include <eq/base/buffer.h>
#include <GL/glew.h>
namespace eq
{

namespace util
{
    class Texture;
}

namespace plugin
{

class Compressor1TO1 : public Compressor
{
public:
    Compressor1TO1( uint32_t format, uint32_t type, uint32_t _depth );
    virtual ~Compressor1TO1();

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
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
protected:
    eq::base::Bufferb _buffer;
    util::Texture*    _texture;
    uint32_t          _format;         //!< the GL format
    uint32_t          _type;           //!< the GL type 
    uint32_t          _depth;          //!< the GL type 

    void _init( const uint64_t  inDims[4],
                      uint64_t  outDims[4] );
};

class Compressor1TO1Color8 : public Compressor1TO1
{
public:
    Compressor1TO1Color8(): Compressor1TO1( GL_RGBA, GL_UNSIGNED_BYTE, 4 ){}
    static void* getNewCompressor( ){ 
                                  return new eq::plugin::Compressor1TO1Color8; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_8;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE | 
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_BYTE;

        info->quality      = 1.0f;
        info->ratio        = 1.0f;
        info->speed        = 1.0f;

        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_BYTE;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name           = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_8;
        functions.newCompressor  = getNewCompressor;  
        functions.decompress     = 0;
        functions.getInfo        = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

};

class Compressor1TO1Color32F : public Compressor1TO1
{
public:
    Compressor1TO1Color32F(): Compressor1TO1( GL_RGBA, GL_FLOAT,16 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1Color32F; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_32F;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE | 
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT;

        info->quality      = 1.0f;
        info->ratio        = 1.0f;
        info->speed        = 1.0f;
        
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT;
        info->outputTokenSize = 16;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name           = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_32F;
        functions.newCompressor  = getNewCompressor;  
        functions.decompress     = 0;
        functions.getInfo        = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};

class Compressor1TO1Color16F : public Compressor1TO1
{
public:
    Compressor1TO1Color16F(): Compressor1TO1( GL_RGBA, GL_HALF_FLOAT, 8 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1Color16F; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_16F;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE | 
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_HALF;

        info->quality      = 1.0f;
        info->ratio        = 1.0f;
        info->speed        = 1.0f;
        
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_HALF;
        info->outputTokenSize = 8;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name           = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_16F;
        functions.newCompressor  = getNewCompressor;  
        functions.decompress     = 0;
        functions.getInfo        = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};
class Compressor1TO1Color10A2 : public Compressor1TO1
{
public:
    Compressor1TO1Color10A2(): Compressor1TO1( GL_RGBA, 
                                               GL_UNSIGNED_INT_10_10_10_2, 4 ){}
    static void* getNewCompressor( )
                             { return new eq::plugin::Compressor1TO1Color10A2; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_10A2;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE | 
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_10A2;

        info->quality      = 1.0f;
        info->ratio        = 1.0f;
        info->speed        = 1.0f;

        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_10A2;
        info->outputTokenSize = 16;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name           = EQ_COMPRESSOR_TRANSFER_1TO1_COLOR_10A2;
        functions.newCompressor  = getNewCompressor;  
        functions.decompress     = 0;
        functions.getInfo        = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};
class Compressor1TO1Depth8 : public Compressor1TO1
{
public:
    Compressor1TO1Depth8(): Compressor1TO1( GL_RGBA, GL_UNSIGNED_INT, 4 ){}
    static void* getNewCompressor( )
                                { return new eq::plugin::Compressor1TO1Depth8; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_TRANSFER_1TO1_DEPTH_8;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE | 
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_UNSIGNED;

        info->quality      = 1.0f;
        info->ratio        = 1.0f;
        info->speed        = 1.0f;

        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_UNSIGNED;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name            = EQ_COMPRESSOR_TRANSFER_1TO1_DEPTH_8;
        functions.newCompressor   = getNewCompressor;      
        functions.getInfo         = getInfo;
        functions.isCompatible   = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
    
};

}
}
#endif
