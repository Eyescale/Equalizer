
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/base/debug.h>
#include <eq/base/compressor.h>

#include <string.h>

namespace eq
{
namespace net
{
DataIStream::DataIStream()
        : _input( 0 )
        , _inputSize( 0 )
        , _position( 0 )
        , _decompressor( 0 )
{}

DataIStream::DataIStream( const DataIStream& from )
        : DataStream( from )
        , _input( 0 )
        , _inputSize( 0 )
        , _position( 0 )
        , _decompressor( 0 )
{}

DataIStream::~DataIStream()
{
    reset();
    
    base::Compressor* plugin = _getCompressorPlugin();
    if( plugin && _decompressor )
        plugin->deleteDecompressor( _decompressor );
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
        if( !getNextBuffer( &_input, &_inputSize ))
            return false;
        _position = 0;
    }
    return true;
}

void DataIStream::_decompress( const uint8_t* src, const uint8_t** dst, 
                               const uint32_t name, const uint32_t nChunks,
                               const uint64_t dataSize )
{
    _data.resize( dataSize );
    *dst = _data.getData();
        
    EQASSERT( name > EQ_COMPRESSOR_NONE );

    _initDecompressor( name );

    uint64_t outDim[2] = { 0, dataSize };
    uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                alloca( nChunks * sizeof( uint64_t )));
    void** chunks = static_cast< void ** >( 
                                alloca( nChunks * sizeof( void* )));
    
    for( size_t i = 0; i < nChunks; ++i )
    {
        const uint64_t size = *reinterpret_cast< const uint64_t* >( src );
        chunkSizes[ i ] = size;
        src += sizeof( uint64_t );

        // The plugin API uses non-const source buffers for in-place operations
        chunks[ i ] = const_cast< uint8_t* >( src );
        src += size;
    }

    base::Compressor* plugin = _getCompressorPlugin();
    EQASSERT( plugin );
    plugin->decompress( _decompressor, name, chunks, chunkSizes, nChunks, 
                        _data.getData(), outDim, EQ_COMPRESSOR_DATA_1D );

}

void DataIStream::_initDecompressor( const uint32_t name )
{
    if( _getCompressorName() != name && _decompressor )
    {
        base::Compressor* plugin = _getCompressorPlugin();
        EQASSERT( plugin );
        plugin->deleteDecompressor( _decompressor );
        _decompressor = 0;
    }

    base::Compressor* plugin = _initCompressorPlugin( name );

    if( !_decompressor )
    {
        EQASSERT( plugin );
        _decompressor = plugin->newDecompressor( name );
    }
}

}
}

