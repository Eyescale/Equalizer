
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
REGISTER_ENGINE( CompressorRLE4B, RGBA, RGBA, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorRLE4B, BGRA, BGRA, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorRLE4B, RGBA_UINT_8_8_8_8_REV,    \
                 RGBA_UINT_8_8_8_8_REV, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorRLE4B, BGRA_UINT_8_8_8_8_REV,    \
                 BGRA_UINT_8_8_8_8_REV, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorRLE4B, RGB10_A2, RGB10_A2, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorRLE4B, BGR10_A2, BGR10_A2, 1., 0.59, 1., true );
REGISTER_ENGINE( CompressorRLE4B, DEPTH_UNSIGNED_INT, \
                 DEPTH_UNSIGNED_INT, 1., 0.59, 1., false );

REGISTER_ENGINE( CompressorDiffRLE4B, DIFF_RGBA, RGBA, 1., .5, 1.1, true );
REGISTER_ENGINE( CompressorDiffRLE4B, DIFF_BGRA, BGRA, 1., .5, 1.1, true );
REGISTER_ENGINE( CompressorDiffRLE4B, DIFF_RGBA_UINT_8_8_8_8_REV,   \
                 RGBA_UINT_8_8_8_8_REV, 1., .5, 1.1, true );
REGISTER_ENGINE( CompressorDiffRLE4B, DIFF_BGRA_UINT_8_8_8_8_REV, \
                 BGRA_UINT_8_8_8_8_REV, 1., .5, 1.1, true );

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
            (( input &  ( LB_BIT32 | LB_BIT31 | LB_BIT22 | LB_BIT21 | 
                          LB_BIT12 | LB_BIT11 | LB_BIT2 | LB_BIT1 ))        |
             (( input & ( LB_BIT8  | LB_BIT7 ))<<18 )                       |
             (( input & ( LB_BIT24 | LB_BIT23 | LB_BIT14 | LB_BIT13 ))<<6 ) |
             (( input & ( LB_BIT16 | LB_BIT15 | LB_BIT6  | 
                          LB_BIT5  | LB_BIT4  | LB_BIT3 )) <<12 )           |
             (( input & ( LB_BIT28 | LB_BIT27 | LB_BIT26 | LB_BIT25 ))>>18) |
             (( input & ( LB_BIT18 | LB_BIT17 ))>>12)                       |
             (( input & ( LB_BIT30 | LB_BIT29 | LB_BIT20 | LB_BIT19 |
                          LB_BIT10 | LB_BIT9 ))>>6 )),
                                  one, two, three, four );
    }
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        { assert( 0 ); }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
    {
        const uint32_t input = one + (two<<8) + (three<<16) + (four<<24);
        return ((  input & ( LB_BIT32 | LB_BIT31 | LB_BIT22 | LB_BIT21 | 
                             LB_BIT11 | LB_BIT12 | LB_BIT2 | LB_BIT1 ))        |
                (( input & ( LB_BIT26 | LB_BIT25 )) >>18 )                     |
                (( input & ( LB_BIT30 | LB_BIT29 | LB_BIT20 | LB_BIT19 ))>>6 ) |
                (( input & ( LB_BIT28 | LB_BIT27 | LB_BIT18 | 
                             LB_BIT17 | LB_BIT16 |LB_BIT15 ))>>12 )            |
                (( input & ( LB_BIT10 | LB_BIT9  | LB_BIT8  | LB_BIT7 ))<<18 ) |
                (( input & ( LB_BIT6  | LB_BIT5 ))<<12 )                       |
                (( input & ( LB_BIT24 | LB_BIT23 | LB_BIT14 | 
                             LB_BIT13 | LB_BIT4  | LB_BIT3 ))<<6 ));
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
            (( input &  ( LB_BIT24 | LB_BIT23 | LB_BIT22 | LB_BIT13 | 
                          LB_BIT12 | LB_BIT3  | LB_BIT2  | LB_BIT1 )) |
             (( input & ( LB_BIT16 | LB_BIT15 | LB_BIT14 )) << 5 )    | 
             (( input & ( LB_BIT11 | LB_BIT10 | LB_BIT9  )) >> 5 )    |
             (( input & ( LB_BIT8  | LB_BIT7  | LB_BIT6  | LB_BIT5  |
                          LB_BIT4  )) << 10 )                         |
             (( input & ( LB_BIT21 | LB_BIT20 | LB_BIT19 | LB_BIT18 |
                          LB_BIT17 )) >> 10 )),
                                  one, two, three );
    }
    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
        { assert( 0 ); return 0; }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three )
    {
        const uint32_t input = one + (two<<8) + (three<<16);
        return ((  input & ( LB_BIT24 | LB_BIT23 | LB_BIT22 | LB_BIT13 | 
                             LB_BIT12 | LB_BIT3  | LB_BIT2  | LB_BIT1 )) |
                (( input & ( LB_BIT21 | LB_BIT20 | LB_BIT19 ))>>5 )      |
                (( input & ( LB_BIT6  | LB_BIT5  | LB_BIT4 ))<<5 )       | 
                (( input & ( LB_BIT18 | LB_BIT17 | LB_BIT16 | 
                             LB_BIT15 | LB_BIT14 ))>>10 )                |
                (( input & ( LB_BIT11 | LB_BIT10 | LB_BIT9  | 
                             LB_BIT8  | LB_BIT7 ))<<10 ));
    }
};  

}

void CompressorRLE4B::compress( const void* const inData, 
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

void CompressorRLE4B::decompress( const void* const* inData, 
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

void CompressorDiffRLE4B::decompress( const void* const* inData, 
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
