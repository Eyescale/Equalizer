
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

namespace
{
// NaN
static const uint16_t _rleMarker = 0xffff; 
}

#include "compressorRLE.ipp"

namespace co
{
namespace plugin
{
namespace
{
REGISTER_ENGINE( CompressorRLE4HF, RGBA16F, RGBA16F, 1., 0.7, 1., true );
REGISTER_ENGINE( CompressorRLE4HF, BGRA16F, BGRA16F, 1., 0.7, 1., true );
REGISTER_ENGINE( CompressorDiffRLE4HF, DIFF_RGBA16F,    \
                 RGBA16F, 1., 0.9, 1., true );
REGISTER_ENGINE( CompressorDiffRLE4HF, DIFF_BGRA16F,    \
                 BGRA16F, 1., 0.9, 1., true );
        
class NoSwizzle
{
public:
    static inline void swizzle( const uint64_t input, uint16_t& one,
                                uint16_t& two, uint16_t& three, uint16_t& four )
        {
            one   = input & 0xffffull;
            two   = (input & 0xffff0000ull) >> 16;
            three = (input & 0xffff00000000ull) >> 32;
            four  = (input & 0xffff000000000000ull) >> 48;
        }

    static inline void swizzle( const uint64_t input, uint16_t& one,
                                uint16_t& two, uint16_t& three )
        {
            one   = input & 0xffffull;
            two   = (input & 0xffff0000ull) >> 16;
            three = (input & 0xffff00000000ull) >> 32;
        }

    static inline uint64_t deswizzle( const uint16_t one, const uint16_t two,
                                      const uint16_t three, const uint16_t four)
    {
        return 
            one +
            ( static_cast< uint64_t >( two ) << 16 ) +
            ( static_cast< uint64_t > ( three ) << 32) +
            ( static_cast< uint64_t > ( four ) << 48 );
    }

    static inline uint64_t deswizzle( const uint16_t one, const uint16_t two,
                                      const uint16_t three )
    {
        return 
            one +
            ( static_cast< uint64_t >( two ) << 16 ) +
            ( static_cast< uint64_t > ( three ) << 32);
    }
};

class Swizzle
{
public:
    static inline void swizzle( const uint64_t input, uint16_t& one,
                                uint16_t& two, uint16_t& three, uint16_t& four )
    {
         NoSwizzle::swizzle(
            ( input & 0xffc0000007c0001full ) |
            (( input & 0x3f0000ull) >> 11 ) |
            (( input & 0x7ff00000000ull) >> 21 ) |
            (( input & 0xc0003fe0ull) << 22 ) |
            (( input & 0x38000000ull) << 9 ) |
            (( input & 0x3f000000000000ull) >> 9 ) |
            (( input & 0xf80000000000ull) << 2 ) |
            (( input & 0xc000ull) << 36 ) ,one, two, three, four );
    }

    static inline void swizzle( const uint64_t input, uint16_t& one,
                                uint16_t& two, uint16_t& three )
    {
         NoSwizzle::swizzle(
                ( input & 0xc073070c380ull )|
               (( input & 0x3c000000000ull ) >> 38 ) |    // one
               (( input & 0x3800000ull ) >> 19 ) |
               (( input & 0xc00000000000ull ) >> 36 ) |
               (( input & 0x3000c0000000ull ) >> 18 )|

               (( input & 0x78ull ) << 13 )|            //two
               (( input & 0x3800000000ull ) >> 12 )|

               (( input & 0xc00ull ) << 36 )|       //three
               (( input & 0x7ull ) << 39 )|
               (( input & 0xc003000ull ) << 18 )|
               (( input & 0xf0000ull ) << 19 ),one, two, three);
    }

    static inline uint64_t deswizzle( const uint16_t one, const uint16_t two,
                                      const uint16_t three, const uint16_t four)
    {
        uint64_t output = NoSwizzle::deswizzle( one, two, three, four );

        return (( output & 0xffc0000007c0001full ) |
               (( output & 0x7e0ull ) << 11 ) |
               (( output & 0x3ff800ull ) << 21 ) |
               (( output & 0x30000ff8000000ull ) >> 22 ) |
               (( output & 0x7000000000ull) >> 9 ) |
               (( output & 0x1f8000000000ull ) << 9 ) |
               (( output & 0x3e00000000000ull ) >> 2 ) |
               (( output & 0xc000000000000ull ) >> 36 ) );


    }

    static inline uint64_t deswizzle( const uint16_t one, const uint16_t two,
                                      const uint16_t three )
    {
        uint64_t output = NoSwizzle::deswizzle( one, two, three );
        return ( output & 0xc073070c380ull ) |
               (( output & 0xfull ) << 38 ) |    // one
               (( output & 0x70ull ) << 19 ) |
               (( output & 0xc00ull ) << 36 ) |
               (( output & 0xc003000ull ) << 18 )|

               (( output & 0xf0000ull ) >> 13 )|            //two
               (( output & 0x3800000ull ) << 12 )|

               (( output & 0xc00000000000ull ) >> 36 )|       //three
               (( output & 0x38000000000ull ) >> 39 )|
               (( output & 0x3000c0000000ull ) >> 18 )|
               (( output & 0x7800000000ull ) >> 19 );
    }
};
}

void CompressorRLE4HF::compress( const void* const inData, 
                                 const eq_uint64_t nPixels, const bool useAlpha,
                                 const bool swizzle )
{
    if ( swizzle )
    {
        if( useAlpha )
            _nResults = _compress< uint64_t, uint16_t, Swizzle, UseAlpha >(
                            inData, nPixels, _results );
        else
            _nResults = _compress< uint64_t, uint16_t, Swizzle, NoAlpha >(
                            inData, nPixels, _results );
    }
    else 
    {
        if( useAlpha )
            _nResults = _compress< uint64_t, uint16_t, NoSwizzle, UseAlpha >(
                            inData, nPixels, _results );
        else
            _nResults = _compress< uint64_t, uint16_t, NoSwizzle, NoAlpha >(
                            inData, nPixels, _results );
    }
}

void CompressorRLE4HF::decompress( const void* const* inData, 
                                   const eq_uint64_t* const inSizes,
                                   const unsigned nInputs, void* const outData, 
                                   const eq_uint64_t nPixels, 
                                   const bool useAlpha )
{
    if( useAlpha )
        _decompress< uint64_t, uint16_t, NoSwizzle, UseAlpha >( 
            inData, inSizes, nInputs, outData, nPixels );
    else
        _decompress< uint64_t, uint16_t, NoSwizzle, NoAlpha >(
            inData, inSizes, nInputs, outData, nPixels );
}


void CompressorDiffRLE4HF::decompress( const void* const* inData, 
                                   const eq_uint64_t* const inSizes,
                                   const unsigned nInputs, void* const outData, 
                                   const eq_uint64_t nPixels, 
                                   const bool useAlpha )
{
    if( useAlpha )
        _decompress< uint64_t, uint16_t, Swizzle, UseAlpha >( 
            inData, inSizes, nInputs, outData, nPixels );
    else
        _decompress< uint64_t, uint16_t, Swizzle, NoAlpha >(
            inData, inSizes, nInputs, outData, nPixels );
}
}
}
