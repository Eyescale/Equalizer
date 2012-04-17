
/* Copyright (c) 2009-2010, Sarah Amsellem <sarah.amsellem@gmail.com> 
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

#include "compressorRLE565.h"

namespace
{
static const uint8_t _rleMarker = 0x17; // just a random number
}

#include "compressorRLE.ipp"

namespace co
{
namespace plugin
{
namespace
{
REGISTER_ENGINE( CompressorRLE565, DIFF_565_RGBA, RGBA, .7, .1, 1.1, true );
REGISTER_ENGINE( CompressorRLE565, DIFF_565_BGRA, BGRA, .7, .1, 1.1, true );
REGISTER_ENGINE( CompressorRLE565, DIFF_565_RGBA_UINT_8_8_8_8_REV,  \
                 RGBA_UINT_8_8_8_8_REV, .7, .1, 1.1, true );
REGISTER_ENGINE( CompressorRLE565, DIFF_565_BGRA_UINT_8_8_8_8_REV,  \
                 BGRA_UINT_8_8_8_8_REV, .7, .1, 1.1, true );

class NoSwizzle
{
public:
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three, uint8_t& four ) 
        {
            one   = input & 0xff;
            two   = ( input & 0xff00 ) >> 8;
            three = ( input & 0xff0000 ) >> 16;
            four  = ( input & 0xff000000 ) >> 24;
        }

    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        {
            one   = input & 0xff;
            two   = ( input & 0xff00 ) >> 8;
            three = ( input & 0xff0000 ) >> 16;
        }
};

class SwizzleUInt32
{
public:
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three, uint8_t& four ) 
    {
         NoSwizzle::swizzle(
           ((( input & ( LB_BIT6  | LB_BIT5  | LB_BIT4 )) >> 3 )           |
            (( input & ( LB_BIT14 | LB_BIT13 | LB_BIT12 )) >> 8 )          |
            (( input & ( LB_BIT8  | LB_BIT7 )) << 5 )                      |
            (( input & ( LB_BIT24 | LB_BIT23 | LB_BIT22 | 
                         LB_BIT21 | LB_BIT20 )) >> 13 )                    |
            (( input & ( LB_BIT16 | LB_BIT15 )) >> 1 )                     |
            (( input &   LB_BIT32 ) >> 16 )),
                 one, two, three, four );
    }

    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        { assert( 0 ); }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
    {
        const uint32_t input = one + ( two << 8 ) + ( three << 16 ) +
                               ( four << 24 );
        return (((( input & ( LB_BIT3 | LB_BIT2 | LB_BIT1 )) << 3 )           |
                 (( input & ( LB_BIT6 | LB_BIT5 | LB_BIT4 )) << 8 )           |
                 (( input & ( LB_BIT13 | LB_BIT12 )) >> 5 )                   |
                 (( input & ( LB_BIT11 | LB_BIT10 | LB_BIT9 |
                              LB_BIT8 | LB_BIT7 )) << 13 )                    |
                 (( input & ( LB_BIT15 | LB_BIT14 )) << 1 )                   |
                 (( input &   LB_BIT16 ) << 16 )))
                          | 0x3f040404;
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
           ((( input & ( LB_BIT6  | LB_BIT5  | LB_BIT4 )) >> 3 )           |
                (( input & ( LB_BIT13 | LB_BIT12 | LB_BIT11 )) >> 7 )          |
                (( input & ( LB_BIT8  | LB_BIT7 )) << 5 )                      |
                (( input & ( LB_BIT24 | LB_BIT23 | LB_BIT22 |
                             LB_BIT21 | LB_BIT20 )) >> 13 )                    |
                 ( input & ( LB_BIT16 | LB_BIT15 | LB_BIT14 ))),
                   one, two, three );
    }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
        { assert( 0 ); return 0; }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three )
    {
        const uint32_t input = one + ( two << 8 ) + ( three << 16 );
        return (((( input & ( LB_BIT3 | LB_BIT2 | LB_BIT1 )) << 3 )           |
                 (( input & ( LB_BIT6 | LB_BIT5 | LB_BIT4 )) << 7 )           |
                 (( input & ( LB_BIT13 | LB_BIT12 )) >> 5 )                   |
                 (( input & ( LB_BIT11 | LB_BIT10 | LB_BIT9 |
                              LB_BIT8  | LB_BIT7 )) << 13 )                   |
                  ( input & ( LB_BIT16 | LB_BIT15 | LB_BIT14 ))) 
                          & 0xf8fcf8 )
                          | 0x020202;
    }
};  

}

void CompressorRLE565::compress( const void* const inData,
                                 const eq_uint64_t nPixels,
                                 const bool useAlpha )
{
    if( useAlpha )
        _nResults = _compress< uint32_t, uint8_t, SwizzleUInt32, UseAlpha >(
                        inData, nPixels, _results );
    else
        _nResults = _compress< uint32_t, uint8_t, SwizzleUInt24, NoAlpha >(
                        inData, nPixels, _results );
}

void CompressorRLE565::decompress( const void* const* inData, 
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
