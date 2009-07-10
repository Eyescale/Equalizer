
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

#ifndef EQ_PLUGIN_RLE3BYTECOMPRESSOR
#define EQ_PLUGIN_RLE3BYTECOMPRESSOR

#include "compressorRLE.h"

namespace eq
{
namespace plugin
{

class CompressorRLE3B : public CompressorRLE
{
public:
    CompressorRLE3B() {}

    virtual void compress( const void* const inData, const eq_uint64_t inSize, 
                           const bool useAlpha );
    
    static void decompress( const void* const* inData,
                            const eq_uint64_t* const inSizes, 
                            const unsigned numInputs, void* const outData, 
                            const eq_uint64_t outSize, const bool useAlpha );

    static void* getNewCompressor() { return new eq::plugin::CompressorRLE3B; }
    static void* getNewDecompressor() { return 0; }

    static void getInfo( EqCompressorInfo* const info )
    {
         info->version = EQ_COMPRESSOR_VERSION;
         info->name = EQ_COMPRESSOR_RLE_3_BYTE;
         info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D;
         info->tokenType = EQ_COMPRESSOR_DATATYPE_3_BYTE;
         
         info->quality = 1.f;
         info->ratio = .8f;
         info->speed = 0.95f;
    }

    static Functions getFunctions()
    {
        Functions functions;
        functions.name               = EQ_COMPRESSOR_RLE_3_BYTE;
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
#endif
