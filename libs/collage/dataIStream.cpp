
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009-2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "dataIStream.h"

#include "log.h"
#include "node.h"

#include "base/cpuCompressor.h" // internal header
#include <co/base/debug.h>
#include <co/plugins/compressor.h>

#include <string.h>

namespace co
{

DataIStream::DataIStream()
        : _input( 0 )
        , _inputSize( 0 )
        , _position( 0 )
        , _decompressor( new co::base::CPUCompressor )
{}

DataIStream::DataIStream( const DataIStream& )
        : _input( 0 )
        , _inputSize( 0 )
        , _position( 0 )
        , _decompressor( new co::base::CPUCompressor )
{}

DataIStream::~DataIStream()
{
    reset();
    delete _decompressor;
}

void DataIStream::reset()
{
    _input     = 0;
    _inputSize = 0;
    _position  = 0;
}

void DataIStream::read( void* data, uint64_t size )
{
    if( !_checkBuffer( ))
    {
        EQUNREACHABLE;
        EQERROR << "No more input data" << std::endl;
        return;
    }

    EQASSERT( _input );
    
    if( _position + size > _inputSize )
    {
        EQERROR << "Not enough data in input buffer: need " << size 
                << " bytes, " << _inputSize - _position << " left "<< std::endl;
        EQUNREACHABLE;
        // TODO: Allow reads which are asymmetric to writes by reading from
        // multiple blocks here?
        return;
    }

    memcpy( data, _input + _position, size );
    _position += size;
}

const void* DataIStream::getRemainingBuffer()
{
    if( !_checkBuffer( ))
        return 0;

    return _input + _position;
}

uint64_t DataIStream::getRemainingBufferSize()
{
    if( !_checkBuffer( ))
        return 0;

    return _inputSize - _position;
}

void DataIStream::advanceBuffer( const uint64_t offset )
{
    EQASSERT( _position + offset <= _inputSize );
    _position += offset;
}

bool DataIStream::_checkBuffer()
{
    while( _position >= _inputSize )
    {
        uint32_t compressor = EQ_COMPRESSOR_NONE;
        uint32_t nChunks = 0;
        const void* chunkData = 0;
        
        if( !getNextBuffer( &compressor, &nChunks, &chunkData, &_inputSize ))
            return false;

        _input = _decompress( chunkData, compressor, nChunks, _inputSize );
        _position = 0;
    }
    return true;
}

const uint8_t* DataIStream::_decompress( const void* data, const uint32_t name,
                                         const uint32_t nChunks,
                                         const uint64_t dataSize )
{
    const uint8_t* src = reinterpret_cast< const uint8_t* >( data );
    if( name == EQ_COMPRESSOR_NONE )
        return src;

    EQASSERT( name > EQ_COMPRESSOR_NONE );
    _data.resize( dataSize );

    if ( !_decompressor->isValid( name ) )
        _decompressor->initDecompressor( name );

    uint64_t outDim[2] = { 0, dataSize };
    uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                alloca( nChunks * sizeof( uint64_t )));
    void** chunks = static_cast< void ** >( 
                                alloca( nChunks * sizeof( void* )));
    
    for( uint32_t i = 0; i < nChunks; ++i )
    {
        const uint64_t size = *reinterpret_cast< const uint64_t* >( src );
        chunkSizes[ i ] = size;
        src += sizeof( uint64_t );

        // The plugin API uses non-const source buffers for in-place operations
        chunks[ i ] = const_cast< uint8_t* >( src );
        src += size;
    }

    _decompressor->decompress( chunks, chunkSizes, nChunks, 
                               _data.getData(), outDim );
    return _data.getData();
}

}

