
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

#include "compressorRLEYUV.h"

namespace
{
static const uint8_t _rleMarker = 0x42; // just a random number
}

#include "compressorRLE.ipp"

namespace co
{
namespace plugin
{
namespace
{
REGISTER_ENGINE( CompressorRLEYUV, YUVA_50P, YUVA_50P, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorDiffRLEYUV, DIFF_YUVA_50P,   \
                 YUVA_50P, 1., 0.5, 1.1, true );

class NoSwizzle
{
public:
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three, uint8_t& four ) 
        {
            one = input & 0xff;
            two = (input & 0xff00) >> 8;
            three = (input & 0xff0000) >> 16;
            four = (input & 0xff000000) >> 24;
        }

    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        {
            one = input & 0xff;
            two = (input & 0xff00) >> 8;
            three = (input & 0xff0000) >> 16;
        }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
    {
        return one + (two<<8) + (three<<16) + (four<<24);
    }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three )
    {
        return one + (two<<8) + (three<<16);
    }
};

class SwizzleUInt32
{
public:
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three, uint8_t& four ) 
    {
        NoSwizzle::swizzle(
            (( input &  ( EQ_BIT32 | EQ_BIT31 | EQ_BIT22 | EQ_BIT21 | 
                          EQ_BIT12 | EQ_BIT11 | EQ_BIT2 | EQ_BIT1 ))        |
             (( input & ( EQ_BIT8  | EQ_BIT7 ))<<18 )                       |
             (( input & ( EQ_BIT24 | EQ_BIT23 | EQ_BIT14 | EQ_BIT13 ))<<6 ) |
             (( input & ( EQ_BIT16 | EQ_BIT15 | EQ_BIT6  | 
                          EQ_BIT5  | EQ_BIT4  | EQ_BIT3 )) <<12 )           |
             (( input & ( EQ_BIT28 | EQ_BIT27 | EQ_BIT26 | EQ_BIT25 ))>>18) |
             (( input & ( EQ_BIT18 | EQ_BIT17 ))>>12)                       |
             (( input & ( EQ_BIT30 | EQ_BIT29 | EQ_BIT20 | EQ_BIT19 |
                          EQ_BIT10 | EQ_BIT9 ))>>6 )),
                                  one, two, three, four );
    }
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        { assert( 0 ); }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
    {
        const uint32_t input = one + (two<<8) + (three<<16) + (four<<24);
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

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three )
        { assert( 0 ); return 0; }
};
 
class SwizzleUInt24
{
public:
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three, uint8_t& four )
        { assert( 0 ); }

    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three ) 
    {
        NoSwizzle::swizzle(
            (( input &  ( EQ_BIT24 | EQ_BIT23 | EQ_BIT22 | EQ_BIT13 | 
                          EQ_BIT12 | EQ_BIT3  | EQ_BIT2  | EQ_BIT1 )) |
             (( input & ( EQ_BIT16 | EQ_BIT15 | EQ_BIT14 )) << 5 )    | 
             (( input & ( EQ_BIT11 | EQ_BIT10 | EQ_BIT9  )) >> 5 )    |
             (( input & ( EQ_BIT8  | EQ_BIT7  | EQ_BIT6  | EQ_BIT5  |
                          EQ_BIT4  )) << 10 )                         |
             (( input & ( EQ_BIT21 | EQ_BIT20 | EQ_BIT19 | EQ_BIT18 |
                          EQ_BIT17 )) >> 10 )),
                                  one, two, three );
    }
    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
        { assert( 0 ); return 0; }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three )
    {
        const uint32_t input = one + (two<<8) + (three<<16);
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

}

void CompressorRLEYUV::compress( const void* const inData, 
                                const eq_uint64_t nPixels, const bool useAlpha,
                                const bool swizzle )
{
    if( useAlpha )
        if( swizzle )
            _nResults = _compress< uint32_t, uint8_t, SwizzleUInt32, UseAlpha >(
                            inData, nPixels, _results );
        else
            _nResults = _compress< uint32_t, uint8_t, NoSwizzle, UseAlpha >(
                            inData, nPixels, _results );
    else
        if( swizzle )
            _nResults = _compress< uint32_t, uint8_t, SwizzleUInt24, NoAlpha >(
                            inData, nPixels, _results );
        else
            _nResults = _compress< uint32_t, uint8_t, NoSwizzle, NoAlpha >(
                            inData, nPixels, _results );
}

void CompressorRLEYUV::decompress( const void* const* inData, 
                                   const eq_uint64_t* const inSizes, 
                                   const unsigned numInputs,
                                   void* const outData, 
                                   const eq_uint64_t nPixels,
                                   const bool useAlpha )
{
    if( useAlpha )
        _decompress< uint32_t, uint8_t, NoSwizzle, UseAlpha >( 
            inData, inSizes, numInputs, outData, nPixels );
    else
        _decompress< uint32_t, uint8_t, NoSwizzle, NoAlpha >(
            inData, inSizes, numInputs, outData, nPixels );
}

void CompressorDiffRLEYUV::decompress( const void* const* inData, 
                                      const eq_uint64_t* const inSizes, 
                                      const unsigned numInputs,
                                      void* const outData,
                                      const eq_uint64_t nPixels,
                                      const bool useAlpha )
{
    if( useAlpha )
        _decompress< uint32_t, uint8_t, SwizzleUInt32, UseAlpha >(
            inData, inSizes, numInputs, outData, nPixels );
    else
        _decompress< uint32_t, uint8_t, SwizzleUInt24, NoAlpha >(
            inData, inSizes, numInputs, outData, nPixels );
}

}
}
