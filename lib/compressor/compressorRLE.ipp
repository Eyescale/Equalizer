
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

#include <eq/base/omp.h>

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
static inline void _compress( const void* const input, const uint64_t nPixels,
                              eq::plugin::Compressor::Result** results )
{
    const PixelType* pixel = reinterpret_cast< const PixelType* >( input );

    ComponentType* oneOut(   reinterpret_cast< ComponentType* >( 
                                 results[ 0 ]->getData( ))); 
    ComponentType* twoOut(   reinterpret_cast< ComponentType* >( 
                                 results[ 1 ]->getData( )));
    ComponentType* threeOut( reinterpret_cast< ComponentType* >( 
                                 results[ 2 ]->getData( )));
    ComponentType* fourOut(  reinterpret_cast< ComponentType* >( 
                                 results[ 3 ]->getData( )));

    ComponentType oneLast(0), twoLast(0), threeLast(0), fourLast(0);
    if( alphaFunc::use( ))
        swizzleFunc::swizzle( *pixel, oneLast, twoLast, threeLast, fourLast );
    else
        swizzleFunc::swizzle( *pixel, oneLast, twoLast, threeLast );
    
    ComponentType oneSame( 1 ), twoSame( 1 ), threeSame( 1 ), fourSame( 1 );
    ComponentType one(0), two(0), three(0), four(0);
    
    for( uint64_t i = 1; i < nPixels; ++i )
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

#define READ( name )                                        \
    if( name ## Left == 0 )                                 \
    {                                                       \
        name = *name ## In;                                 \
        if( name == _rleMarker )                            \
        {                                                   \
            name = name ## In[1];                           \
            name ## Left = name ## In[2];                   \
            name ## In += 3;                                \
        }                                                   \
        else                                                \
        {                                                   \
            name ## Left = 1;                               \
            ++name ## In;                                   \
        }                                                   \
    }                                                       \
    --name ## Left; 

template< typename PixelType, typename ComponentType,
          typename swizzleFunc, typename alphaFunc >
static inline void _decompress( const void* const* inData,
                                const uint64_t* const inSizes,
                                const unsigned nInputs,
                                void* const outData, const uint64_t nPixels )
{
    assert( (nInputs % 4) == 0 );
    assert( (inSizes[0] % sizeof( ComponentType )) == 0 ); 
    assert( (inSizes[1] % sizeof( ComponentType )) == 0 ); 
    assert( (inSizes[2] % sizeof( ComponentType )) == 0 ); 

    const uint64_t nElems = nPixels * 4;
    const float width = static_cast< float >( nElems ) /  
                        static_cast< float >( nInputs );

    const ComponentType* const* in = 
        reinterpret_cast< const ComponentType* const* >( inData );

#pragma omp parallel for
    for( ssize_t i = 0; i < static_cast< ssize_t >( nInputs ) ; i+=4 )
    {
        const uint64_t startIndex = static_cast<uint64_t>( i/4 * width ) * 4;
        const uint64_t nextIndex  =
            static_cast< uint64_t >(( i/4 + 1 ) * width ) * 4;
        const uint64_t chunkSize = ( nextIndex - startIndex ) / 4;
        PixelType* out = reinterpret_cast< PixelType* >( outData ) + 
                         startIndex / 4;

        const ComponentType* oneIn   = in[ i + 0 ];
        const ComponentType* twoIn   = in[ i + 1 ];
        const ComponentType* threeIn = in[ i + 2 ];
        const ComponentType* fourIn  = in[ i + 3 ];
        
        ComponentType one(0), two(0), three(0), four(0);
        ComponentType oneLeft(0), twoLeft(0), threeLeft(0), fourLeft(0);
   
        for( uint64_t j = 0; j < chunkSize ; ++j )
        {
            assert( static_cast< uint64_t >( oneIn-in[i+0])   <= 
                    inSizes[i+0] / sizeof( ComponentType ) );
            assert( static_cast< uint64_t >( twoIn-in[i+1])   <=
                    inSizes[i+1] / sizeof( ComponentType ) );
            assert( static_cast< uint64_t >( threeIn-in[i+2]) <=
                    inSizes[i+2] / sizeof( ComponentType ) );

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
        assert( static_cast< uint64_t >( oneIn-in[i+0] )   ==
                inSizes[i+0] / sizeof( ComponentType ) );
        assert( static_cast< uint64_t >( twoIn-in[i+1] )   ==
                inSizes[i+1] / sizeof( ComponentType ) );
        assert( static_cast< uint64_t >( threeIn-in[i+2] ) ==
                inSizes[i+2] / sizeof( ComponentType ) );
    }
}

static void _setupResults( const uint32_t nChannels, const eq_uint64_t inSize,
                           eq::plugin::Compressor::ResultVector& results )
{
    // determine number of chunks and set up output data structure
#ifdef EQ_USE_OPENMP
    const size_t nChunks = nChannels * eq::base::OMP::getNThreads() * 4;
#else
    const size_t nChunks = nChannels;
#endif

    while( results.size() < nChunks )
        results.push_back( new eq::plugin::Compressor::Result );

    // The maximum possible size is twice the input size for each chunk, since
    // the worst case scenario is input made of tupels of 'rle marker, data'
    const eq_uint64_t maxChunkSize = (inSize/nChunks + 1) * 2;
    for( size_t i = 0; i < nChunks; ++i )
        results[i]->resize( maxChunkSize );
}

template< typename PixelType, typename ComponentType,
          typename swizzleFunc, typename alphaFunc >
static inline void _compress( const void* const inData, 
                              const eq_uint64_t nPixels, const bool useAlpha,
                              const bool swizzle, 
                              eq::plugin::Compressor::ResultVector& results )
{
    const uint64_t size = nPixels * sizeof( PixelType );
    _setupResults( 4, size, results );

    const uint64_t nElems = nPixels * 4;
    const uint64_t nResults = results.size();
    const float width = static_cast< float >( nElems ) /  
                        static_cast< float >( nResults );

    const ComponentType* const data = 
        reinterpret_cast< const ComponentType* const >( inData );
    
#pragma omp parallel for
    for( ssize_t i = 0; i < static_cast< ssize_t >( nResults ) ; i += 4 )
    {
        const uint64_t startIndex = static_cast< uint64_t >( i/4 * width ) * 4;
        const uint64_t nextIndex = 
            static_cast< uint64_t >(( i/4 + 1 ) * width ) * 4;
        const uint64_t chunkSize = ( nextIndex - startIndex ) / 4;

        _compress< PixelType, ComponentType, swizzleFunc, alphaFunc >(
            &data[ startIndex ], chunkSize, &results[i] );
    }
}

}
