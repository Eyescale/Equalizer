
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

 
#ifndef EQ_PLUGIN_COMPRESSORRLE4HF
#define EQ_PLUGIN_COMPRESSORRLE4HF

#include "compressor.h"

namespace eq
{
namespace plugin
{
class CompressorRLE4HF : public Compressor
{
public:
    CompressorRLE4HF( const EqCompressorInfo* info ): Compressor( info ) {}
    virtual ~CompressorRLE4HF() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    
    static void* getNewCompressor( const EqCompressorInfo* info )
        { return new eq::plugin::CompressorRLE4HF( info ); }

    static void* getNewDecompressor( const EqCompressorInfo* info ){ return 0; }
    
    static void getInfo0( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->name = EQ_COMPRESSOR_RLE_4_HALF_FLOAT;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D | 
                             EQ_COMPRESSOR_IGNORE_MSE;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT;
        info->quality = 1.f;
        info->ratio = .45f;
        info->speed = 1.f;
    }
    static void getInfo1( EqCompressorInfo* const info )
    {
        CompressorRLE4HF::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_RGBA16F;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA16F;
    }
    static void getInfo2( EqCompressorInfo* const info )
    {
        CompressorRLE4HF::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_BGRA16F;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA16F;
    }

    static Functions getFunctions( uint32_t index )
    {
        Functions functions;
        switch ( index )
        {
        case 0: functions.getInfo = CompressorRLE4HF::getInfo0;
                break;
        case 1: functions.getInfo = CompressorRLE4HF::getInfo1;
                break;
        case 2: functions.getInfo = CompressorRLE4HF::getInfo2;
                break;
        }
        functions.newCompressor = getNewCompressor;       
        functions.newDecompressor = getNewDecompressor;       
        functions.decompress = decompress;
        return functions;
    }

protected:
    void compress( const void* const inData, const eq_uint64_t nPixels, 
                   const bool useAlpha, const bool swizzle );
};

class CompressorDiffRLE4HF : public CompressorRLE4HF
{
public:
    CompressorDiffRLE4HF( const EqCompressorInfo* info )
        : CompressorRLE4HF( info ){}
    virtual ~CompressorDiffRLE4HF() {}
            
    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                              const bool useAlpha )
        { CompressorRLE4HF::compress( inData, nPixels, useAlpha, true ); }
        
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
        
        
    static void* getNewCompressor( const EqCompressorInfo* info )
        { return new eq::plugin::CompressorDiffRLE4HF( info ); }

    static void getInfo0( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->name = EQ_COMPRESSOR_DIFF_RLE_4_HALF_FLOAT;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D | 
        EQ_COMPRESSOR_IGNORE_MSE;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT;
        info->quality = 1.f;
        info->ratio = .95f;
        info->speed = 1.f;
    }
    
    static void getInfo1( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4HF::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_RGBA16F;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA16F;
    }
    
    static void getInfo2( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4HF::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_BGRA16F;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA16F;
    }
    
    static Functions getFunctions( uint32_t index )
        {
            Functions functions;
            switch ( index )
            {
            case 0: functions.getInfo = CompressorDiffRLE4HF::getInfo0;
                    break;
            case 1: functions.getInfo = CompressorDiffRLE4HF::getInfo1;
                    break;
            case 2: functions.getInfo = CompressorDiffRLE4HF::getInfo2;
                    break;
            }
            functions.newCompressor = getNewCompressor;       
            functions.newDecompressor = getNewDecompressor;       
            functions.decompress = decompress;
            return functions;
        }
        
};

}
}
#endif // EQ_PLUGIN_COMPRESSORRLE4HF
