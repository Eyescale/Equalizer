
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * Template functions used by all compression routines
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

namespace
{

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

#define WRITE_OUTPUT( name )                                            \
    {                                                                   \
        if( name ## Last == _rleMarker )                                \
        {                                                               \
            name ## Out[0] = _rleMarker;                                \
            name ## Out[1] = _rleMarker;                                \
            name ## Out[2] = name ## Same;                              \
            name ## Out += 3;                                           \
        }                                                               \
        else                                                            \
            switch( name ## Same )                                      \
            {                                                           \
                case 0:                                                 \
                    break;                                              \
                case 2:                                                 \
                    name ## Out[0] = name ## Last;                      \
                    name ## Out[1] = name ## Last;                      \
                    name ## Out += 2;                                   \
                    break;                                              \
                case 1:                                                 \
                    name ## Out[0] = name ## Last;                      \
                    ++(name ## Out);                                    \
                    break;                                              \
                default:                                                \
                    name ## Out[0] = _rleMarker;                        \
                    name ## Out[1] = name ## Last;                      \
                    name ## Out[2] = name ## Same;                      \
                    name ## Out += 3;                                   \
                    break;                                              \
            }                                                           \
    }

#define WRITE( name )                                                   \
    if( name == name ## Last && name ## Same != 255 )                   \
        ++(name ## Same );                                              \
    else                                                                \
    {                                                                   \
        WRITE_OUTPUT( name );                                           \
        name ## Last = name;                                            \
        name ## Same = 1;                                               \
    }


template< typename PixelType, typename ComponentType,
          typename swizzleFunc, typename alphaFunc >
static inline void _compress( const void* const input, const uint64_t size,
                              eq::plugin::Compressor::Result** results )
{
    const PixelType* pixel = reinterpret_cast< const PixelType* >( input );

    ComponentType* oneOut(   results[ 0 ]->getData( )); 
    ComponentType* twoOut(   results[ 1 ]->getData( )); 
    ComponentType* threeOut( results[ 2 ]->getData( )); 
    ComponentType* fourOut(  results[ 3 ]->getData( )); 

    ComponentType oneLast(0), twoLast(0), threeLast(0), fourLast(0);
    if( alphaFunc::use( ))
        swizzleFunc::swizzle( *pixel, oneLast, twoLast, threeLast, fourLast );
    else
        swizzleFunc::swizzle( *pixel, oneLast, twoLast, threeLast );
    
    ComponentType oneSame( 1 ), twoSame( 1 ), threeSame( 1 ), fourSame( 1 );
    ComponentType one(0), two(0), three(0), four(0);
    
    for( uint64_t i = 1; i < size; ++i )
    {
        ++pixel;

        if( alphaFunc::use( ))
        {
            swizzleFunc::swizzle( *pixel, one, two, three, four );
            WRITE( one );
            WRITE( two );
            WRITE( three );
            WRITE( four );
        }
        else
        {
            swizzleFunc::swizzle( *pixel, one, two, three );
            WRITE( one );
            WRITE( two );
            WRITE( three );
        }
    }

    WRITE_OUTPUT( one );
    WRITE_OUTPUT( two );
    WRITE_OUTPUT( three )
    WRITE_OUTPUT( four );

    results[0]->setSize( reinterpret_cast< uint8_t* > ( oneOut )  -
                         results[0]->getData( ));
    results[1]->setSize( reinterpret_cast< uint8_t* >( twoOut )   -
                         results[1]->getData( ));
    results[2]->setSize( reinterpret_cast< uint8_t* >( threeOut ) -
                         results[2]->getData( ));
    results[3]->setSize( reinterpret_cast< uint8_t* >( fourOut )  -
                         results[3]->getData( ));
}

#if 0
template< typename PixelType, typename ComponentType,
          typename swizzleFunc, typename alphaFunc >
static inline void _decompress( const void* const* inData,
                                const uint64_t* const inSizes,
                                const unsigned numInputs,
                                void* const outData, const uint64_t nPixels )
{
    const uint64_t size = nPixels * sizeof( PixelType );
    const float width = static_cast< float >( size ) /  
                        static_cast< float >( numInputs );

    const ComponentType* const* inData8 = reinterpret_cast< const ComponentType* const* >(
        inData );

    assert( (numInputs%4) == 0 );

#pragma omp parallel for
    for( ssize_t i = 0; i < static_cast< ssize_t >( numInputs ) ; i+=4 )
    {
        const uint32_t startIndex = static_cast< uint32_t >( i/4.f * width ) *4;
        const uint32_t nextIndex  =
            static_cast< uint32_t >(( i/4.f + 1.f ) * width ) * 4;
        const uint64_t chunkSize = ( nextIndex - startIndex ) / 4;
        PixelType* out = reinterpret_cast< PixelType* >( outData ) + startIndex/4;

        const ComponentType* oneIn  = inData8[ i + 0 ];
        const ComponentType* twoIn  = inData8[ i + 1 ];
        const ComponentType* threeIn= inData8[ i + 2 ];
        const ComponentType* fourIn = inData8[ i + 3 ];
        
        ComponentType one(0), two(0), three(0), four(0);
        ComponentType oneLeft(0), twoLeft(0), threeLeft(0), fourLeft(0);
   
        for( PixelType j = 0; j < chunkSize ; ++j )
        {
            assert( static_cast<uint64_t>(oneIn-inData8[i+0]) <= inSizes[i+0] );
            assert( static_cast<uint64_t>(twoIn-inData8[i+1]) <= inSizes[i+1] );
            assert( static_cast<uint64_t>(threeIn-inData8[i+2]) <=inSizes[i+2]);

            if( alphaFunc::use( ))
            {
                READ( one );
                READ( two );
                READ( three );
                READ( four );

                *out = swizzleFunc::deswizzle( one, two, three, four );
            }
            else
            {
                READ( one );
                READ( two );
                READ( three );

                *out = swizzleFunc::deswizzle( one, two, three );
            }
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

    const ComponentType* const data = 
        reinterpret_cast< const ComponentType* const >( inData );
    
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
                                  const uint64_t nPixels,
                                  const bool useAlpha )
{
    if( useAlpha )
        _decompress< NoSwizzle, UseAlpha >( inData, inSizes, numInputs, 
                                            outData, nPixels );
    else
        _decompress< NoSwizzle, NoAlpha >( inData, inSizes, numInputs,
                                           outData, nPixels );
}

void CompressorDiffRLE4B::decompress( const void* const* inData, 
                                      const uint64_t* const inSizes, 
                                      const unsigned numInputs,
                                      void* const outData,
                                      const uint64_t nPixels,
                                      const bool useAlpha )
{
    if( useAlpha )
        _decompress< SwizzleUInt32, UseAlpha >( inData, inSizes, numInputs,
                                                outData, nPixels );
    else
        _decompress< SwizzleUInt24, NoAlpha >( inData, inSizes, numInputs,
                                               outData, nPixels );
}
#endif

}
