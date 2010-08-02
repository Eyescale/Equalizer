
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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
 
#ifndef EQ_PLUGIN_COMPRESSORRLEB
#define EQ_PLUGIN_COMPRESSORRLEB

#include "compressor.h"

namespace eq
{
namespace plugin
{

class CompressorRLEB : public Compressor
{
public:
    CompressorRLEB( const EqCompressorInfo* info ): Compressor( info ) {}
    virtual ~CompressorRLEB() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    

    static void* getNewCompressor( const EqCompressorInfo* info  )
        { return new eq::plugin::CompressorRLEB( info ); }
    static void* getNewDecompressor( const EqCompressorInfo* info ){ return 0; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version       = EQ_COMPRESSOR_VERSION;
        info->name          = EQ_COMPRESSOR_RLE_BYTE;
        info->capabilities  = EQ_COMPRESSOR_DATA_1D;
        info->tokenType     = EQ_COMPRESSOR_DATATYPE_BYTE;
        info->quality       = 1.0f;
        info->ratio         = .7f;
        info->speed         = 1.0f;
    }

    static Functions getFunctions( )
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
}
}
#endif 
