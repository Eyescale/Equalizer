
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009, Makhinya Maxim
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
 
#include "compressorRLE4U.h"

namespace
{
// NaN
static const uint64_t _rleMarker = 0xE3A49A3D; // just a random number
}

namespace eq
{
namespace plugin
{


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


#define WRITE_OUTPUT                                                    \
    {                                                                   \
        if( lastSymbol == _rleMarker )                                  \
        {                                                               \
            out[ outPos++ ] = _rleMarker;                               \
            out[ outPos++ ] = lastSymbol;                               \
            out[ outPos++ ] = nSame;                                    \
        }                                                               \
        else                                                            \
            switch( nSame )                                             \
            {                                                           \
                case 0:                                                 \
                    EQASSERTINFO( false, "Unreachable code");           \
                    break;                                              \
                case 3:                                                 \
                    out[ outPos++ ] = lastSymbol; /* fall through */    \
                case 2:                                                 \
                    out[ outPos++ ] = lastSymbol; /* fall through */    \
                case 1:                                                 \
                    out[ outPos++ ] = lastSymbol;                       \
                    break;                                              \
                default:                                                \
                    out[ outPos++ ] = _rleMarker;                       \
                    out[ outPos++ ] = lastSymbol;                       \
                    out[ outPos++ ] = nSame;                            \
                    break;                                              \
            }                                                           \
        EQASSERTINFO( nWords<<1 >= outPos,                              \
                      "Overwrite array bounds during image compress" ); \
    }


static uint64_t _compress( const uint64_t* data, const uint64_t nWords, 
                                 uint64_t* out )
{
    out[ 0 ] = nWords;

    uint64_t outPos     = 1;
    uint64_t nSame      = 1;
    uint64_t lastSymbol = data[0];

    for( uint64_t i=1; i<nWords; ++i )
    {
        const uint64_t symbol = data[i];

        if( symbol == lastSymbol )
            ++nSame;
        else
        {
            WRITE_OUTPUT;
            lastSymbol = symbol;
            nSame      = 1;
        }
    }

    WRITE_OUTPUT;
    return (outPos<<3);
}

void CompressorRLE4U::compress( const void* const inData,
                                  const eq_uint64_t nPixels,
                                  const bool        useAlpha )
{
    const uint64_t size = nPixels * sizeof( uint32_t );
    EQASSERT( size > 0 );

    _setupResults( 1, size, _results );

    const uint64_t nElems   = (size%8) ? (size>>3)+1 : (size>>3);
    const uint64_t nResults = _results.size();


    const float width = static_cast< float >( nElems ) /
                        static_cast< float >( nResults );

    const uint64_t* const data =
        reinterpret_cast< const uint64_t* const >( inData );

#pragma omp parallel for
    for ( uint64_t i = 0; i < nResults; ++i )
    {
        const uint64_t startIndex = static_cast< uint64_t >( i * width );
        const uint64_t endIndex   = static_cast< uint64_t >( (i+1) * width );
        uint64_t*      out        = reinterpret_cast< uint64_t* >(
                                                     _results[i]->getData( ));

        const uint64_t cSize = _compress( &data[ startIndex ],
                                                    endIndex-startIndex, out );
        _results[i]->setSize( cSize );
    }
}


void CompressorRLE4U::decompress( const void* const* inData, 
                                   const eq_uint64_t* const inSizes,
                                   const unsigned nInputs, void* const outData, 
                                   const eq_uint64_t nPixels, 
                                   const bool useAlpha )
{

    if( nPixels == 0 )
        return;

    const uint64_t size = nPixels * sizeof( uint32_t );

    // Prepare table with input pointer into decompressed data
    //   Needed since decompress loop is parallelized
    uint64_t**    outTable = static_cast< uint64_t** >(
        alloca( nInputs * sizeof( uint64_t* )));

    {
        uint8_t* out = reinterpret_cast< uint8_t* >( outData );
        for( unsigned i = 0; i < nInputs; ++i )
        {
            outTable[i] = reinterpret_cast< uint64_t* >( out );

            const uint64_t* in  = 
                reinterpret_cast< const uint64_t* >( inData[i] );
            const uint64_t nWords = in[0];
            out += nWords * sizeof( uint64_t );
        }
        EQASSERTINFO(
                size >= (uint64_t)(out-reinterpret_cast<uint8_t*>(outData)-7),
                "Pixel data size does not match expected image size: "
                << size << " ? " 
                << (uint64_t)(out-reinterpret_cast<uint8_t*>(outData)-7));
    }

    // decompress each block
    // On OS X the loop is sometimes slower when parallelized. Investigate this!
#pragma omp parallel for
    for( unsigned i = 0; i < nInputs; ++i )
    {
        const uint64_t* in  = reinterpret_cast< const uint64_t* >( inData[i] );
              uint64_t* out = outTable[i];

        uint64_t       outPos = 0;
        const uint64_t endPos = in[0];
        uint64_t       inPos  = 1;

        while( outPos < endPos )
        {
            const uint64_t token = in[inPos++];
            if( token == _rleMarker )
            {
                const uint64_t symbol = in[inPos++];
                const uint64_t nSame  = in[inPos++];
                EQASSERT( outPos + nSame <= endPos );

                for( uint32_t j = 0; j<nSame; ++j )
                    out[outPos++] = symbol;
            }
            else // symbol
                out[outPos++] = token;

            EQASSERTINFO( ((outPos-1) << 3) <= size,
                          "Overwrite array bounds during decompress" );
        }
        EQASSERT( outPos == endPos );
    }
}

}
}
