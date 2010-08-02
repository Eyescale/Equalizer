
/* Copyright (c) 2009-2010, Sarah Amsellem <sarah.amsellem@gmail.com> 
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
 
#ifndef EQ_PLUGIN_COMPRESSOR_DIFF_RLE_565
#define EQ_PLUGIN_COMPRESSOR_DIFF_RLE_565

#include "compressor.h"

namespace eq
{
namespace plugin
{

class CompressorRLE565 : public Compressor
{
public:
    CompressorRLE565( const EqCompressorInfo* info ): Compressor( info ) {}
    virtual ~CompressorRLE565() {}

    /** Get a new instance of this compressor */
    static void* getNewCompressor( const EqCompressorInfo* info )
        { return new eq::plugin::CompressorRLE565( info ); }
    
    /** Not used. */
    static void* getNewDecompressor( const EqCompressorInfo* info ){ return 0; }
    
    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha );

    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
};

}
}
#endif // EQ_PLUGIN_COMPRESSOR_DIFF_RLE_565
