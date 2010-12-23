
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
 
#ifndef CO_PLUGIN_COMPRESSOR_DIFF_RLE_565
#define CO_PLUGIN_COMPRESSOR_DIFF_RLE_565

#include "compressor.h"

namespace co
{
namespace plugin
{

class CompressorRLE565 : public Compressor
{
public:
    CompressorRLE565(): Compressor() {}
    virtual ~CompressorRLE565() {}

    /** Get a new instance of this compressor */
    static void* getNewCompressor( const unsigned name )
        { return new co::plugin::CompressorRLE565; }
    
    /** Not used. */
    static void* getNewDecompressor( const unsigned name ){ return 0; }
    
    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha );

    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
};

}
}
#endif // CO_PLUGIN_COMPRESSOR_DIFF_RLE_565
