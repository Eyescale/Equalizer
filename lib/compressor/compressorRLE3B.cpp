
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
 
#include "compressorRLE3B.h"

namespace eq
{
namespace plugin
{
const uint64_t _rleMarker = 0x42; // just a random number


void CompressorRLE3B::compress( const void* const inData, 
                                const uint64_t inSize, 
                                const bool useAlpha )
{
     const uint64_t size = inSize * 3 ;
    _setupResults( size );

    const uint32_t numResult = _results.size();
   
    const float width = static_cast< float >( size ) /  
                        static_cast< float >( numResult );
    
    const uint8_t* const data = reinterpret_cast< const uint8_t* >(inData);
    
#pragma omp parallel for
    for ( size_t i = 0; i < numResult; i += 4 )
    {
        const uint32_t startIndex = 
            static_cast< uint32_t >( i / _numChannels * width ) * 
               _numChannels;
        const uint32_t nextIndex  =
            static_cast< uint32_t >(( i/_numChannels + 1 ) * width) * 
              _numChannels;
        const uint64_t inSizeParallel  = (nextIndex - startIndex) / 
            _numChannels;
        
        _writeHeader( &_results[i], Header( inSizeParallel, useAlpha ));
        _compress( &data[ startIndex ], inSizeParallel, &_results[i] );
    }

}

void CompressorRLE3B::_compress( const uint8_t* const input,
                                 const uint64_t size, Result** results )
{
    uint8_t* oneOut(   results[ 0 ]->data + sizeof( Header ) ); 
    uint8_t* twoOut(   results[ 1 ]->data + sizeof( Header ) ); 
    uint8_t* threeOut( results[ 2 ]->data + sizeof( Header ) ); 

    const uint8_t* data = reinterpret_cast<const uint8_t*>( input );

    uint8_t oneLast( data[0] ), twoLast( data[1] ), threeLast( data[2] );
    uint8_t oneSame( 1 ), twoSame( 1 ), threeSame( 1 );
    uint8_t one;
    uint8_t two;
    uint8_t three;
    data += 3;

    for( uint32_t i = 1; i < size; ++i )
    {        
        one = *data;
        ++data;
        WRITE( one );

        two = *data;
        ++data;
        WRITE( two );

        three = *data;
        ++data;
        WRITE( three );
    }

    WRITE_OUTPUT( one );
    WRITE_OUTPUT( two );
    WRITE_OUTPUT( three );

    results[0]->size = oneOut   - results[0]->data;
    results[1]->size = twoOut   - results[1]->data;
    results[2]->size = threeOut - results[2]->data;
}


void CompressorRLE3B::decompress( const void* const* inData, 
                                  const uint64_t* const inSizes,
                                  const unsigned numInputs,
                                  void* const outData, const uint64_t outSize,
                                  const bool useAlpha )
{

    const uint8_t* const* inData8 = reinterpret_cast< const uint8_t* const* >(
                                        inData );
    uint8_t*       out   = reinterpret_cast< uint8_t* >( outData );

    // decompress 
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
    assert( (numInputs%3) == 0 );
    const uint64_t numBlocks = numInputs / 3;
    for( uint64_t i = 0; i < numBlocks ; i++ )
    {
        const uint8_t* oneIn      = inData8[  i*3 + 0 ];
        const uint8_t* twoIn      = inData8[  i*3 + 1 ];  
        const uint8_t* threeIn    = inData8[  i*3 + 2 ];
                
        uint8_t one(0), two(0), three(0);
        uint8_t oneLeft(0), twoLeft(0), threeLeft(0);


        const uint64_t* u64In = 
            reinterpret_cast< const uint64_t* >( inData8[i] );
        const uint64_t  blockSize = u64In[0];

        oneIn++;
        
        for( uint32_t j = 0; j < blockSize; ++j )
        { 
           uint32_t result;
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


           if( twoLeft == 0 )
           {
               two = *twoIn; ++twoIn;
               if( two == _rleMarker )
               {
                   two     = *twoIn; ++twoIn;
                   twoLeft = *twoIn; ++twoIn;
               }
               else // single symbol
                   twoLeft = 1;
               
           }
           --twoLeft;

           if( threeLeft == 0 )
           {
               three = *threeIn; ++threeIn;
               if( three == _rleMarker )
               {
                   three     = *threeIn; ++threeIn;
                   threeLeft = *threeIn; ++threeIn;
               }
               else // single symbol
                   threeLeft = 1;

              
           }
           --threeLeft;    

           result = one + (two<<8) + (three<<16);
           
           uint8_t* result8 = reinterpret_cast< uint8_t* >( &result ); 
           out[j*3] = result8[0];
           out[j*3+1] = result8[0];
           out[j*3+2] = result8[0];
       }
    }
    return;
}

}
}
