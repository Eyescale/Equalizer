
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
 
#include "compressorRLEByte.h"


//#define EQ_USE_OPENMP
namespace eq
{
namespace plugin
{
const uint64_t _rleMarker = 0x42; // just a random number

void CompressorRLEByte::compress( void* const inData, 
                                const uint64_t inSize, 
                                const bool useAlpha )
{
    _setupResults( inSize );

    const uint32_t numResult = _results.size();

    const float width = static_cast< float >( inSize ) /  
        static_cast< float >( numResult );
    
    uint8_t* const data = (uint8_t* const )inData;

    #pragma omp parallel for
    for ( size_t i = 0; i < numResult; i += 4 )
    {
        const uint32_t startIndex = 
            static_cast< uint32_t >(i/_numChannels * width) * 
                         _numChannels;

        const uint32_t nextIndex  =
            static_cast< uint32_t >((i/_numChannels + 1) * width) * 
                         _numChannels;
        
        const uint64_t inSizeParallel = (nextIndex - startIndex) / 
                          _numChannels;
        
        _writeHeader( &_results[i], Header( inSizeParallel, useAlpha ) );

        _compress( &data[ startIndex ], inSizeParallel, 
                            &_results[i] );

    }

}

void CompressorRLEByte::_compress( const uint8_t* input, 
                                   const uint64_t size, 
                                   Result** results)
{    
    uint8_t* outOne( results[ 0 ]->data + 4 /* nWords 'header' */ ); 

    uint8_t lastOne( input[0] );
    uint8_t sameOne( 1 );
    const uint8_t* data   = reinterpret_cast< const uint8_t* >( input );
    uint8_t one;
    for( uint32_t i = 1; i < size; ++i )
    {    

        one = data[i];
        if( one == lastOne && sameOne != 255 )
            ++sameOne;
        else
        {
            WRITE_OUTPUT( One );
            lastOne = one;
            sameOne = 1;
        }
    }

    WRITE_OUTPUT( One );
    
    _results[0]->size = outOne - _results[0]->data;
}


void CompressorRLEByte::decompress( const void** const inData, 
                                    const uint64_t* const inSizes,
                                    void* const outData, 
                                    const uint64_t* const outSize )
{

    const uint8_t** inData8 = reinterpret_cast< const uint8_t** >( inData );
    uint8_t*       out   = reinterpret_cast< uint8_t* >( outData );

    // decompress 
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
    uint64_t numBlock = inSizes[0];
    for( uint64_t i = 0; i < numBlock ; i++ )
    {
        const uint8_t* oneIn;
        
        oneIn  = inData8[ i*4 ] + 4;

        uint8_t one(0);
        uint8_t oneLeft(0);

        const uint32_t* u32In = 
                        reinterpret_cast< const uint32_t* >( inData8[i] );
        const uint32_t  blockSize = u32In[0];

        for( uint32_t j = 0; j < blockSize; ++j )
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
    return;
}

}
}
