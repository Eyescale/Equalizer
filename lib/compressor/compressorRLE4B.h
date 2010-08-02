
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
 
#ifndef EQ_PLUGIN_COMPRESSORRLE4B
#define EQ_PLUGIN_COMPRESSORRLE4B

#include "compressor.h"

namespace eq
{
namespace plugin
{

class CompressorRLE4B : public Compressor
{
public:
    CompressorRLE4B( const EqCompressorInfo* info ) : Compressor( info ) {}
    virtual ~CompressorRLE4B() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    

    static void* getNewCompressor( const EqCompressorInfo* info )
                           { return new eq::plugin::CompressorRLE4B( info ); }
    static void* getNewDecompressor( const EqCompressorInfo* info ){ return 0; }
    
    static void getInfo0( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->name = EQ_COMPRESSOR_RLE_4_BYTE;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_4_BYTE;

        info->quality = 1.0f;
        info->ratio   = .59f;
        info->speed   = 1.0f;
    }
    static void getInfo1( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_RGBA;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA;
    }

    static void getInfo2( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_BGRA;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA;
    }
            
    static void getInfo3( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_RGBA_UINT_8_8_8_8_REV;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV;
    }
    
    static void getInfo4( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_BGRA_UINT_8_8_8_8_REV;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV;
    }

    static void getInfo5( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_RGB10_A2;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGB10_A2;
    }
    
    static void getInfo6( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_RLE_BGR10_A2;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGR10_A2;
    }

    static void getInfo7( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name         = EQ_COMPRESSOR_RLE_UNSIGNED;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_UNSIGNED;
    }
    
    static void getInfo8( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name         = EQ_COMPRESSOR_RLE_DEPTH_UNSIGNED_INT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
    }
    
    static Functions getFunctions( uint32_t index )
    {
        Functions functions;
        switch ( index )
        {
        case 0: functions.getInfo = getInfo0;
                break;
        case 1: functions.getInfo = getInfo1;
                break;
        case 2: functions.getInfo = getInfo2;
                break;
        case 3: functions.getInfo = getInfo3;
                break;
        case 4: functions.getInfo = getInfo4;
                break;
        case 5: functions.getInfo = getInfo5;
                break;
        case 6: functions.getInfo = getInfo6;
                break;
        case 7: functions.getInfo = getInfo7;
                break;
        case 8: functions.getInfo = getInfo8;
                break;
        }
        functions.newCompressor   = getNewCompressor;       
        functions.newDecompressor = getNewDecompressor;       
        functions.decompress      = decompress;
        return functions;
    }

protected:
    void compress( const void* const inData, const eq_uint64_t nPixels, 
                   const bool useAlpha, const bool swizzle );
};



class CompressorDiffRLE4B : public CompressorRLE4B
{
public:
    CompressorDiffRLE4B( const EqCompressorInfo* info ) 
        : CompressorRLE4B( info ) {}
    virtual ~CompressorDiffRLE4B() {}

    /** @name getNewCompressor */
    /*@{*/
    /**
     * get a new instance of compressor RLE 4 bytes and swizzle data.
     *
     */         
    static void* getNewCompressor( const EqCompressorInfo* info )
                     { return new eq::plugin::CompressorDiffRLE4B( info ); }
    
    /** @name getInfo */
    /*@{*/
    /**
     * get information about this compressor.
     *
     * @param info about this compressor.
     */
    static void getInfo0( EqCompressorInfo* const info )
    {
        CompressorRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_4_BYTE;
        info->ratio = 0.50f;
        info->speed = 1.1f;
    }
    static void getInfo1( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_RGBA;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA;
    }
    static void getInfo2( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_BGRA;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA;
    }
            
    static void getInfo3( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_RGBA_UINT_8_8_8_8_REV;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV;
    }
    
    static void getInfo4( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_BGRA_UINT_8_8_8_8_REV;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV;
    }

    static void getInfo5( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_RGB10_A2;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGB10_A2;
    }
    
    static void getInfo6( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_BGR10_A2;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGR10_A2;
    }

    static void getInfo7( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name         = EQ_COMPRESSOR_DIFF_RLE_UNSIGNED;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_UNSIGNED;
    }
    
    static void getInfo8( EqCompressorInfo* const info )
    {
        CompressorDiffRLE4B::getInfo0( info );
        info->name         = EQ_COMPRESSOR_DIFF_RLE_DEPTH_UNSIGNED_INT;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_DEPTH_UNSIGNED_INT;
    }
    /** @name getFunctions */
    /*@{*/
    /** @return the function pointer list for this compressor. */
    static Functions getFunctions( uint32_t index )
    {
        Functions functions;
        switch ( index )
        {
        case 0: functions.getInfo = CompressorDiffRLE4B::getInfo0;
                break;
        case 1: functions.getInfo = CompressorDiffRLE4B::getInfo1;
                break;
        case 2: functions.getInfo = CompressorDiffRLE4B::getInfo2;
                break;
        case 3: functions.getInfo = CompressorDiffRLE4B::getInfo3;
                break;
        case 4: functions.getInfo = CompressorDiffRLE4B::getInfo4;
                break;
        case 5: functions.getInfo = CompressorDiffRLE4B::getInfo5;
                break;
        case 6: functions.getInfo = CompressorDiffRLE4B::getInfo6;
                break;
        case 7: functions.getInfo = CompressorDiffRLE4B::getInfo7;
                break;
        case 8: functions.getInfo = CompressorDiffRLE4B::getInfo8;
                break;
        }
        functions.newCompressor = getNewCompressor;
        functions.newDecompressor = getNewDecompressor;       
        functions.decompress = decompress;
        return functions;
    }

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { CompressorRLE4B::compress( inData, nPixels, useAlpha, true ); }

    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
};    
}
}
#endif // EQ_PLUGIN_COMPRESSORRLE4B
