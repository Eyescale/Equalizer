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
#include "eq/base/compressor.h"
#include "eq/base/global.h"
#include "eq/base/pluginRegistry.h"

namespace eq
{
namespace util
{

void CompressorDataCPU::compress( void* const in, 
                               const fabric::PixelViewport& pvpIn,
                               const eq_uint64_t flags )
{
    const uint64_t inDims[4] = { pvpIn.x, pvpIn.w, pvpIn.y, pvpIn.h }; 
    _plugin->compress( _instance, _name, in, inDims, flags );
}

unsigned CompressorDataCPU::getNumResults( )
{
    return _plugin->getNumResults( _instance, _name );
}

void CompressorDataCPU::getResult( const unsigned i, 
                                   void** const out, 
                                   uint64_t* const outSize )
{
    return _plugin->getResult( _instance, _name, i, out, outSize ); 
}

void CompressorDataCPU::decompress( const void* const* in, 
                                    const uint64_t* const inSizes,
                                    const unsigned numInputs,
                                    void* const out,
                                    fabric::PixelViewport& pvpOut,
                                    const uint64_t flags )
{ 
    uint64_t outDim[4] = { pvpOut.x, pvpOut.w, pvpOut.y, pvpOut.h };
    _plugin->decompress( _instance, _name, in, inSizes,
                         numInputs, out, outDim, flags );
}

}
}
