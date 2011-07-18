
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
        
        one = ( input & 0xf ) | (( input &( EQ_BIT14 | EQ_BIT13 )) >> 8 ) |
                                (( input &( EQ_BIT24 | EQ_BIT23 )) >> 16 );

        two = (( input &( EQ_BIT26 | EQ_BIT25 )) >> 24 ) |
               ( input &( EQ_BIT6 | EQ_BIT5 )) |
              (( input &( EQ_BIT16 | EQ_BIT15 )) >> 12 ) |
              (( input &( EQ_BIT12 | EQ_BIT11 )) >> 4 );

        three = (( input &( EQ_BIT28 | EQ_BIT27 )) >> 22 ) |
                (( input &( EQ_BIT18 | EQ_BIT17 )) >> 14 ) |
                 ( input &( EQ_BIT8 | EQ_BIT7 )) |
                (( input &( EQ_BIT22 | EQ_BIT21 )) >> 20 );
        
        four  = (( input &( EQ_BIT32 | EQ_BIT31 | 
                            EQ_BIT30 | EQ_BIT29 )) >> 24 ) |
                (( input &( EQ_BIT20 | EQ_BIT19 )) >> 16 ) |
                (( input &( EQ_BIT10 | EQ_BIT9 )) >> 8 );
    }

    static inline void swizzle( const uint32_t input, uint8_t& one,
                                uint8_t& two, uint8_t& three )
        { assert( 0 ); }
    
    static inline uint32_t deswizzle( const uint8_t one, const uint8_t two,
                                      const uint8_t three, const uint8_t four )
    {
        return ( one & 0xf ) | 
               (( one &( EQ_BIT5 | EQ_BIT6 )) << 8 ) |
               (( one &( EQ_BIT7 | EQ_BIT8 )) << 16 ) |
               (( two &( EQ_BIT1 | EQ_BIT2 )) << 24 ) |
               (( two &( EQ_BIT3 | EQ_BIT4 )) << 12 ) |
                ( two & 0x30 ) |
               (( two &( EQ_BIT7 | EQ_BIT8 )) << 4 )|
               ( three & 0xc0) |
               (( three &( EQ_BIT6 | EQ_BIT5 )) << 22 ) |
               (( three &( EQ_BIT4 | EQ_BIT3 )) << 14 ) |
               (( three &( EQ_BIT2 | EQ_BIT1 )) << 20 )|
               (( four & 0xf0 ) << 24 ) |
               (( four &( EQ_BIT4 | EQ_BIT3 )) << 16 ) |
               (( four &( EQ_BIT2 | EQ_BIT1 )) << 8 );
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
