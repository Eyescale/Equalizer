
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "compressorFastLZ.h"

#include "fastlz/fastlz.h"

namespace co
{
namespace plugin
{
namespace
{
static void _getInfo( EqCompressorInfo* const info )
{
    info->version = EQ_COMPRESSOR_VERSION;
    info->capabilities = EQ_COMPRESSOR_DATA_1D | EQ_COMPRESSOR_DATA_2D;
    info->quality = 1.f;
    info->ratio   = .55f;
    info->speed   = .76f;
    info->name = EQ_COMPRESSOR_FASTLZ_BYTE;
    info->tokenType = EQ_COMPRESSOR_DATATYPE_BYTE;
}

static bool _register()
{
    Compressor::registerEngine(
        Compressor::Functions( EQ_COMPRESSOR_FASTLZ_BYTE,
                               _getInfo,
                               CompressorFastLZ::getNewCompressor,
                               CompressorFastLZ::getNewDecompressor,
                               CompressorFastLZ::decompress, 0 ));
    return true;
}

static const bool _initialized = _register();
}

void CompressorFastLZ::compress( const void* const inData,
                              const eq_uint64_t nPixels, const bool useAlpha )
{
    _nResults = 1;
    if( _results.size() < _nResults )
        _results.push_back( new co::plugin::Compressor::Result );
    const eq_uint64_t maxSize = eq_uint64_t( float( nPixels ) * 1.1f ) + 66;
    _results[0]->reserve( maxSize );

    const int size = fastlz_compress( inData, nPixels, _results[0]->getData( ));
    _results[0]->resize( size );
    assert( size != 0 );
}

void CompressorFastLZ::decompress( const void* const* inData,
                                const eq_uint64_t* const inSizes,
                                const unsigned nInputs,
                                void* const outData,
                                const eq_uint64_t nPixels,
                                const bool useAlpha )
{
    if( nInputs == 0 )
        return;

    fastlz_decompress( inData[0], inSizes[0], outData, nPixels );
}

}
}
