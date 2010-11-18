
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

namespace
{
static const uint8_t _rleMarker = 0x42; // just a random number
}

#include "compressorRLE.ipp"

namespace eq
{
namespace plugin
{
namespace
{
REGISTER_ENGINE( CompressorRLEB, BYTE, BYTE, 1., 0.7, 1., false );
}

static inline void _compress( const uint8_t* const in, const uint64_t nPixels,
                              eq::plugin::Compressor::Result* result )
{
    if( nPixels == 0 )
    {
        result->setSize( 0 );
        return;
    }

    uint8_t* tokenOut( result->getData( )); 
    uint8_t tokenLast( in[0] );
    uint8_t tokenSame( 1 );
    uint8_t token(0);
    
    for( uint64_t i = 1; i < nPixels; ++i )
    {
        token = in[i];
        if( token == tokenLast && tokenSame != 255 )
            ++tokenSame;
        else
        {
            WRITE_OUTPUT( token );
            tokenLast = token;
            tokenSame = 1;
        }
    }

    WRITE_OUTPUT( token );

    result->setSize( tokenOut - result->getData( ));
}


static inline void _decompress( const uint8_t* in, uint8_t* out,
                                const uint64_t nPixels )
{
    uint8_t token(0);
    uint8_t tokenLeft(0);
   
    for( uint64_t i = 0; i < nPixels ; ++i )
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

void CompressorRLEB::compress( const void* const inData, 
                               const eq_uint64_t nPixels, const bool useAlpha,
                               const bool swizzle )
{
    const uint64_t size = nPixels;
    const ssize_t nChunks = _setupResults( 1, size, _results );

    const uint64_t nElems = nPixels;
    const float width = static_cast< float >( nElems ) /  
                        static_cast< float >( nChunks );

    const uint8_t* const data = 
        reinterpret_cast< const uint8_t* >( inData );
    
#ifdef EQ_USE_OPENMP
#pragma omp parallel for
#endif
    for( ssize_t i = 0; i < static_cast< ssize_t >( nChunks ) ; i++ )
    {
        const uint64_t startIndex = static_cast< uint64_t >( i * width );
        
        uint64_t nextIndex;
        if ( i == nChunks -1 )
            nextIndex = nPixels;
        else
            nextIndex = static_cast< uint64_t >(( i + 1 ) * width );
        const uint64_t chunkSize = ( nextIndex - startIndex );

        _compress( &data[ startIndex ], chunkSize, _results[i] );
    }
    _nResults = nChunks;
}

void CompressorRLEB::decompress( const void* const* inData, 
                                 const eq_uint64_t* const inSizes, 
                                 const unsigned nInputs,
                                 void* const outData, 
                                 const eq_uint64_t nPixels,
                                 const bool useAlpha )
{
    assert( (inSizes[0] % sizeof( uint8_t )) == 0 ); 
    assert( (inSizes[1] % sizeof( uint8_t )) == 0 ); 
    assert( (inSizes[2] % sizeof( uint8_t )) == 0 ); 

    const uint64_t nElems = nPixels;
    const float width = static_cast< float >( nElems ) /  
                        static_cast< float >( nInputs );

    const uint8_t* const* in = 
        reinterpret_cast< const uint8_t* const* >( inData );

#ifdef EQ_USE_OPENMP
#pragma omp parallel for
#endif
    for( ssize_t i = 0; i < static_cast< ssize_t >( nInputs ) ; ++i )
    {
        const uint64_t startIndex = static_cast<uint64_t>( i * width );
        
        uint64_t nextIndex;
        if ( i == static_cast<ssize_t>( nInputs -1 ) )
            nextIndex = nPixels;
        else
            nextIndex = static_cast< uint64_t >(( i + 1 ) * width );
        
        const uint64_t chunkSize = ( nextIndex - startIndex );
        uint8_t* out = reinterpret_cast< uint8_t* >( outData ) + 
                         startIndex;

        _decompress( in[i], out, chunkSize );
    }
}

}
}
