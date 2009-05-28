
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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
 
#include "compressor.h"
#include "compressorRLEByte.h" 
#include "compressorRLE3B.h" 
#include "compressorRLE4B.h"
#include "compressorRLE4F.h"
#include "compressorRLE4HF.h"

namespace eq
{
namespace plugin
{
    Functions _functions[] =
    {   
        eq::plugin::CompressorRLE4B::getFunctions(),
        eq::plugin::CompressorDiffRLE4B::getFunctions(),
        eq::plugin::CompressorRLEByte::getFunctions(),
        eq::plugin::CompressorRLE3B::getFunctions(),
        eq::plugin::CompressorDiffRLE3B::getFunctions(),
        eq::plugin::CompressorRLE4F::getFunctions(),
        eq::plugin::CompressorRLE4HF::getFunctions()
    };
    
    Functions::Functions()
    {
        getInfo = 0;
        newCompressor = 0;  
    }
    Functions _findFunctions( const unsigned name )
    {
        const size_t size = EqCompressorGetNumCompressors();
        for( size_t i = 0 ; i < size; i++ )
        {
            EqCompressorInfo info;
            EqCompressorGetInfo( i, &info );
            if ( info.type == name )
                return _functions[i];
        }

        EQASSERT( 0 );
        return Functions();
    }
}
}


EQ_PLUGIN_API size_t EqCompressorGetNumCompressors()
{
    return sizeof( eq::plugin::_functions ) / sizeof( eq::plugin::Functions );
}
           
EQ_PLUGIN_API void EqCompressorGetInfo( const size_t n, 
                                        EqCompressorInfo* const info )
{
    return eq::plugin::_functions[ n ].getInfo( info );
}

EQ_PLUGIN_API void* EqCompressorNewCompressor( const unsigned name )
{
    const eq::plugin::Functions& functions = eq::plugin::_findFunctions( name );
    return functions.newCompressor( );
}

EQ_PLUGIN_API void EqCompressorDeleteCompressor( void* const compressor )
{
    delete reinterpret_cast< eq::plugin::Compressor* >( compressor );
}

EQ_PLUGIN_API void EqCompressorGetResult( void* const compressor, 
                                          const unsigned i, 
                                          void** const out, 
                                          eq_uint64_t* const outSize )
{
    const eq::plugin::Result* result = 
        (( eq::plugin::Compressor* )( compressor ))->getResults()[ i ];
    
    *out = (void*)(result->data);
    *outSize = result->size;
}

EQ_PLUGIN_API void* EqCompressorNewDecompressor( const unsigned name ) 
                { return 0; }
EQ_PLUGIN_API void EqCompressorDeleteDecompressor( void* const decompressor ) 
                { /* nop */ }

EQ_PLUGIN_API void EqCompressorCompress( void* const compressor,
                                         void* const in, 
                                         const uint64_t* inDims,
                                         const uint64_t flags )
{
    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_MSE);
    const uint64_t inSize =  (flags & EQ_COMPRESSOR_DATA_1D) ?
                             inDims[1]: inDims[1] * inDims[3];

    reinterpret_cast< eq::plugin::Compressor* >
        ( compressor )->compress( in, inSize, useAlpha );
}

EQ_PLUGIN_API unsigned EqCompressorGetNumResults( 
                                      void* const compressor )
{
    return (( eq::plugin::Compressor* )( compressor ))->getResults().size();
}


EQ_PLUGIN_API void EqCompressorDecompress( void* const decompressor, 
                                           const void** const in, 
                                           const eq_uint64_t* const inSizes,
                                           const unsigned numInputs,
                                           void* const out,
                                           eq_uint64_t* const outDims,
                                           const eq_uint64_t flags )
{
    const uint64_t outSize = ( flags & EQ_COMPRESSOR_DATA_1D) ?
                           outDims[1] : outDims[1] * outDims[3];
    uint64_t numInput64 = numInputs;

    (( eq::plugin::Compressor* )( decompressor ))->decompress( 
        in, &numInput64, out, &outSize );

}

namespace eq
{
namespace plugin
{
    Compressor::Compressor( const uint32_t numChannels )
        : _numChannels( numChannels )
        , _swizzleData( false ) 
    { 
        _results.resize( numChannels );
        for (size_t i = 0; i < numChannels; i++)
            _results[i] = new Result;
    }
}
}
