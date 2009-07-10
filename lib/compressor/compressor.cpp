
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
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
namespace
{
    Compressor::Functions _functions[] =
    {   
        eq::plugin::CompressorRLE4B::getFunctions(),
        eq::plugin::CompressorDiffRLE4B::getFunctions(),
#if 0
        eq::plugin::CompressorRLEByte::getFunctions(),
        eq::plugin::CompressorRLE3B::getFunctions(),
        eq::plugin::CompressorRLE4F::getFunctions(),
        eq::plugin::CompressorRLE4HF::getFunctions(),
#endif
        Compressor::Functions()
    };

    Compressor::Functions& _findFunctions( const unsigned name )
    {
        for( size_t i = 0; true; ++i )
        {
            if( _functions[i].name == name )
                return _functions[i];

            if( _functions[i].name == 0 )
            {
                // EQASSERT( 0 );
                return _functions[i];
            }
        }

        // UNREACHABLE
        return _functions[0];
    }
}

Compressor::Compressor()
{}

Compressor::~Compressor()
{
    for ( size_t i = 0; i < _results.size(); i++ )
        delete ( _results[i] );

    _results.clear();
}

Compressor::Functions::Functions()
        : name( 0 )
        , getInfo( 0 )
        , newCompressor( 0 )
        , decompress( 0 )
{}
    
}
}

size_t EqCompressorGetNumCompressors()
{
    return sizeof( eq::plugin::_functions ) /
           sizeof( eq::plugin::Compressor::Functions ) - 1;
}
           
void EqCompressorGetInfo( const size_t n, 
                                        EqCompressorInfo* const info )
{
    return eq::plugin::_functions[ n ].getInfo( info );
}

void* EqCompressorNewCompressor( const unsigned name )
{
    const eq::plugin::Compressor::Functions& functions = 
        eq::plugin::_findFunctions( name );
    return functions.newCompressor( );
}

void EqCompressorDeleteCompressor( void* const compressor )
{
    delete reinterpret_cast< eq::plugin::Compressor* >( compressor );
}

void* EqCompressorNewDecompressor( const unsigned name ) 
{
    return 0;
}

void EqCompressorDeleteDecompressor( void* const decompressor ) 
{
    assert( decompressor == 0 );
    /* nop */
}

void EqCompressorCompress( void* const compressorPtr,
                                         const unsigned name,
                                         void* const in, 
                                         const eq_uint64_t* inDims,
                                         const eq_uint64_t flags )
{
    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_MSE);
    const eq_uint64_t nPixels = (flags & EQ_COMPRESSOR_DATA_1D) ?
                                  inDims[1]: inDims[1] * inDims[3];

    eq::plugin::Compressor* compressor = 
        reinterpret_cast< eq::plugin::Compressor* >( compressorPtr );
    compressor->compress( in, nPixels, useAlpha );
}

unsigned EqCompressorGetNumResults( void* const compressor,
                                                  const unsigned name )
{
    return (( eq::plugin::Compressor* )( compressor ))->getResults().size();
}

void EqCompressorGetResult( void* const compressor, 
                                          const unsigned name,
                                          const unsigned i, 
                                          void** const out, 
                                          eq_uint64_t* const outSize )
{
    const eq::plugin::Compressor::Result* result = 
        (( eq::plugin::Compressor* )( compressor ))->getResults()[ i ];
    
    *out = (void*)(result->data);
    *outSize = result->size;
    assert( result->getMaxSize() >= result->size );
}


void EqCompressorDecompress( void* const decompressor, 
                                           const unsigned name,
                                           const void* const* in, 
                                           const eq_uint64_t* const inSizes,
                                           const unsigned numInputs,
                                           void* const out,
                                           eq_uint64_t* const outDims,
                                           const eq_uint64_t flags )
{
    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_MSE);
    const eq_uint64_t nPixels = ( flags & EQ_COMPRESSOR_DATA_1D) ?
                           outDims[1] : outDims[1] * outDims[3];

    eq::plugin::Compressor::Functions& functions = 
        eq::plugin::_findFunctions( name );
    functions.decompress( in, inSizes, numInputs, out, nPixels, useAlpha );
}
