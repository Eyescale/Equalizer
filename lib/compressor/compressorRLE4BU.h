
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009, Makhinya Maxim
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

 
#ifndef EQ_PLUGIN_COMPRESSORRLE4BU
#define EQ_PLUGIN_COMPRESSORRLE4BU

#include "compressor.h"

namespace eq
{
namespace plugin
{
class CompressorRLE4BU : public Compressor
{
public:
    CompressorRLE4BU() {}
    virtual ~CompressorRLE4BU() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha );

    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );

    static void* getNewCompressor( ){ return new eq::plugin::CompressorRLE4BU; }
    static void* getNewDecompressor( ){ return 0; }

    static void getInfo( EqCompressorInfo* const info )
    {
        info->version       = EQ_COMPRESSOR_VERSION;
        info->name          = EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED;
        info->capabilities  = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D;
        info->tokenType     = EQ_COMPRESSOR_DATATYPE_4_BYTE;
        info->quality       = 1.f;
        info->ratio         =  .89f;
        info->speed         = 2.11f;
    }

    static Functions getFunctions()
    {
        Functions functions;
        functions.name          = EQ_COMPRESSOR_RLE_4_BYTE_UNSIGNED;
        functions.getInfo       = getInfo;
        functions.newCompressor = getNewCompressor;
        functions.decompress    = decompress;
        return functions;
    }
};

}
}
#endif // EQ_PLUGIN_COMPRESSORRLE4BU
