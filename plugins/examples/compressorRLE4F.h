
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
 
#ifndef EQ_PLUGIN_COMPRESSORRLE4FLOAT
#define EQ_PLUGIN_COMPRESSORRLE4FLOAT

#include "compressorRLE.h"

namespace eq
{
namespace plugin
{
class CompressorRLE4F : public CompressorRLE
{
public:
    /** @name CompressorRLE4F */
    /*@{*/
    /** 
     * Compress data with an algorithm RLE and process it for 
     * each float length 4 vector.
     * 
     * @param the number channel.
     */
    CompressorRLE4F(): CompressorRLE( 4 ){} 
    
    /** @name compress */
    /*@{*/
    /**
     * compress Data.
     *
     * @param inData data to compress.
     * @param inSize number data to compress.
     * @param useAlpha use alpha channel in compression.
     */
    virtual void compress( void* const inData, 
                          const uint64_t inSize, 
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
    virtual void decompress( const void** const inData, 
                            const uint64_t* const inSizes, 
                            void* const outData, 
                            const uint64_t* const outSize );     
    
    static void* getNewCompressor( )
                                   { return new eq::plugin::CompressorRLE4F; }
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
         info->type = EQ_COMPRESSOR_RLE_4_FLOAT;
         info->capabilities = EQ_COMPRESSOR_DATA_2D | EQ_COMPRESSOR_IGNORE_MSE;
         info->tokenType = EQ_COMPRESSOR_DATATYPE_4_FLOAT;
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
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        return functions;
    }
private:
    void _compress( const uint32_t* input, const uint64_t size, 
                    Result** results,const bool useAlpha );
};







}
}
#endif //COMPRESSORRLE4FLOAT
