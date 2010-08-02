
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

class CompressorDiffRLE565 : public Compressor
{
public:
    CompressorDiffRLE565( const EqCompressorInfo* info ): Compressor( info ) {}
    virtual ~CompressorDiffRLE565() {}

    /** Get a new instance of this compressor */
    static void* getNewCompressor( const EqCompressorInfo* info )
        { return new eq::plugin::CompressorDiffRLE565( info ); }
    
    /** Not used. */
    static void* getNewDecompressor( const EqCompressorInfo* info ){ return 0; }
    
    /** @name getFunctions */
    /*@{*/
    /** @return the function pointer list for this compressor. */
    static Functions getFunctions( uint32_t index )
    {
        Functions functions;
        switch ( index )
        {
            default: assert( false );
            case 0: functions.getInfo = getInfo1; break;
            case 1: functions.getInfo = getInfo2; break;
            case 2: functions.getInfo = getInfo3; break;
            case 3: functions.getInfo = getInfo4; break;
        }
        functions.newCompressor = getNewCompressor;
        functions.newDecompressor = getNewDecompressor;
        functions.decompress = decompress;
        return functions;
    }

    virtual void compress( const void* const inData, const eq_uint64_t nPixels, 
                           const bool useAlpha );

    static void decompress( const void* const* inData, 
                            const eq_uint64_t* const inSizes, 
                            const unsigned nInputs, void* const outData, 
                            const eq_uint64_t nPixels, const bool useAlpha );
    private:
    /** @name getInfo */
    /*@{*/
    /**
     * get information about this compressor.
     *
     * @param info about this compressor.
     */
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version = EQ_COMPRESSOR_VERSION;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_ALPHA;
        info->quality = 0.7f;
        info->ratio = 0.06f;
        info->speed = 1.1f;
    }

    static void getInfo1( EqCompressorInfo* const info )
    {
        CompressorDiffRLE565::getInfo( info );
        info->name = EQ_COMPRESSOR_RLE_RGBA;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA;
    }

    static void getInfo2( EqCompressorInfo* const info )
    {
        CompressorDiffRLE565::getInfo( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_565_BGRA;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA;
    }
            
    static void getInfo3( EqCompressorInfo* const info )
    {
        CompressorDiffRLE565::getInfo( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_565_RGBA_UINT_8_8_8_8_REV;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_RGBA_UINT_8_8_8_8_REV;
    }
    
    static void getInfo4( EqCompressorInfo* const info )
    {
        CompressorDiffRLE565::getInfo( info );
        info->name = EQ_COMPRESSOR_DIFF_RLE_565_BGRA_UINT_8_8_8_8_REV;
        info->tokenType = EQ_COMPRESSOR_DATATYPE_BGRA_UINT_8_8_8_8_REV;
    }
};
}
}
#endif // EQ_PLUGIN_COMPRESSOR_DIFF_RLE_565
