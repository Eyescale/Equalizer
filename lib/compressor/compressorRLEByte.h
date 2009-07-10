
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
 
#ifndef EQ_PLUGIN_RLE1BYTECOMPRESSOR
#define EQ_PLUGIN_RLE1BYTECOMPRESSOR

#include "compressorRLE.h"

namespace eq
{
namespace plugin
{
class CompressorRLEByte : public CompressorRLE
{
public:
    /** @name CompressorRLEByte */
    /*@{*/
    /** 
     * Compress data with an algorithm RLE and process it for each byte.
     * 
     * @param the number channel.
     */
    CompressorRLEByte(){}
    
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
                            const eq_uint64_t outSize, const bool useAlpha);
    
    /** @name getNewCompressor */
    /*@{*/
    /**
     * get a new instance of compressor RLE bytes.
     *
     */         
    static void* getNewCompressor( )
                                   { return new eq::plugin::CompressorRLEByte; }
    
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
         info->name = EQ_COMPRESSOR_RLE_BYTE;
         info->capabilities = EQ_COMPRESSOR_DATA_1D;
         info->tokenType = EQ_COMPRESSOR_DATATYPE_BYTE;

         info->capabilities = EQ_COMPRESSOR_DATA_1D;
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
        functions.name               = EQ_COMPRESSOR_RLE_BYTE;
        functions.getInfo            = getInfo;
        functions.newCompressor      = getNewCompressor;       
        functions.decompress         = decompress;
        return functions;
    }

private:
    void _compress( const uint8_t* const input, const eq_uint64_t size, 
                    Result** results );
};
}
}
#endif // EQ_PLUGIN_RLE1BYTECOMPRESSOR

