
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
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
    CompressorRLE4HF() {}
    virtual ~CompressorRLE4HF() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    

    static void* getNewCompressor( ){ return new eq::plugin::CompressorRLE4HF; }
    static void* getNewDecompressor( ){ return 0; }
    
    static void getInfo( EqCompressorInfo* const info )
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

    static Functions getFunctions()
    {
        Functions functions;
        functions.name               = EQ_COMPRESSOR_RLE_4_HALF_FLOAT;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        functions.decompress         = decompress;
        return functions;
    }

protected:
    void compress( const void* const inData, const eq_uint64_t nPixels, 
                   const bool useAlpha, const bool swizzle );
};

class CompressorDiffRLE4HF : public CompressorRLE4HF
{
public:
    CompressorDiffRLE4HF() {}
    virtual ~CompressorDiffRLE4HF() {}
            
    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                              const bool useAlpha )
        { compress( inData, nPixels, useAlpha, true ); }
        
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
        
        
    static void* getNewCompressor( ){ return new eq::plugin::CompressorDiffRLE4HF; }
    static void* getNewDecompressor( ){ return 0; }
        
    static void getInfo( EqCompressorInfo* const info )
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
        
    static Functions getFunctions()
        {
            Functions functions;
            functions.name               = EQ_COMPRESSOR_DIFF_RLE_4_HALF_FLOAT;
            functions.getInfo            = getInfo;
            functions.newCompressor      = getNewCompressor;       
            functions.decompress         = decompress;
            return functions;
        }
        
};

}
}
#endif // EQ_PLUGIN_COMPRESSORRLE4HF
