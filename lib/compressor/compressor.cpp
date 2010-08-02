
/* Copyright (c) 2009-2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "compressor.h"

#include "compressorRLE4B.h"
#include "compressorRLEB.h"
#include "compressorRLE4HF.h"
#include "compressorRLE4BU.h"
#include "compressorRLE565.h"
#include "compressorRLE10A2.h"
#include "compressorYUV.h"
#include "compressorReadDrawPixels.h"
#include "compressorRLEYUV.h"
namespace eq
{
namespace plugin
{
namespace
{
    Compressor::Functions _functions[] =
    {   
        eq::plugin::CompressorRLE4B::getFunctions( 0 ),
        eq::plugin::CompressorRLE4B::getFunctions( 1 ),
        eq::plugin::CompressorRLE4B::getFunctions( 2 ),
        eq::plugin::CompressorRLE4B::getFunctions( 3 ),
        eq::plugin::CompressorRLE4B::getFunctions( 4 ),
        eq::plugin::CompressorRLE4B::getFunctions( 5 ),
        eq::plugin::CompressorRLE4B::getFunctions( 6 ),
        eq::plugin::CompressorDiffRLE4B::getFunctions( 0 ),
        eq::plugin::CompressorDiffRLE4B::getFunctions( 1 ),
        eq::plugin::CompressorDiffRLE4B::getFunctions( 2 ),
        eq::plugin::CompressorDiffRLE4B::getFunctions( 3 ),
        eq::plugin::CompressorRLE4HF::getFunctions( 0 ),
        eq::plugin::CompressorRLE4HF::getFunctions( 1 ),
        eq::plugin::CompressorDiffRLE4HF::getFunctions( 0 ),
        eq::plugin::CompressorDiffRLE4HF::getFunctions( 1 ),
        eq::plugin::CompressorRLE4BU::getFunctions(),
        eq::plugin::CompressorDiffRLE565::getFunctions( 0 ),
        eq::plugin::CompressorDiffRLE565::getFunctions( 1 ),
        eq::plugin::CompressorDiffRLE565::getFunctions( 2 ),
        eq::plugin::CompressorDiffRLE565::getFunctions( 3 ),
        eq::plugin::CompressorRLEB::getFunctions( ),
        eq::plugin::CompressorDiffRLE10A2::getFunctions(),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 0 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 1 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 2 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 3 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 4 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 5 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 6 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 7 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 8 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 9 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 10 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 11 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 12 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 13 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 14 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 15 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 16 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 17 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 18 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 19 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 20 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 21 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 22 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 23 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 24 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 25 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 26 ),
        eq::plugin::CompressorReadDrawPixels::getFunctions( 27 ),
        eq::plugin::CompressorYUV::getFunctions(),
        eq::plugin::CompressorDiffRLEYUV::getFunctions(),

#if 0
        eq::plugin::CompressorRLE3B::getFunctions(),
        eq::plugin::CompressorRLE4F::getFunctions(),
#endif
        Compressor::Functions()
    };

    Compressor::Functions& _findFunctions( const unsigned name )
    {
        for( size_t i = 0; true; ++i )
        {
            EqCompressorInfo info;
            _functions[i].getInfo( &info );
            if( info.name == name )
                return _functions[i];

            if( info.name == 0 )
            {
                assert( 0 ); // UNREACHABLE
                return _functions[i];
            }
        }

        assert( 0 ); // UNREACHABLE
        return _functions[0];
    }
}

Compressor::Compressor( const EqCompressorInfo* info )
{}

Compressor::~Compressor()
{
    for ( size_t i = 0; i < _results.size(); i++ )
        delete ( _results[i] );

    _results.clear();
}

Compressor::Functions::Functions()
        : getInfo( 0 )
        , newCompressor( 0 )
        , newDecompressor( 0 )
        , decompress( 0 )
        , isCompatible( 0 )
{}
    
}
}

size_t EqCompressorGetNumCompressors()
{
    return sizeof( eq::plugin::_functions ) /
           sizeof( eq::plugin::Compressor::Functions ) - 1;
}
           
void EqCompressorGetInfo( const size_t n, EqCompressorInfo* const info )
{
    eq::plugin::_functions[ n ].getInfo( info );
}

void* EqCompressorNewCompressor( const unsigned name )
{
    const eq::plugin::Compressor::Functions& functions = 
        eq::plugin::_findFunctions( name );
    
    EqCompressorInfo info;
    functions.getInfo( &info );
    return functions.newCompressor( &info );
}

void EqCompressorDeleteCompressor( void* const compressor )
{
    delete reinterpret_cast< eq::plugin::Compressor* >( compressor );
}

void* EqCompressorNewDecompressor( const unsigned name ) 
{
    const eq::plugin::Compressor::Functions& functions = 
        eq::plugin::_findFunctions( name );
    
    EqCompressorInfo info;
    functions.getInfo( &info );
    return functions.newDecompressor( &info );
}

void EqCompressorDeleteDecompressor( void* const decompressor ) 
{
    delete reinterpret_cast< eq::plugin::Compressor* >( decompressor );
}

void EqCompressorCompress( void* const ptr, const unsigned name,
                           void* const in, const eq_uint64_t* inDims,
                           const eq_uint64_t flags )
{
    assert( ptr );
    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_ALPHA);
    const eq_uint64_t nPixels = (flags & EQ_COMPRESSOR_DATA_1D) ?
                                  inDims[1]: inDims[1] * inDims[3];

    eq::plugin::Compressor* compressor = 
        reinterpret_cast< eq::plugin::Compressor* >( ptr );
    compressor->compress( in, nPixels, useAlpha );
}

unsigned EqCompressorGetNumResults( void* const ptr,
                                    const unsigned name )
{
    assert( ptr );
    eq::plugin::Compressor* compressor = 
        reinterpret_cast< eq::plugin::Compressor* >( ptr );
    return compressor->getNResults();
}

void EqCompressorGetResult( void* const ptr, const unsigned name,
                            const unsigned i, void** const out, 
                            eq_uint64_t* const outSize )
{
    assert( ptr );
    eq::plugin::Compressor* compressor = 
        reinterpret_cast< eq::plugin::Compressor* >( ptr );
    eq::plugin::Compressor::Result* result = compressor->getResults()[ i ];
    
    *out = result->getData();
    *outSize = result->getSize();
    assert( result->getMaxSize() >= result->getSize( ));
}


void EqCompressorDecompress( void* const decompressor, const unsigned name,
                             const void* const* in,
                             const eq_uint64_t* const inSizes,
                             const unsigned nInputs,
                             void* const out, eq_uint64_t* const outDims,
                             const eq_uint64_t flags )
{
    assert( !decompressor );
    const bool useAlpha = !(flags & EQ_COMPRESSOR_IGNORE_ALPHA);
    const eq_uint64_t nPixels = ( flags & EQ_COMPRESSOR_DATA_1D) ?
                           outDims[1] : outDims[1] * outDims[3];

    eq::plugin::Compressor::Functions& functions = 
        eq::plugin::_findFunctions( name );
    functions.decompress( in, inSizes, nInputs, out, nPixels, useAlpha );
}

bool EqCompressorIsCompatible( const unsigned     name,
                               const GLEWContext* glewContext )
{
    eq::plugin::Compressor::Functions& functions = 
        eq::plugin::_findFunctions( name );
    
    if ( functions.isCompatible == 0 )
    {
        assert( false );
        return false;
    }

    return functions.isCompatible( glewContext );
}

void EqCompressorDownload( void* const        ptr,
                           const unsigned     name,
                           const GLEWContext* glewContext,
                           const eq_uint64_t  inDims[4],
                           const unsigned     source,
                           const eq_uint64_t  flags,
                           eq_uint64_t        outDims[4],
                           void**             out )
{
    assert( ptr );
    eq::plugin::Compressor* compressor = 
        reinterpret_cast< eq::plugin::Compressor* >( ptr );
    compressor->download( glewContext, inDims, source, flags, outDims, out );
}


void EqCompressorUpload( void* const        ptr,
                         const unsigned     name,
                         const GLEWContext* glewContext, 
                         const void*        buffer,
                         const eq_uint64_t  inDims[4],
                         const eq_uint64_t  flags,
                         const eq_uint64_t  outDims[4],  
                         const unsigned     destination )
{
    assert( ptr );
    eq::plugin::Compressor* compressor = 
        reinterpret_cast< eq::plugin::Compressor* >( ptr );
    compressor->upload( glewContext, buffer, inDims, flags, outDims,
                        destination );
}

