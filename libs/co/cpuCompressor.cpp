
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010-2012, Stefan Eilemann <eile@eyescale.ch>
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
#include "global.h"
#include "plugin.h"
#include "pluginRegistry.h"

namespace co
{

void CPUCompressor::compress( void* const in, const uint64_t pvpIn[4],
                              const eq_uint64_t flags )
{
    LBASSERT( _plugin );
    LBASSERT( _instance );
    _plugin->compress( _instance, _name, in, pvpIn, flags );
}

void CPUCompressor::compress( void* const in, const uint64_t inDims[2] )
{
    LBASSERT( _plugin );
    LBASSERT( _instance );
    _plugin->compress( _instance, _name, in, inDims, EQ_COMPRESSOR_DATA_1D );
}

unsigned CPUCompressor::getNumResults( ) const 
{
    LBASSERT( _plugin );
    LBASSERT( _instance );
    return _plugin->getNumResults( _instance, _name );
}

void CPUCompressor::getResult( const unsigned i, 
                                   void** const out, 
                                   uint64_t* const outSize ) const
{
    LBASSERT( _plugin );
    LBASSERT( _instance );
    return _plugin->getResult( _instance, _name, i, out, outSize ); 
}

void CPUCompressor::decompress( const void* const* in,
                                const uint64_t* const inSizes,
                                const unsigned numInputs, void* const out,
                                uint64_t pvpOut[4], const uint64_t flags )
{ 
    _plugin->decompress( _instance, _name, in, inSizes, numInputs, out, pvpOut,
                         flags );
}

void CPUCompressor::decompress( const void* const* in, 
                                const uint64_t* const inSizes,
                                const unsigned numInputs, void* const out,
                                uint64_t outDim[2])
{
    _plugin->decompress( _instance, _name, in, inSizes, numInputs, out, outDim,
                         EQ_COMPRESSOR_DATA_1D );
}

bool CPUCompressor::initCompressor( const uint32_t dataType,
                                    const float quality, const bool noAlpha )
{
    return Compressor::initCompressor( chooseCompressor( dataType, quality,
                                                         noAlpha ));
}

uint32_t CPUCompressor::chooseCompressor( const uint32_t tokenType, 
                                          const float minQuality,
                                          const bool ignoreAlpha )
{
    CompressorInfo candidate;
    candidate.name = EQ_COMPRESSOR_NONE;
    candidate.ratingAlpha = 1.0f;
    candidate.quality = 1.0f;
    candidate.speed = 1.0f;

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
            if( info.tokenType != tokenType || info.quality < minQuality ||
                ( info.capabilities & EQ_COMPRESSOR_TRANSFER ))
            {
                continue;
            }

            const float rating = ignoreAlpha ? info.ratingNoAlpha :
                                               info.ratingAlpha;

            if( rating > candidate.ratingAlpha )
            {
                candidate = info;
                candidate.ratingAlpha = rating;
            }
        }
    }

    return candidate.name;
}

}
