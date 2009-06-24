
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
    CompressorRLE4B() {}

    virtual void compress( const void* const inData, const uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const uint64_t* const inSizes, 
                            const unsigned numInputs, void* const outData, 
                            const uint64_t outSize, const bool useAlpha);
    

    static void* getNewCompressor( ){ return new eq::plugin::CompressorRLE4B; }
    
    static void* getNewDecompressor( ){ return 0; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->name = EQ_COMPRESSOR_RLE_4_BYTE;
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
        functions.name               = EQ_COMPRESSOR_RLE_4_BYTE;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        functions.decompress         = decompress;
        return functions;
    }

protected:
    void compress( const void* const inData, const uint64_t nPixels, 
                   const bool useAlpha, const bool swizzle );
};



class CompressorDiffRLE4B : public CompressorRLE4B
{
public:
    /** @name CompressorDiffRLE4B */
    /*@{*/
    /** 
     * Compress data with an algorithm RLE diff and process it for 
     * each byte length 4 vector.
     * 
     * @param the number channel.
     */
    CompressorDiffRLE4B():CompressorRLE4B() 
    {}
    
    /** @name getNewCompressor */
    /*@{*/
    /**
     * get a new instance of compressor RLE 4 bytes and swizzle data.
     *
     */         
    static void* getNewCompressor( )
                                 { return new eq::plugin::CompressorDiffRLE4B; }
    
    /** @name getNewDecompressor */
    /*@{*/
    /**
     * NOT IMPLEMENTED.
     *
     */
    static void* getNewDecompressor( ){ return 0; }

    /** @name getInfo */
    /*@{*/
    /**
     * get information about this compressor.
     *
     * @param info about this compressor.
     */
    static void getInfo( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
        info->ratio = .9f;
        info->speed = 0.9f;
    }
    
    /** @name getFunctions */
    /*@{*/
    /**
     * get the pointer functions for work with.
     *
     * @param info about this compressor.
     */
    static Functions getFunctions()
    {
        Functions functions;
        functions.name               = EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        functions.decompress         = decompress;
        return functions;
    }

    virtual void compress( const void* const inData, const uint64_t nPixels, 
                           const bool useAlpha )
        { CompressorRLE4B::compress( inData, nPixels, useAlpha, true ); }

    static void decompress( const void* const* inData, 
                            const uint64_t* const inSizes, 
                            const unsigned numInputs, void* const outData, 
                            const uint64_t outSize, const bool useAlpha );
};    
}
}
#endif
