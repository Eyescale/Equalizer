
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
 
#ifndef EQ_PLUGIN_RLE4BYTECOMPRESSOR
#define EQ_PLUGIN_RLE4BYTECOMPRESSOR

#include "compressorRLE.h"

namespace eq
{
namespace plugin
{

class CompressorRLE4B : public CompressorRLE
{
public:
    CompressorRLE4B(): CompressorRLE( 4 ){}
    
    virtual ~CompressorRLE4B(){} 

    virtual void compress(  void* const inData, 
                            const uint64_t inSize, 
                            const bool useAlpha );

    virtual void decompress( const void** const inData, 
                             const uint64_t* const inSizes, 
                             void* const outData, 
                             const uint64_t* const outSize );    
    

    static void* getNewCompressor( ){ return new eq::plugin::CompressorRLE4B; }
    
    static void* getNewDecompressor( ){ return 0; }

    static void getInfo( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->type = EQ_COMPRESSOR_RLE_4_BYTE;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_4_BYTE;
        info->quality = 1.f;
        info->ratio = .8f;
        info->speed = 0.95f;
    }

    static Functions getFunctions()
    {
        Functions functions;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        return functions;
    }

private:
    void _compress( const uint8_t* input, const uint64_t size, Result** results,
                    const bool ignoreAlpha );
    void _swizzlePixelData( uint32_t* data, const bool useAlpha );
    void _unswizzlePixelData( uint32_t* data, const bool useAlpha  );
};



class CompressorDiffRLE4B : public CompressorRLE4B
{
public:
    CompressorDiffRLE4B():CompressorRLE4B() 
    { 
        _swizzleData = true; 
        _name = EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
    }
    
    static void* getNewCompressor( ){ return new eq::plugin::CompressorDiffRLE4B; }
    
    static void* getNewDecompressor( ){ return 0; }

    static void getDeleteCompressor( void* const compressor )
    {
        delete ( eq::plugin::CompressorDiffRLE4B* )( compressor );
    }

    static void getDeleteDecompressor( void* const decompressor )
    {
        delete ( eq::plugin::CompressorDiffRLE4B* )( decompressor );
    }


    static void getInfo( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo( info );
        info->type = EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
        info->ratio = .7f;
        info->speed = 0.9f;
    }
    static Functions getFunctions()
    {
        Functions functions;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        return functions;
    }
};    
}
}
#endif
