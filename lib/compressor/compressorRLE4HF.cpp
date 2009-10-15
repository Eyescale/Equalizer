
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
 
#include "compressorRLE4HF.h"

namespace
{
// NaN
static const uint16_t _rleMarker = 0xffff; 
}

#include "compressorRLE.ipp"

namespace eq
{
namespace plugin
{

namespace
{
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
}

void CompressorRLE4HF::compress( const void* const inData, 
                                 const eq_uint64_t nPixels, const bool useAlpha,
                                 const bool swizzle )
{
    assert( !swizzle );
    if( useAlpha )
        _nResults = _compress< uint64_t, uint16_t, NoSwizzle, UseAlpha >(
                        inData, nPixels, useAlpha, swizzle, _results );
    else
        _nResults = _compress< uint64_t, uint16_t, NoSwizzle, NoAlpha >(
                        inData, nPixels, useAlpha, swizzle, _results );
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

}
}
