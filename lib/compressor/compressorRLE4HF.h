
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

 
#ifndef EQ_PLUGIN_RLE4HALFFLOATCOMPRESSOR
#define EQ_PLUGIN_RLE4HALFFLOATCOMPRESSOR

#include "compressorRLE.h"

namespace eq
{
namespace plugin
{
class CompressorRLE4HF : public CompressorRLE
{
public:
    
    /** @name CompressorRLE4HF */
    /*@{*/
    /** 
     * Compress data with an algorithm RLE and process it for 
     * each half float length 4 vector.
     * 
     * @param the number channel.
     */
    CompressorRLE4HF() {}
    
    /** @name compress */
    /*@{*/
    /**
     * compress Data.
     *
     * @param inData data to compress.
     * @param inSize number data to compress.
     * @param useAlpha use alpha channel in compression.
     */
    virtual void compress( const void* const inData, 
                           const eq_uint64_t inSize, 
                           const bool useAlpha );
    
    /** @name decompress */
    /*@{*/
    /**
     * uncompress Data.
     *
     * @param inData data(s) to compress.
     * @param inSizes size(s)of the data to compress.
     * @param outData result of uncompressed data.
     * @param outSize size of the result.
     */
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes,
                            const unsigned numInputs, void* const outData, 
                            const eq_uint64_t outSize, const bool useAlpha );
    
    /** @name getNewCompressor */
    /*@{*/
    /**
     * get a new instance of compressor RLE 4 half float.
     *
     */         
    static void* getNewCompressor( )
                                   { return new eq::plugin::CompressorRLE4HF; }
    
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
    static void  getInfo( EqCompressorInfo* const info )
    {
         info->version = EQ_COMPRESSOR_VERSION;
         info->name = EQ_COMPRESSOR_RLE_4_HALF_FLOAT;
         info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D |
                              EQ_COMPRESSOR_IGNORE_MSE;
         info->tokenType = EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT;
         info->quality = 1.f;
         info->ratio = .8f;
         info->speed = 0.95f;
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
        functions.name               = EQ_COMPRESSOR_RLE_4_HALF_FLOAT;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        functions.decompress         = decompress;
        return functions;
    }
private:
    void _compress( const uint16_t* const input, const eq_uint64_t size, 
                    Result** results,const bool useAlpha );
};

}
}
#endif
