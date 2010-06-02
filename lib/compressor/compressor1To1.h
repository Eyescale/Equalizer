
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
    const uint32_t    _format;         //!< the GL format
    const uint32_t    _type;           //!< the GL type 
    const uint32_t    _depth;          //!< the size of one output token

    void _init( const uint64_t  inDims[4],
                      uint64_t  outDims[4] );

    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->capabilities = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_USE_TEXTURE |
                             EQ_COMPRESSOR_USE_FRAMEBUFFER;
        info->quality      = 1.0f;
        info->ratio        = 1.0f;
        info->speed        = 1.0f;
    }
};

class Compressor1TO1RGBAUnsignedByte : public Compressor1TO1
{
public:
    Compressor1TO1RGBAUnsignedByte(): Compressor1TO1( GL_RGBA, GL_UNSIGNED_BYTE, 4 ){}
    static void* getNewCompressor( ){ 
                                  return new Compressor1TO1RGBAUnsignedByte; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGBA_UNSIGNED_BYTE;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_BYTE;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_BYTE;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name           = EQ_TRANSFER_1TO1_RGBA_UNSIGNED_BYTE;
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

class Compressor1TO1RGBAInt8888rev : public Compressor1TO1
{
public:
    Compressor1TO1RGBAInt8888rev(): Compressor1TO1( GL_RGBA, 
                                            GL_UNSIGNED_INT_8_8_8_8_REV, 4 ){}
    static void* getNewCompressor( ){ 
                              return new Compressor1TO1RGBAInt8888rev; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGBA_UNSIGNED_INT_8_8_8_8_REV;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_8_8_8_8_REV;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_8_8_8_8_REV;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGBA_UNSIGNED_INT_8_8_8_8_REV;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

};

class Compressor1TO1RGBAInt1010102 : public Compressor1TO1
{
public:
    Compressor1TO1RGBAInt1010102(): Compressor1TO1( GL_RGBA, 
                                               GL_UNSIGNED_INT_10_10_10_2, 4 ){}
    static void* getNewCompressor( )
                             { return new eq::plugin::Compressor1TO1RGBAInt1010102; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGBA_UNSIGNED_INT_10_10_10_2;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_10_10_10_2;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UNSIGNED_INT_10_10_10_2;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGBA_UNSIGNED_INT_10_10_10_2;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
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

class Compressor1TO1RGBAFloat : public Compressor1TO1
{
public:
    Compressor1TO1RGBAFloat(): Compressor1TO1( GL_RGBA, GL_FLOAT, 16 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1RGBAFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name            = EQ_TRANSFER_1TO1_RGBA_FLOAT;
        info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA_FLOAT;
        info->outputTokenSize = 16;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGBA_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};

class Compressor1TO1RGBAHalfFloat : public Compressor1TO1
{
public:
    Compressor1TO1RGBAHalfFloat(): Compressor1TO1( GL_RGBA, GL_HALF_FLOAT, 8 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1RGBAHalfFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGBA_HALF_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGBA_HALF_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGBA_HALF_FLOAT;
        info->outputTokenSize = 8;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGBA_HALF_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
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

class Compressor1TO1BGRAUnsignedByte : public Compressor1TO1
{
public:
    Compressor1TO1BGRAUnsignedByte(): Compressor1TO1( GL_BGRA, GL_UNSIGNED_BYTE, 4 ){}
    static void* getNewCompressor( ){ 
                               return new eq::plugin::Compressor1TO1BGRAUnsignedByte; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGRA_UNSIGNED_BYTE;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_BYTE;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_BYTE;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGRA_UNSIGNED_BYTE;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

};

class Compressor1TO1BGRAInt8888rev : public Compressor1TO1
{
public:
    Compressor1TO1BGRAInt8888rev(): Compressor1TO1( GL_BGRA, 
                                            GL_UNSIGNED_INT_8_8_8_8_REV, 4 ){}
    static void* getNewCompressor( ){ 
                              return new eq::plugin::Compressor1TO1BGRAInt8888rev; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGRA_UNSIGNED_INT_8_8_8_8_REV;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_8_8_8_8_REV;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_8_8_8_8_REV;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGRA_UNSIGNED_INT_8_8_8_8_REV;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

};

class Compressor1TO1BGRAInt1010102 : public Compressor1TO1
{
public:
    Compressor1TO1BGRAInt1010102(): Compressor1TO1( GL_BGRA, 
                                               GL_UNSIGNED_INT_10_10_10_2, 4 ){}
    static void* getNewCompressor( )
                             { return new eq::plugin::Compressor1TO1RGBAInt1010102; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGRA_UNSIGNED_INT_10_10_10_2;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_10_10_10_2;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UNSIGNED_INT_10_10_10_2;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGRA_UNSIGNED_INT_10_10_10_2;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
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

class Compressor1TO1BGRAFloat : public Compressor1TO1
{
public:
    Compressor1TO1BGRAFloat(): Compressor1TO1( GL_BGRA, GL_FLOAT,16 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1RGBAFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGRA_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_FLOAT;
        info->outputTokenSize = 16;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGRA_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
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

class Compressor1TO1BGRAHalfFloat : public Compressor1TO1
{
public:
    Compressor1TO1BGRAHalfFloat(): Compressor1TO1( GL_BGRA, GL_HALF_FLOAT,8 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1BGRAHalfFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGRA_HALF_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGRA_HALF_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGRA_HALF_FLOAT;
        info->outputTokenSize = 8;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGRA_HALF_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
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

class Compressor1TO1RGBUnsignedByte : public Compressor1TO1
{
public:
    Compressor1TO1RGBUnsignedByte(): Compressor1TO1( GL_RGB, GL_UNSIGNED_BYTE, 3 ){}
    static void* getNewCompressor( ){ 
                                  return new eq::plugin::Compressor1TO1RGBUnsignedByte; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGB_UNSIGNED_BYTE;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGB_UNSIGNED_BYTE;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB_UNSIGNED_BYTE;
        info->outputTokenSize = 3;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGB_UNSIGNED_BYTE;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

};

class Compressor1TO1RGBFloat : public Compressor1TO1
{
public:
    Compressor1TO1RGBFloat(): Compressor1TO1( GL_RGB, GL_FLOAT, 12 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1RGBFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGB_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGB_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB_FLOAT;
        info->outputTokenSize = 12;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGB_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};

class Compressor1TO1RGBHalfFloat : public Compressor1TO1
{
public:
    Compressor1TO1RGBHalfFloat(): Compressor1TO1( GL_RGB, GL_HALF_FLOAT, 6 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1RGBHalfFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_RGB_HALF_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_RGB_HALF_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_RGB_HALF_FLOAT;
        info->outputTokenSize = 6;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_RGB_HALF_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};

class Compressor1TO1BGRUnsignedByte : public Compressor1TO1
{
public:
    Compressor1TO1BGRUnsignedByte(): Compressor1TO1( GL_BGR, GL_UNSIGNED_BYTE, 3 ){}
    static void* getNewCompressor( ){ 
                                  return new eq::plugin::Compressor1TO1BGRUnsignedByte; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGR_UNSIGNED_BYTE;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGR_UNSIGNED_BYTE;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR_UNSIGNED_BYTE;
        info->outputTokenSize = 3;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGR_UNSIGNED_BYTE;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

};

class Compressor1TO1BGRFloat : public Compressor1TO1
{
public:
    Compressor1TO1BGRFloat(): Compressor1TO1( GL_BGR, GL_FLOAT, 12 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1BGRFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGR_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGR_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR_FLOAT;
        info->outputTokenSize = 12;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGR_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};

class Compressor1TO1BGRHalfFloat : public Compressor1TO1
{
public:
    Compressor1TO1BGRHalfFloat(): Compressor1TO1( GL_BGR, GL_HALF_FLOAT, 6 ){}
    static void* getNewCompressor( )
                              { return new eq::plugin::Compressor1TO1BGRHalfFloat; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_BGR_HALF_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_BGR_HALF_FLOAT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_BGR_HALF_FLOAT;
        info->outputTokenSize = 6;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_BGR_HALF_FLOAT;
        functions.newCompressor = getNewCompressor;  
        functions.decompress    = 0;
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
};
class Compressor1TO1DepthUINT : public Compressor1TO1
{
public:
    Compressor1TO1DepthUINT(): Compressor1TO1( GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4 ){}
    static void* getNewCompressor( )
                                { return new eq::plugin::Compressor1TO1DepthUINT; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_DEPTH_UNSIGNED_INT ;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_DEPTH_UNSIGNED_INT ;
        functions.newCompressor = getNewCompressor;      
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
        return functions;
    }

    virtual void compress( const void* const inData, 
                           const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { EQDONTCALL; }

    bool isCompatible( const GLEWContext* glewContext )
        { return true; }
    
};

class Compressor1TO1DepthFLOAT : public Compressor1TO1
{
public:
    Compressor1TO1DepthFLOAT(): Compressor1TO1( GL_DEPTH_COMPONENT, GL_FLOAT, 4 ){}
    static void* getNewCompressor( )
                            { return new eq::plugin::Compressor1TO1DepthFLOAT; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_DEPTH_FLOAT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_DEPTH_FLOAT ;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_DEPTH_FLOAT;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name           = EQ_TRANSFER_1TO1_DEPTH_FLOAT;
        functions.newCompressor  = getNewCompressor;      
        functions.getInfo        = getInfo;
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

class Compressor1TO1DepthStencil248NV : public Compressor1TO1
{
public:
    Compressor1TO1DepthStencil248NV(): Compressor1TO1( GL_DEPTH_STENCIL_NV, 
                                            GL_UNSIGNED_INT_24_8_NV, 4 ){}
    static void* getNewCompressor( )
                        { return new eq::plugin::Compressor1TO1DepthStencil248NV; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        Compressor1TO1::getInfo( info );
        info->name         = EQ_TRANSFER_1TO1_DEPTH_UNSIGNED_INT_24_8_NV;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT_24_8_NV;
        info->outputTokenType = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT_24_8_NV;
        info->outputTokenSize = 4;
    }

    static Functions getFunctions( )
    {
        Functions functions;
        functions.name          = EQ_TRANSFER_1TO1_DEPTH_UNSIGNED_INT_24_8_NV;
        functions.newCompressor = getNewCompressor;      
        functions.getInfo       = getInfo;
        functions.isCompatible  = (IsCompatible_t)Compressor1TO1::isCompatible;
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
