
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

 
#ifndef CO_PLUGIN_COMPRESSORRLE4HF
#define CO_PLUGIN_COMPRESSORRLE4HF

#include "compressor.h"

namespace co
{
namespace plugin
{
class CompressorRLE4HF : public Compressor
{
public:
    CompressorRLE4HF(): Compressor() {}
    virtual ~CompressorRLE4HF() {}

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha )
        { compress( inData, nPixels, useAlpha, false ); }
    
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    
    static void* getNewCompressor( const unsigned name )
        { return new co::plugin::CompressorRLE4HF; }

    static void* getNewDecompressor( const unsigned name ){ return 0; }
    
protected:
    void compress( const void* const inData, const eq_uint64_t nPixels, 
                   const bool useAlpha, const bool swizzle );
};

class CompressorDiffRLE4HF : public CompressorRLE4HF
{
public:
    CompressorDiffRLE4HF() : CompressorRLE4HF() {}
    virtual ~CompressorDiffRLE4HF() {}
            
    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                              const bool useAlpha )
        { CompressorRLE4HF::compress( inData, nPixels, useAlpha, true ); }
        
    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );

    static void* getNewCompressor( const unsigned name )
        { return new co::plugin::CompressorDiffRLE4HF; }

};

}
}
#endif // CO_PLUGIN_COMPRESSORRLE4HF
