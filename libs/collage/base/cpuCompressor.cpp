
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "cpuCompressor.h"

#include "compressorInfo.h"
#include "debug.h"
#include "global.h"
#include "plugin.h"
#include "pluginRegistry.h"

namespace co
{
namespace base
{

void CPUCompressor::compress( void* const in, const uint64_t pvpIn[4],
                              const eq_uint64_t flags )
{
    EQASSERT( _plugin );
    EQASSERT( _instance );
    _plugin->compress( _instance, _name, in, pvpIn, flags );
}

void CPUCompressor::compress( void* const in, const uint64_t inDims[2] )
{
    EQASSERT( _plugin );
    EQASSERT( _instance );
    _plugin->compress( _instance, _name, in, inDims, EQ_COMPRESSOR_DATA_1D );
}

unsigned CPUCompressor::getNumResults( ) const 
{
    EQASSERT( _plugin );
    EQASSERT( _instance );
    return _plugin->getNumResults( _instance, _name );
}

void CPUCompressor::getResult( const unsigned i, 
                                   void** const out, 
                                   uint64_t* const outSize ) const
{
    EQASSERT( _plugin );
    EQASSERT( _instance );
    return _plugin->getResult( _instance, _name, i, out, outSize ); 
}

void CPUCompressor::decompress( const void* const* in, 
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs,
                                    void* const out,
                                    uint64_t pvpOut[4],
                                    const uint64_t flags )
{ 
    _plugin->decompress( _instance, _name, in, inSizes,
                         numInputs, out, pvpOut, flags );
}

void CPUCompressor::decompress( const void* const* in, 
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs,
                                    void* const out,
                                    uint64_t outDim[2])
{
    _plugin->decompress( _instance, _name, in, inSizes,
                         numInputs, out, outDim, EQ_COMPRESSOR_DATA_1D );
}

bool CPUCompressor::initCompressor( const uint32_t dataType,
                                    const float quality, const bool noAlpha )
{
    return Compressor::initCompressor(
        chooseCompressor( dataType, quality, noAlpha ));
}

uint32_t CPUCompressor::chooseCompressor( const uint32_t tokenType, 
                                          const float minQuality,
                                          const bool ignoreALPHA )
{
    uint32_t name = EQ_COMPRESSOR_NONE;
    float ratio = 1.0f;
    float minDiffQuality = 1.0f;

    PluginRegistry& registry = Global::getPluginRegistry();
    const Plugins& plugins = registry.getPlugins();
    for( Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i )
    {
        const Plugin* plugin = *i;
        const CompressorInfos& infos = plugin->getInfos();
        
        for( CompressorInfos::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const CompressorInfo& info = *j;
            if( info.tokenType != tokenType )
                continue;

            if( info.capabilities & EQ_COMPRESSOR_TRANSFER )
                continue;

            float infoRatio = info.ratio;
            if( ignoreALPHA && ( info.capabilities&EQ_COMPRESSOR_IGNORE_ALPHA ))
            {
                switch( tokenType )
                {
                    default:
                        break;
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
