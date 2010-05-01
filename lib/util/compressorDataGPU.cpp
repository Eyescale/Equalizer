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

#include "compressorDataGPU.h"
#include "eq/base/compressor.h"
#include "eq/base/global.h"
#include "eq/base/pluginRegistry.h"

namespace eq
{
namespace util
{

bool CompressorDataGPU::isValidDownloader( uint32_t name,
                                           uint32_t inputToken, 
                                           uint32_t outputToken,
                                           float    minQuality )
{
    return ( _info.quality >= minQuality ) && isValid( name ) && 
            isValidUploader( outputToken, inputToken  );
}

bool CompressorDataGPU::isValidUploader( uint32_t inputToken, 
                                      uint32_t outputToken )
{
    return ( _info.outputTokenType == inputToken ) &&
           ( _info.tokenType == outputToken );
}

void CompressorDataGPU::download( const fabric::PixelViewport& pvpIn,
                                  const unsigned     source,
                                  const eq_uint64_t  flags,
                                  fabric::PixelViewport&       pvpOut,
                                  void**             out )
{
    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
    uint64_t outDims[4] = { 0, 0, 0, 0 };
    _plugin->download( _instance, _name, _glewContext,
                       inDims, source, flags, outDims, out );
    pvpOut.x = outDims[0];
    pvpOut.w = outDims[1];
    pvpOut.y = outDims[2];
    pvpOut.h = outDims[3];
}

void CompressorDataGPU::upload( const void*          buffer,
                                const fabric::PixelViewport& pvpIn,
                                const eq_uint64_t    flags,
                                const fabric::PixelViewport& pvpOut,  
                                const unsigned       destination )
{
    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
    uint64_t outDims[4] = { pvpOut.x, pvpOut.w, pvpOut.y, pvpOut.h };
    _plugin->upload( _instance, _name, _glewContext, buffer, inDims,
                     flags, outDims, destination );
}

void CompressorDataGPU::initUploader( uint32_t gpuTokenType, 
                                      uint32_t tokenType )
{
    base::PluginRegistry& registry = base::Global::getPluginRegistry();
    const base::CompressorVector& compressors = registry.getCompressors();

    uint32_t name = EQ_COMPRESSOR_NONE;
    float speed = 0.0f;
    for( base::CompressorVector::const_iterator i = compressors.begin();
         i != compressors.end(); ++i )
    {
        const base::Compressor* compressor = *i;
        const base::CompressorInfoVector& infos = compressor->getInfos();

        EQINFO << "Searching in DSO " << (void*)compressor << std::endl;
        
        for( base::CompressorInfoVector::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;
            
            if(( info.capabilities & EQ_COMPRESSOR_TRANSFER ) == 0 )
                continue;

            if( info.outputTokenType != gpuTokenType )
                continue;

            if( info.tokenType != tokenType )
                continue;
            
            if( !compressor->isCompatible( info.name, _glewContext ))
                continue;
            
            if ( speed < info.speed )
            {
                speed = info.speed;
                name = info.name;
                _info = info;
            }
        }
    }

    if ( name == EQ_COMPRESSOR_NONE )
        _reset();
    else if( name != _name )
        _initCompressor( name );    
}

void CompressorDataGPU::initDownloader( float minQuality, 
                                        uint32_t tokenType )
{ 
    base::PluginRegistry& registry = base::Global::getPluginRegistry();
    const base::CompressorVector& compressors = registry.getCompressors();

    float factor = 1.1f;
    uint32_t name = EQ_COMPRESSOR_NONE;

    for( base::CompressorVector::const_iterator i = compressors.begin();
         i != compressors.end(); ++i )
    {
        const base::Compressor* compressor = *i;
        const base::CompressorInfoVector& infos = compressor->getInfos();

        EQINFO << "Searching in DSO " << (void*)compressor << std::endl;
        
        for( base::CompressorInfoVector::const_iterator j = infos.begin();
             j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;

            if(( info.capabilities & EQ_COMPRESSOR_TRANSFER ) == 0 )
                continue;
            
            if( info.tokenType != tokenType )
                continue;

            if( info.quality < minQuality )
                continue;

            if( !compressor->isCompatible( info.name, _glewContext ))
                continue;
            
            if ( factor > ( info.ratio * info.quality ))
            {
                factor = ( info.ratio * info.quality );
                name = info.name;
                _info = info;
            }
        }
    }

    if ( name == EQ_COMPRESSOR_NONE )
        _reset();
    else if( name != _name )
    {
        _initCompressor( name );
    }
}


}
}
