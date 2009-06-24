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
 
#include "compressorRLE4HF.h"

namespace eq
{
namespace plugin
{

// nan number
const uint16_t _rleMarker = 0xFFFF; 


void CompressorRLE4HF::compress( void* const inData, 
                                 const uint64_t inSize, 
                                 const bool useAlpha )
{
    const uint64_t size = inSize * 2 * 2 ;
    _setupResults( size );

    const uint32_t numResult = _results.size();

    const float width = static_cast< float >( size ) /  
                        static_cast< float >( numResult );

    uint16_t* const data = (uint16_t* const )inData;
     
    #pragma omp parallel for
    for ( size_t i = 0; i < numResult; i += 4 )
    {
        const uint32_t startIndex = 
            static_cast< uint32_t >( i/_numChannels * width ) * 
                         _numChannels;
        
        const uint32_t nextIndex  =
            static_cast< uint32_t >(( i/_numChannels + 1 ) * width ) * 
                         _numChannels;

        const uint64_t inSizeParallel = (nextIndex - startIndex) / 
                         _numChannels;
        
        _writeHeader( &_results[i], Header( inSizeParallel, useAlpha ) );

        _compress( &data[ startIndex ], inSizeParallel / 2, 
                            &_results[i],useAlpha );
    }

}

void CompressorRLE4HF::_compress( const uint16_t* input, 
                                  const uint64_t size, 
                                  Result** results, 
                                  const bool useAlpha )
{
    size_t sizeHeader  = sizeof( Header ) / 2 ;
    uint16_t* outOne  ( reinterpret_cast<uint16_t*>
                        ( results[ 0 ]->data) + sizeHeader ); 
    uint16_t* outTwo  ( reinterpret_cast<uint16_t*>
                        ( results[ 1 ]->data) + sizeHeader ); 
    uint16_t* outThree( reinterpret_cast<uint16_t*>
                        ( results[ 2 ]->data) + sizeHeader ); 
    uint16_t* outFour ( reinterpret_cast<uint16_t*>
                        ( results[ 3 ]->data) + sizeHeader ); 

    uint16_t lastOne  ( input[0] ), 
             lastTwo  ( input[1] ), 
             lastThree( input[2] ),
             lastFour ( input[3] );

    uint16_t sameOne  ( 1 ), 
             sameTwo  ( 1 ), 
             sameThree( 1 ), 
             sameFour ( 1 );
    
    // step four because it's in outOne, .... outFour.
    const uint16_t* data   = reinterpret_cast< const uint16_t* >( input ) +
                             _numChannels;
    uint16_t one;
    uint16_t two;
    uint16_t three;
    uint16_t four;
    
    for( uint32_t i = 0; i < size; ++i )
    {
        const uint16_t* word = reinterpret_cast< const uint16_t* >( data );
        
        one = word[0];
        if( one == lastOne && sameOne != 0xFFFF )
            ++sameOne;
        else
        {
            WRITE_OUTPUT( One );
            lastOne = one;
            sameOne = 1;
        }

     
        two = word[1];
        if( two == lastTwo && sameTwo != 0xFFFF )
            ++sameTwo;
        else
        {
            WRITE_OUTPUT( Two );
            lastTwo = two;
            sameTwo = 1;
        }


        three = word[2];
        if( three == lastThree && sameThree != 0xFFFF )
            ++sameThree;
        else
        {
            WRITE_OUTPUT( Three );
            lastThree = three;
            sameThree = 1;
        }


        if ( useAlpha ) 
        {
            four = word[3];
            if( four == lastFour && sameFour != 0xFFFF )
                ++sameFour;
            else
            {
                WRITE_OUTPUT( Four );
                lastFour = four;
                sameFour = 1;
            }
        }
        data +=4;
    }

    WRITE_OUTPUT( One );
    WRITE_OUTPUT( Two );
    WRITE_OUTPUT( Three );
    WRITE_OUTPUT( Four );
    
    _results[0]->size = reinterpret_cast< const uint8_t* >(outOne)   - 
                        _results[0]->data;
    _results[1]->size = reinterpret_cast< const uint8_t* >(outTwo)   - 
                        _results[1]->data;
    _results[2]->size = reinterpret_cast< const uint8_t* >(outThree) - 
                        _results[2]->data;
    _results[3]->size = reinterpret_cast< const uint8_t* >(outFour)  - 
                        _results[3]->data;
}


void CompressorRLE4HF::decompress( const void* const* inData, 
                                   const uint64_t* const inSizes,
                                   void* const outData, 
                                   const uint64_t* const outSize )
{
    
    const uint16_t* const* inData16 = reinterpret_cast< const uint16_t* const* >
                                          ( inData );
    uint16_t*             out = reinterpret_cast< uint16_t* >( outData );

    // decompress 
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
    uint64_t numBlock = inSizes[0] / 4;
    size_t sizeHeader  = sizeof( Header ) / 2 ;

    for( uint64_t i = 0; i < numBlock ; i++ )
    {
        
        const uint16_t* oneIn   = inData16[ i*4 + 0 ] + sizeHeader;
        const uint16_t* twoIn   = inData16[ i*4 + 1 ] + sizeHeader;
        const uint16_t* threeIn = inData16[ i*4 + 2 ] + sizeHeader;
        const uint16_t* fourIn  = inData16[ i*4 + 3 ] + sizeHeader;
                
        uint16_t one(0), two(0), three(0), four(0);
        uint16_t oneLeft(0), twoLeft(0), threeLeft(0), fourLeft(0);

        Header header = _readHeader( 
            reinterpret_cast<const uint8_t*> ( inData16[i*4] ) );
        
        for( uint32_t j = 0; j < header.size / 2; ++j )
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
            out[j*4] = one; 
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

                //two <<= 8;
            }
            --twoLeft;

            out[j*4+1] = two;
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
            out[j*4+2] = three;

            if( fourLeft == 0 )
            {
               four = *fourIn; ++fourIn;
               if( four == _rleMarker )
               {
                   four     = *fourIn; ++fourIn;
                   fourLeft = *fourIn; ++fourIn;
               }
               else // single symbol
                   fourLeft = 1;

            }
            --fourLeft;
            out[j*4+3] = four;
       }
    }
}
}
}
