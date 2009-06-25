
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

#include "compressorRLE4B.h"

namespace eq
{
namespace plugin
{
static const uint64_t _rleMarker = 0x42; // just a random number

namespace
{
class NoSwizzle
{
public:
    static uint32_t swizzle( const uint32_t input ) { return input; }
    static uint32_t deswizzle( const uint32_t input ) { return input; }
};

class SwizzleUInt32
{
public:
    static inline uint32_t swizzle( const uint32_t input )
    {
        return (( input &  ( EQ_BIT32 | EQ_BIT31 | EQ_BIT22 | EQ_BIT21 | 
                             EQ_BIT12 | EQ_BIT11 | EQ_BIT2 | EQ_BIT1 ))        |
                (( input & ( EQ_BIT8  | EQ_BIT7 ))<<18 )                       |
                (( input & ( EQ_BIT24 | EQ_BIT23 | EQ_BIT14 | EQ_BIT13 ))<<6 ) |
                (( input & ( EQ_BIT16 | EQ_BIT15 | EQ_BIT6  | 
                             EQ_BIT5  | EQ_BIT4  | EQ_BIT3 )) <<12 )           |
                (( input & ( EQ_BIT28 | EQ_BIT27 | EQ_BIT26 | EQ_BIT25 ))>>18) |
                (( input & ( EQ_BIT18 | EQ_BIT17 ))>>12)                       |
                (( input & ( EQ_BIT30 | EQ_BIT29 | EQ_BIT20 | EQ_BIT19 |
                             EQ_BIT10 | EQ_BIT9 ))>>6 ));
    }
    static inline uint32_t deswizzle( const uint32_t input )
    {
        return ((  input & ( EQ_BIT32 | EQ_BIT31 | EQ_BIT22 | EQ_BIT21 | 
                             EQ_BIT11 | EQ_BIT12 | EQ_BIT2 | EQ_BIT1 ))        |
                (( input & ( EQ_BIT26 | EQ_BIT25 )) >>18 )                     |
                (( input & ( EQ_BIT30 | EQ_BIT29 | EQ_BIT20 | EQ_BIT19 ))>>6 ) |
                (( input & ( EQ_BIT28 | EQ_BIT27 | EQ_BIT18 | 
                             EQ_BIT17 | EQ_BIT16 |EQ_BIT15 ))>>12 )            |
                (( input & ( EQ_BIT10 | EQ_BIT9  | EQ_BIT8  | EQ_BIT7 ))<<18 ) |
                (( input & ( EQ_BIT6  | EQ_BIT5 ))<<12 )                       |
                (( input & ( EQ_BIT24 | EQ_BIT23 | EQ_BIT14 | 
                             EQ_BIT13 | EQ_BIT4  | EQ_BIT3 ))<<6 ));
    }
};
 
class SwizzleUInt24
{
public:
    static inline uint32_t swizzle( const uint32_t input )
    {
        return (( input &  ( EQ_BIT24 | EQ_BIT23 | EQ_BIT22 | EQ_BIT13 | 
                             EQ_BIT12 | EQ_BIT3  | EQ_BIT2  | EQ_BIT1 )) |
                (( input & ( EQ_BIT16 | EQ_BIT15 | EQ_BIT14 )) << 5 )    | 
                (( input & ( EQ_BIT11 | EQ_BIT10 | EQ_BIT9  )) >> 5 )    |
                (( input & ( EQ_BIT8  | EQ_BIT7  | EQ_BIT6  | EQ_BIT5  |
                             EQ_BIT4  )) << 10 )                         |
                (( input & ( EQ_BIT21 | EQ_BIT20 | EQ_BIT19 | EQ_BIT18 |
                             EQ_BIT17 )) >> 10 ));
    }
    static inline uint32_t deswizzle( const uint32_t input )
    {
        return ((  input & ( EQ_BIT24 | EQ_BIT23 | EQ_BIT22 | EQ_BIT13 | 
                             EQ_BIT12 | EQ_BIT3  | EQ_BIT2  | EQ_BIT1 )) |
                (( input & ( EQ_BIT21 | EQ_BIT20 | EQ_BIT19 ))>>5 )      |
                (( input & ( EQ_BIT6  | EQ_BIT5  | EQ_BIT4 ))<<5 )       | 
                (( input & ( EQ_BIT18 | EQ_BIT17 | EQ_BIT16 | 
                             EQ_BIT15 | EQ_BIT14 ))>>10 )                |
                (( input & ( EQ_BIT11 | EQ_BIT10 | EQ_BIT9  | 
                             EQ_BIT8  | EQ_BIT7 ))<<10 ));
    }    
};  

class UseAlpha
{
public:
    static inline bool use() { return true; }
};

class NoAlpha
{
public:
    static inline bool use() { return false; }
};

union ByteWord
{
    uint8_t  bytes[4];
    uint32_t word;
};

template< typename swizzleFunc, typename alphaFunc >
static inline void _compress( const void* const input, const uint64_t size,
                              Result** results )
{
    uint8_t* oneOut(   results[ 0 ]->data ); 
    uint8_t* twoOut(   results[ 1 ]->data ); 
    uint8_t* threeOut( results[ 2 ]->data ); 
    uint8_t* fourOut(  results[ 3 ]->data ); 

    const uint32_t* input32 = reinterpret_cast< const uint32_t* >( input );
    ByteWord token;
    token.word = swizzleFunc::swizzle( *input32 );

    uint8_t oneLast( token.bytes[0] ), twoLast( token.bytes[1] ),
            threeLast( token.bytes[2] ), fourLast( token.bytes[3] );
    
    uint8_t oneSame( 1 ), twoSame( 1 ), threeSame( 1 ), fourSame( 1 );
    uint8_t one, two, three, four;
    
    for( uint32_t i = 1; i < size; ++i )
    {
        ++input32;
        token.word = swizzleFunc::swizzle( *input32 );
        
        one = token.bytes[0];
        WRITE( one );

        two = token.bytes[1];
        WRITE( two );

        three = token.bytes[2];
        WRITE( three );

        if( alphaFunc::use( ))
        {
            four = token.bytes[3];
            WRITE( four );
        }
    }

    WRITE_OUTPUT( one );
    WRITE_OUTPUT( two );
    WRITE_OUTPUT( three )
    WRITE_OUTPUT( four );

    results[0]->size = oneOut   - results[0]->data;
    results[1]->size = twoOut   - results[1]->data;
    results[2]->size = threeOut - results[2]->data;
    results[3]->size = fourOut  - results[3]->data;
}

template< typename swizzleFunc, typename alphaFunc >
static inline void _decompress( const void* const* inData,
                                const uint64_t* const inSizes,
                                const unsigned numInputs,
                                void* const outData, const uint64_t outSize )
{
    const uint64_t size = outSize * 4 ;
    const float width = static_cast< float >( size ) /  
                        static_cast< float >( numInputs );

    const uint8_t* const* inData8 = reinterpret_cast< const uint8_t* const* >(
        inData );

    // decompress 
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
    assert( (numInputs%4) == 0 );

#pragma omp parallel for
    for( ssize_t i = 0; i < numInputs ; i+=4 )
    {
        const uint32_t startIndex = static_cast< uint32_t >( i/4.f * width ) *4;
        const uint32_t nextIndex  =
            static_cast< uint32_t >(( i/4.f + 1.f ) * width ) * 4;
        const uint64_t chunkSize = ( nextIndex - startIndex ) / 4;
        uint32_t* out = reinterpret_cast< uint32_t* >( outData ) + startIndex/4;

        const uint8_t* oneIn  = inData8[ i + 0 ];
        const uint8_t* twoIn  = inData8[ i + 1 ];
        const uint8_t* threeIn= inData8[ i + 2 ];
        const uint8_t* fourIn = inData8[ i + 3 ];
        
        uint8_t one(0), two(0), three(0), four(0);
        uint8_t oneLeft(0), twoLeft(0), threeLeft(0), fourLeft(0);
   
        for( uint32_t j = 0; j < chunkSize ; ++j )
        {
            assert( static_cast<uint64_t>(oneIn-inData8[i+0]) <= inSizes[i+0] );
            assert( static_cast<uint64_t>(twoIn-inData8[i+1]) <= inSizes[i+1] );
            assert( static_cast<uint64_t>(threeIn-inData8[i+2]) <=inSizes[i+2]);

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

            if( alphaFunc::use( ))
            {
                assert( static_cast<uint64_t>( fourIn-inData8[i+3] ) <=
                        inSizes[i+3] );
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

                *out = swizzleFunc::deswizzle( 
                    one + (two<<8) + (three<<16) + (four<<24) );
            }
            else
                *out = swizzleFunc::deswizzle( one + (two<<8) + (three<<16) );
            ++out;
        }
        assert( static_cast<uint64_t>(oneIn-inData8[i+0])   == inSizes[i+0] );
        assert( static_cast<uint64_t>(twoIn-inData8[i+1])   == inSizes[i+1] );
        assert( static_cast<uint64_t>(threeIn-inData8[i+2]) == inSizes[i+2] );
    }
}
}

void CompressorRLE4B::compress( const void* const inData, const uint64_t inSize,
                                const bool useAlpha, const bool swizzle )
{
    const uint64_t size = inSize * 4 ;
    _setupResults( 4, size );

    const ssize_t numResults = _results.size();
    const float width = static_cast< float >( size ) /  
                        static_cast< float >( numResults );

    const uint8_t* const data = 
        reinterpret_cast< const uint8_t* const >( inData );
    
#pragma omp parallel for
    for( ssize_t i = 0; i < numResults ; i += 4 )
    {
        const uint32_t startIndex = static_cast< uint32_t >( i/4 * width ) * 4;
        const uint32_t nextIndex = 
            static_cast< uint32_t >(( i/4 + 1 ) * width ) * 4;
        const uint64_t chunkSize = ( nextIndex - startIndex ) / 4;

        if( useAlpha )
            if( swizzle )
                _compress< SwizzleUInt32, UseAlpha >( &data[ startIndex ],
                                                      chunkSize,
                                                      &_results[i] );
            else
                _compress< NoSwizzle, UseAlpha >( &data[ startIndex ],
                                                  chunkSize,
                                                  &_results[i] );
        else
            if( swizzle )
                _compress< SwizzleUInt24, NoAlpha >( &data[ startIndex ], 
                                                     chunkSize,
                                                     &_results[i] );
            else
                _compress< NoSwizzle, NoAlpha >( &data[ startIndex ], chunkSize,
                                                 &_results[i] );
    }
}

void CompressorRLE4B::decompress( const void* const* inData, 
                                  const uint64_t* const inSizes, 
                                  const unsigned numInputs,
                                  void* const outData, 
                                  const uint64_t outSize,
                                  const bool useAlpha )
{
    if( useAlpha )
        _decompress< NoSwizzle, UseAlpha >( inData, inSizes, numInputs, 
                                            outData, outSize );
    else
        _decompress< NoSwizzle, NoAlpha >( inData, inSizes, numInputs,
                                           outData, outSize );
}

void CompressorDiffRLE4B::decompress( const void* const* inData, 
                                      const uint64_t* const inSizes, 
                                      const unsigned numInputs,
                                      void* const outData,
                                      const uint64_t outSize,
                                      const bool useAlpha )
{
    if( useAlpha )
        _decompress< SwizzleUInt32, UseAlpha >( inData, inSizes, numInputs,
                                                outData, outSize );
    else
        _decompress< SwizzleUInt24, NoAlpha >( inData, inSizes, numInputs,
                                               outData, outSize );
}

    
}
}
