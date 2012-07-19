
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "compressorRLEB.h"
#include <limits>

namespace
{
static const uint8_t _rleMarker = 0x42; // just a random number
}

#include "compressorRLE.ipp"

namespace co
{
namespace plugin
{
namespace
{
REGISTER_ENGINE( CompressorRLEB, BYTE, BYTE, 1., 0.93, 1., false );
}

template< typename T >
inline void _compressChunk( const T* const in, const eq_uint64_t nPixels,
                            Compressor::Result* result )
{
    if( nPixels == 0 )
    {
        result->setSize( 0 );
        return;
    }

    T* tokenOut = reinterpret_cast< T* >( result->getData( )); 
    T tokenLast( in[0] );
    T tokenSame( 1 );
    T token(0);
    
    for( eq_uint64_t i = 1; i < nPixels; ++i )
    {
        token = in[i];
        COMPRESS( token );
    }

    WRITE_OUTPUT( token );
    result->setSize( (tokenOut - reinterpret_cast< T* >( result->getData( ))) *
                     sizeof( T ));
#ifndef CO_AGGRESSIVE_CACHING
    result->pack();
#endif
}

template< typename T > 
ssize_t _compress( const void* const inData, const eq_uint64_t nPixels,
                   Compressor::ResultVector& results )
{
    const eq_uint64_t size = nPixels * sizeof( T );
    const ssize_t nChunks = _setupResults( 1, size, results );
    const float width = static_cast< float >( nPixels ) /  
                        static_cast< float >( nChunks );

    const T* const data = reinterpret_cast< const T* >( inData );
    
#ifdef CO_USE_OPENMP
#pragma omp parallel for
#endif
    for( ssize_t i = 0; i < static_cast< ssize_t >( nChunks ) ; ++i )
    {
        const eq_uint64_t startIndex = static_cast< eq_uint64_t >( i * width );
        
        eq_uint64_t nextIndex;
        if ( i == nChunks - 1 )
            nextIndex = nPixels;
        else
            nextIndex = static_cast< eq_uint64_t >(( i + 1 ) * width );
        const eq_uint64_t chunkSize = ( nextIndex - startIndex );

        _compressChunk< T >( &data[ startIndex ], chunkSize, results[i] );
    }
    return nChunks;
}


void CompressorRLEB::compress( const void* const inData, 
                               const eq_uint64_t nPixels, const bool useAlpha )
{
    if( (nPixels & 0x7) == 0 )
        _nResults = _compress< eq_uint64_t >( inData, nPixels>>3, _results );
    else if( (nPixels & 0x3) == 0 )
        _nResults = _compress< uint32_t >( inData, nPixels>>2, _results );
    else if( (nPixels & 0x1) == 0 )
        _nResults = _compress< uint16_t >( inData, nPixels>>1, _results );
    else
        _nResults = _compress< uint8_t >( inData, nPixels, _results );
}

//----------------------------------------------------------------------
template< typename T >
inline void _decompressChunk( const T* in, T* out, const eq_uint64_t nPixels )
{
    T token(0);
    T tokenLeft(0);
   
    for( eq_uint64_t i = 0; i < nPixels ; ++i )
    {
        if( tokenLeft == 0 )
        {
            token = *in; ++in;
            if( token == _rleMarker )
            {
                token     = *in; ++in;
                tokenLeft = *in; ++in;
            }
            else // single symbol
                tokenLeft = 1;
        }

        --tokenLeft;
        out[i] = token;
    }
}


template< typename T >
void _decompress( const void* const* inData, const unsigned nInputs,
                  void* const outData, const eq_uint64_t nPixels )
{
    const float width = static_cast< float >( nPixels ) /  
                        static_cast< float >( nInputs );

    const T* const* in = reinterpret_cast< const T* const* >( inData );

#ifdef CO_USE_OPENMP
#pragma omp parallel for
#endif
    for( ssize_t i = 0; i < static_cast< ssize_t >( nInputs ) ; ++i )
    {
        const eq_uint64_t startIndex = static_cast<uint64_t>( i * width );
        
        eq_uint64_t nextIndex;
        if ( i == static_cast<ssize_t>( nInputs -1 ) )
            nextIndex = nPixels;
        else
            nextIndex = static_cast< eq_uint64_t >(( i + 1 ) * width );
        
        const eq_uint64_t chunkSize = ( nextIndex - startIndex );
        T* out = reinterpret_cast< T* >( outData ) + startIndex;

        _decompressChunk< T >( in[i], out, chunkSize );
    }
}

void CompressorRLEB::decompress( const void* const* inData, 
                                 const eq_uint64_t* const inSizes, 
                                 const unsigned nInputs,
                                 void* const outData, 
                                 const eq_uint64_t nPixels,
                                 const bool useAlpha )
{
    if( (nPixels & 0x7) == 0 )
        _decompress< uint64_t >( inData, nInputs, outData, nPixels>>3 );
    else if( (nPixels & 0x3) == 0 )
        _decompress< uint32_t >( inData, nInputs, outData, nPixels>>2 );
    else if( (nPixels & 0x1) == 0 )
        _decompress< uint16_t >( inData, nInputs, outData, nPixels>>1 );
    else
        _decompress< uint8_t >( inData, nInputs, outData, nPixels );
}

}
}
