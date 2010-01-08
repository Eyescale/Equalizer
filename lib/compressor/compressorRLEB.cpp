
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

static inline void _compress( const void* const input, const uint64_t nPixels,
                              eq::plugin::Compressor::Result** results )
{
    if( nPixels == 0 )
    {
        results[0]->setSize( 0 );
        return;
    }

    const uint8_t* pixel = reinterpret_cast< const uint8_t* >( input );
    uint8_t* oneOut( reinterpret_cast< uint8_t* >( 
                                 results[ 0 ]->getData( ))); 
    
    uint8_t oneLast(pixel[0]);
    uint8_t oneSame( 1 );
    uint8_t one(0);
    
    for( uint64_t i = 1; i < nPixels; ++i )
    {
        one = pixel[i];
        if( one == oneLast && oneSame != 255 )
            ++oneSame;
        else
        {
            WRITE_OUTPUT( one );
            oneLast = one;
            oneSame = 1;
        }
    }

    WRITE_OUTPUT( one );

    results[0]->setSize( reinterpret_cast< uint8_t* > ( oneOut )  -
                         results[0]->getData( ));
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

        _compress( &data[ startIndex ], chunkSize, &_results[i] );
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
    for( ssize_t i = 0; i < static_cast< ssize_t >( nInputs ) ; i++ )
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

        const uint8_t* oneIn   = in[ i + 0 ];
        
        uint8_t one(0);
        uint8_t oneLeft(0);
   
        for( uint64_t j = 0; j < chunkSize ; ++j )
        {

           if( oneLeft == 0 )
           {
               one = *oneIn; ++oneIn;
               if( one == _rleMarker )
               {
                   one     = *oneIn; ++oneIn;
                   oneLeft = *oneIn; ++oneIn;
               }
               else // single symbol
                        oneLeft = 1;
           }
           --oneLeft;

           
           out[j] = one;
        }

    }
}

}
}
