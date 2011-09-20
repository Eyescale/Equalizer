
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "dataOStream.h"

#include "connectionDescription.h"
#include "connections.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "types.h"

#include "base/cpuCompressor.h" // internal header
#include <co/base/global.h>

#ifdef EQ_INSTRUMENT_DATAOSTREAM
#  include <co/base/clock.h>
#endif

namespace co
{

#ifdef EQ_INSTRUMENT_DATAOSTREAM
base::a_int32_t nBytes;
base::a_int32_t nBytesIn;
base::a_int32_t nBytesOut;
CO_API base::a_int32_t nBytesSaved;
CO_API base::a_int32_t nBytesSent;
base::a_int32_t compressionTime;
#endif

DataOStream::DataOStream()
        : _compressorState( STATE_UNCOMPRESSED )
        , _bufferStart( 0 )
        , _dataSize( 0 )
        , _compressor( new base::CPUCompressor )
        , _enabled( false )
        , _dataSent( false )
        , _save( false )
{
}

DataOStream::~DataOStream()
{
    // Can't call disable() from destructor since it uses virtual functions
    EQASSERT( !_enabled );
    delete _compressor;
}

void DataOStream::_initCompressor( const uint32_t compressor )
{
    EQCHECK( _compressor->base::Compressor::initCompressor( compressor ));
    EQ_TS_RESET( _compressor->_thread );
}

void DataOStream::_enable()
{
    EQASSERT( !_enabled );
    EQASSERT( _save || !_connections.empty( ));
    _compressorState = STATE_UNCOMPRESSED;
    _bufferStart = 0;
    _dataSent    = false;
    _enabled     = true;
    _buffer.setSize( 0 );
#ifndef CO_AGGRESSIVE_CACHING
    _buffer.reserve( _dataSize );
    _dataSize    = 0;
#endif
}

void DataOStream::_setupConnections( const Nodes& receivers )
{
    gatherConnections( receivers, _connections );
}

void DataOStream::_setupConnection( NodePtr node, const bool useMulticast )
{
    EQASSERT( _connections.empty( ));
    ConnectionPtr connection = useMulticast ? node->getMulticast() : 0;
    if( !connection )
        connection = node->getConnection();
        
    _connections.push_back( connection );
}

void DataOStream::_resend( )
{
    EQASSERT( !_enabled );
    EQASSERT( !_connections.empty( ));
    EQASSERT( _save );
    
    _compress( _buffer.getData(), _dataSize, STATE_COMPLETE );
    sendData( _buffer.getData(), _dataSize, true );
    _connections.clear();
}

void DataOStream::disable()
{
    if( !_enabled )
        return;

    if( _dataSent )
    {
        _dataSize = _buffer.getSize();
        if( !_connections.empty( ))
        {
            void* ptr = _buffer.getData() + _bufferStart;
            const uint64_t size = _buffer.getSize() - _bufferStart;

            if( size == 0 && _bufferStart == _dataSize &&
                _compressorState == STATE_PARTIAL )
            {
                // OPT: all data has been sent in one compressed chunk
                _compressorState = STATE_COMPLETE;
#ifndef CO_AGGRESSIVE_CACHING
                _buffer.clear();
#endif
            }
            else
            {
                _compressorState = STATE_UNCOMPRESSED;
                _compress( ptr, size, STATE_PARTIAL );
            }

            sendData( ptr, size, true ); // send always to finalize istream
        }
    }
    else if( _buffer.getSize() > 0 )
    {
        _dataSize = _buffer.getSize();
        _dataSent = true;

        EQASSERT( _bufferStart == 0 );
        if( !_connections.empty( ))
        {
            _compressorState = STATE_UNCOMPRESSED;
            _compress( _buffer.getData(), _dataSize, STATE_COMPLETE );
            sendData( _buffer.getData(), _dataSize, true );
        }
    }

    _enabled = false;
    _connections.clear();
#ifndef CO_AGGRESSIVE_CACHING
    if( !_save )
        _buffer.clear();
#endif
}

void DataOStream::enableSave()
{
    EQASSERTINFO( !_enabled || ( !_dataSent && _buffer.getSize() == 0 ),
                  "Can't enable saving after data has been written" );
    _save = true;
}

void DataOStream::disableSave()
{
    EQASSERTINFO( !_enabled || (!_dataSent && _buffer.getSize() == 0 ),
                  "Can't disable saving after data has been written" );
    _save = false;
}

void DataOStream::write( const void* data, uint64_t size )
{
    EQASSERT( _enabled );
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytes += size;
    if( compressionTime > 100000 )
        EQWARN << *this << std::endl;
#endif    

    if( _buffer.getSize() - _bufferStart > Global::getObjectBufferSize( ))
        _flush();
    _buffer.append( static_cast< const uint8_t* >( data ), size );
}

void DataOStream::_flush()
{
    EQASSERT( _enabled );
    if( !_connections.empty( ))
    {
        void* ptr = _buffer.getData() + _bufferStart;
        const uint64_t size = _buffer.getSize() - _bufferStart;

        _compressorState = STATE_UNCOMPRESSED;
        _compress( ptr, size, STATE_PARTIAL );
        sendData( ptr, size, false );
    }
    _dataSent = true;
    _resetBuffer();
}

void DataOStream::reset()
{
    _resetBuffer();
}

void DataOStream::_resetBuffer()
{
    _compressorState = STATE_UNCOMPRESSED;
    if( _save )
        _bufferStart = _buffer.getSize();
    else
    {
        _bufferStart = 0;
        _buffer.setSize( 0 );
    }
}

void DataOStream::_compress( void* src, const uint64_t size,
                             const CompressorState result )
{
    if( _compressorState == result || _compressorState == STATE_UNCOMPRESSIBLE )
        return;

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesIn += size;
#endif
    if( !_compressor->isValid( _compressor->getName( )) || size == 0 )
    {
        _compressorState = STATE_UNCOMPRESSED;
        return;
    }
    
    const uint64_t inDims[2] = { 0, size };

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    base::Clock clock;
#endif
    _compressor->compress( src, inDims );
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    compressionTime += uint32_t( clock.getTimef() * 1000.f );
#endif

    const uint32_t nChunks = _compressor->getNumResults();
    uint64_t compressedSize = 0;
    EQASSERT( nChunks > 0 );

    for( uint32_t i = 0; i < nChunks; ++i )
    {
        void* chunk;
        uint64_t chunkSize;

        _compressor->getResult( i, &chunk, &chunkSize );
        compressedSize += chunkSize;
    }
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesOut += compressedSize;
#endif

    if( compressedSize >= size )
    {
        _compressorState = STATE_UNCOMPRESSIBLE;
#ifndef CO_AGGRESSIVE_CACHING
        const uint32_t name = _compressor->getName();
        _compressor->reset();
        EQCHECK( _compressor->base::Compressor::initCompressor( name ));

        if( result == STATE_COMPLETE )
            _buffer.pack();
#endif
        return;
    }

    _compressorState = result;
#ifndef CO_AGGRESSIVE_CACHING
    if( result == STATE_COMPLETE )
    {
        EQASSERT( _buffer.getSize() == _dataSize );
        _buffer.clear();
    }
#endif
}

uint64_t DataOStream::_getCompressedData( void** chunks, uint64_t* chunkSizes )
    const
{    
    EQASSERT( _compressorState != STATE_UNCOMPRESSED &&
              _compressorState != STATE_UNCOMPRESSIBLE );

    const uint32_t nChunks = _compressor->getNumResults( );
    EQASSERT( nChunks > 0 );

    uint64_t dataSize = 0;
    for ( uint32_t i = 0; i < nChunks; i++ )
    {
        _compressor->getResult( i, &chunks[i], &chunkSizes[i] );
        dataSize += chunkSizes[i];
    }

    return dataSize;
}

std::ostream& operator << ( std::ostream& os,
                            const DataOStream& dataOStream )
{
    os << "DataOStream "
#ifdef EQ_INSTRUMENT_DATAOSTREAM
       << "compressed " << nBytesIn << " -> " << nBytesOut << " of " << nBytes
       << " in " << compressionTime/1000 << "ms, saved " << nBytesSaved
       << " of " << nBytesSent << " brutto sent";

    nBytes = 0;
    nBytesIn = 0;
    nBytesOut = 0;
    nBytesSaved = 0;
    nBytesSent = 0;
    compressionTime = 0;
#else
       << "@" << (void*)&dataOStream;
#endif
    return os;
}

}
