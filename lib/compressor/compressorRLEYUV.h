
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
 
#ifndef EQ_PLUGIN_COMPRESSORRLEYUV
#define EQ_PLUGIN_COMPRESSORRLEYUV

#include "compressor.h"

namespace eq
{
namespace plugin
{

class CompressorRLEYUV : public Compressor
{
public:
    CompressorRLEYUV( const EqCompressorInfo* info ): Compressor( info ) {}
    virtual ~CompressorRLEYUV() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    
    static void* getNewCompressor( const EqCompressorInfo* info )
        { return new eq::plugin::CompressorRLEYUV( info ); }

    static void* getNewDecompressor( const EqCompressorInfo* info ){ return 0; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->name = EQ_COMPRESSOR_RLE_YUVA_50P;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_YUVA_50P;

        info->quality = 1.0f;
        info->ratio   = .59f;
        info->speed   = 1.0f;
    }

    static Functions getFunctions()
    {
        Functions functions;
        functions.getInfo = getInfo;
        functions.newCompressor = getNewCompressor;       
        functions.newDecompressor = getNewDecompressor;       
        functions.decompress = decompress;
        return functions;
    }

protected:
    void compress( const void* const inData, const eq_uint64_t nPixels, 
                   const bool useAlpha, const bool swizzle );
};

class CompressorDiffRLEYUV : public CompressorRLEYUV
{
public:
    CompressorDiffRLEYUV( const EqCompressorInfo* info )
        : CompressorRLEYUV( info ) {}
    virtual ~CompressorDiffRLEYUV() {}

    static void* getNewCompressor( const EqCompressorInfo* info  )
        { return new eq::plugin::CompressorDiffRLEYUV( info ); }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        CompressorRLEYUV::getInfo( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_YUVA_50P;
        info->ratio = 0.50f;
        info->speed = 1.1f;
    }
    
    /** @return the function pointer list for this compressor. */
    static Functions getFunctions()
    {
        Functions functions;
        functions.getInfo = getInfo;
        functions.newCompressor = getNewCompressor;       
        functions.newDecompressor = getNewDecompressor;       
        functions.decompress = decompress;
        return functions;
    }

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { CompressorRLEYUV::compress( inData, nPixels, useAlpha, true ); }

    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
};    
}
}
#endif // EQ_PLUGIN_COMPRESSORRLEYUV
