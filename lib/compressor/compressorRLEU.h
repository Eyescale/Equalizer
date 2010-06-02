
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
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
 
#ifndef EQ_PLUGIN_COMPRESSORRLEU
#define EQ_PLUGIN_COMPRESSORRLEU

#include "compressorRLE4B.h"

namespace eq
{
namespace plugin
{

// Note: We reuse the 4-byte compressor since integer data can be interpreted as
// four bytes, and treating the bytes separately leads to better compression
// with single-component image data (typically depth buffer).

class CompressorRLEU : public CompressorRLE4B
{
public:
    CompressorRLEU() {}
    virtual ~CompressorRLEU() {}

    static void* getNewCompressor( ){ return new eq::plugin::CompressorRLEU; }
    
    static void getInfo( EqCompressorInfo* const info )
    {
        info->version      = EQ_COMPRESSOR_VERSION;
        info->name         = EQ_COMPRESSOR_RLE_UNSIGNED;
        info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D |
                             EQ_COMPRESSOR_IGNORE_MSE;
        info->tokenType    = EQ_COMPRESSOR_DATATYPE_UNSIGNED;

        info->quality = 1.0f;
        info->ratio   = .59f;
        info->speed   = 1.0f;
    }

    static Functions getFunctions()
    {
        Functions functions( CompressorRLE4B::getFunctions( ));
        functions.name               = EQ_COMPRESSOR_RLE_UNSIGNED;
        functions.getInfo            = getInfo;
        return functions;
    }
};

}
}
#endif // EQ_PLUGIN_COMPRESSORRLEU
