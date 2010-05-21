
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "compressorDataCPU.h"
#include "compressor.h"
#include "global.h"
#include "pluginRegistry.h"

#include <typeinfo>

namespace eq
{
namespace base
{

bool CompressorDataCPU::isValid()
{
    return ( _name != EQ_COMPRESSOR_NONE && _plugin && 
           ( !_isCompressor || _instance ) );
}
void CompressorDataCPU::compress( void* const in, 
                                  const uint64_t pvpIn[4],
                                  const eq_uint64_t flags )
{
    _plugin->compress( _instance, _name, in, pvpIn, flags );
}

void CompressorDataCPU::compress( void* const in, const uint64_t inDims[2] )
{
    _plugin->compress( _instance, _name, in, inDims, EQ_COMPRESSOR_DATA_1D );
}

const unsigned CompressorDataCPU::getNumResults( ) const 
{
    return _plugin->getNumResults( _instance, _name );
}

void CompressorDataCPU::getResult( const unsigned i, 
                                   void** const out, 
                                   uint64_t* const outSize ) const
{
    return _plugin->getResult( _instance, _name, i, out, outSize ); 
}

void CompressorDataCPU::decompress( const void* const* in, 
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs,
                                    void* const out,
                                    uint64_t pvpOut[4],
                                    const uint64_t flags )
{ 
    _plugin->decompress( _instance, _name, in, inSizes,
                         numInputs, out, pvpOut, flags );
}

void CompressorDataCPU::decompress( const void* const* in, 
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs,
                                    void* const out,
                                    uint64_t outDim[2])
{
    _plugin->decompress( _instance, _name, in, inSizes,
                         numInputs, out, outDim, EQ_COMPRESSOR_DATA_1D );
}

void CompressorDataCPU::findAndInitCompressor( uint32_t dataType )
{
    initCompressor( chooseCompressor( dataType ) );
}

uint32_t CompressorDataCPU::chooseCompressor( const uint32_t tokenType, 
                                              const float minQuality,
                                              const bool ignoreMSE ) const
{
    uint32_t name = EQ_COMPRESSOR_NONE;
    float ratio = 1.0f;
    float minDiffQuality = 1.0f;

    base::PluginRegistry& registry = base::Global::getPluginRegistry();
    const base::Compressors& compressors = registry.getCompressors();
    for( base::Compressors::const_iterator i = compressors.begin();
         i != compressors.end(); ++i )
    {
        const base::Compressor* compressor = *i;
        const base::CompressorInfos& infos = compressor->getInfos();
        
        for( base::CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;
            if( info.tokenType != tokenType )
                continue;

            float infoRatio = info.ratio;
            if( ignoreMSE && ( info.capabilities & EQ_COMPRESSOR_IGNORE_MSE ))
            {
                switch( tokenType )
                {
                    default:
                        EQUNIMPLEMENTED; // no break;
                    case EQ_COMPRESSOR_DATATYPE_4_BYTE:
                    case EQ_COMPRESSOR_DATATYPE_4_HALF_FLOAT:
                    case EQ_COMPRESSOR_DATATYPE_4_FLOAT:
                        infoRatio *= .75f;
                        break;

                    case EQ_COMPRESSOR_DATATYPE_RGB10_A2:
                        infoRatio *= .9375f; // 30/32
                        break;
                }
            }
            
            const float diffQuality = info.quality - minQuality;
            if( ratio >= infoRatio && diffQuality <= minDiffQuality &&
                info.quality >= minQuality )
            {
                minDiffQuality = diffQuality;
                name = info.name;
                ratio = infoRatio;
            }
        }
    }

    return name;
}

}
}
