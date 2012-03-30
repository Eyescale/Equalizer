
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "compressorRLE10A2.h"

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
REGISTER_ENGINE( CompressorRLE10A2, DIFF_BGR10_A2, BGR10_A2, \
                 1., 0.57, 1., true );

class SwizzleUInt32
{
public:
    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three, uint8_t& four ) 
    {
        
        one = ( input & 0xf ) | (( input &( LB_BIT14 | LB_BIT13 )) >> 8 ) |
                                (( input &( LB_BIT24 | LB_BIT23 )) >> 16 );

        two = (( input &( LB_BIT26 | LB_BIT25 )) >> 24 ) |
               ( input &( LB_BIT6 | LB_BIT5 )) |
              (( input &( LB_BIT16 | LB_BIT15 )) >> 12 ) |
              (( input &( LB_BIT12 | LB_BIT11 )) >> 4 );

        three = (( input &( LB_BIT28 | LB_BIT27 )) >> 22 ) |
                (( input &( LB_BIT18 | LB_BIT17 )) >> 14 ) |
                 ( input &( LB_BIT8 | LB_BIT7 )) |
                (( input &( LB_BIT22 | LB_BIT21 )) >> 20 );
        
        four  = (( input &( LB_BIT32 | LB_BIT31 | 
                            LB_BIT30 | LB_BIT29 )) >> 24 ) |
                (( input &( LB_BIT20 | LB_BIT19 )) >> 16 ) |
                (( input &( LB_BIT10 | LB_BIT9 )) >> 8 );
    }

    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        { assert( 0 ); }
    
    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
    {
        return ( one & 0xf ) | 
               (( one &( LB_BIT5 | LB_BIT6 )) << 8 ) |
               (( one &( LB_BIT7 | LB_BIT8 )) << 16 ) |
               (( two &( LB_BIT1 | LB_BIT2 )) << 24 ) |
               (( two &( LB_BIT3 | LB_BIT4 )) << 12 ) |
                ( two & 0x30 ) |
               (( two &( LB_BIT7 | LB_BIT8 )) << 4 )|
               ( three & 0xc0) |
               (( three &( LB_BIT6 | LB_BIT5 )) << 22 ) |
               (( three &( LB_BIT4 | LB_BIT3 )) << 14 ) |
               (( three &( LB_BIT2 | LB_BIT1 )) << 20 )|
               (( four & 0xf0 ) << 24 ) |
               (( four &( LB_BIT4 | LB_BIT3 )) << 16 ) |
               (( four &( LB_BIT2 | LB_BIT1 )) << 8 );
    }

    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                  const uint8_t three )
        { assert( 0 ); return 0; }
};
 

}

void CompressorRLE10A2::compress( const void* const inData, 
                                const eq_uint64_t nPixels, const bool useAlpha,
                                const bool swizzle )
{
        _nResults = _compress< uint32_t, uint8_t, SwizzleUInt32, UseAlpha >(
                        inData, nPixels, _results );

}

void CompressorRLE10A2::decompress( const void* const* inData, 
                                      const eq_uint64_t* const inSizes, 
                                      const unsigned numInputs,
                                      void* const outData,
                                      const eq_uint64_t nPixels,
                                      const bool useAlpha )
{
    _decompress< uint32_t, uint8_t, SwizzleUInt32, UseAlpha >(
        inData, inSizes, numInputs, outData, nPixels );
}

}
}
