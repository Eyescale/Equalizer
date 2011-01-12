
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
#include "global.h"
#include "log.h"
#include "node.h"
#include "types.h"

#include "base/cpuCompressor.h" // internal header
#include <co/base/global.h>

//#define EQ_INSTRUMENT_DATAOSTREAM
#ifdef EQ_INSTRUMENT_DATAOSTREAM
#  include <co/base/clock.h>
#endif

namespace co
{

namespace
{
#ifdef EQ_INSTRUMENT_DATAOSTREAM
co::base::a_int32_t nBytes;
co::base::a_int32_t nBytesProcessed;
co::base::a_int32_t nBytesCompressed;
co::base::a_int32_t nBytesCompressedSent;
co::base::a_int32_t nBytesSent;
co::base::a_int32_t compressionTime;
#endif
}

DataOStream::DataOStream()
        : _compressorState( NOT_COMPRESSED )
        , _bufferStart( 0 )
        , _compressor( new co::base::CPUCompressor )
        , _enabled( false )
        , _dataSent( false )
        , _save( false )
{
    _compressor->initCompressor( EQ_COMPRESSOR_DATATYPE_BYTE, 1.f );
    EQVERB << "Using byte compressor " << _compressor->getName() << std::endl;
}

DataOStream::~DataOStream()
{
    // Can't call disable() from destructor since it uses virtual functions
    EQASSERT( !_enabled );
    delete _compressor;
}

void DataOStream::enable( const Nodes& receivers )
{
    _setupConnections( receivers );
#if 0
    EQLOG( LOG_OBJECTS )
        << "Enabled " << typeid( *this ).name() << " with " << mcSet.size()
        << "/" << _connections.size() << " multicast connections" << std::endl;
#endif
    enable();
}

void DataOStream::enable( NodePtr node, const bool useMulticast )
{
    _setupConnection( node, useMulticast );
    enable();
}

void DataOStream::enable()
{
    EQASSERT( !_enabled );
    EQASSERT( _save || !_connections.empty( ));
    _compressorState = NOT_COMPRESSED;
    _bufferStart = 0;
    _dataSent    = false;
    _enabled     = true;
    _buffer.setSize( 0 );
}

void DataOStream::_setupConnections( const Nodes& receivers )
{
    EQASSERT( _connections.empty( ));

    const bool useMulticast = receivers.size() > 1;
    ConnectionDescriptions mcSet;

    for( Nodes::const_iterator i = receivers.begin(); i != receivers.end(); ++i)
    {
        NodePtr       node       = *i;
        ConnectionPtr connection = useMulticast ? node->getMulticast() : 0;
        
        if( connection.isValid( ))
        {
            ConnectionDescriptionPtr desc = connection->getDescription();
            if( std::find( mcSet.begin(), mcSet.end(), desc ) != mcSet.end( ))
                // already added by another node
                continue;
            mcSet.push_back( desc );
        }
        else
            connection = node->getConnection();
        
        _connections.push_back( connection );
    }
}

void DataOStream::_setupConnection( NodePtr node, const bool useMulticast )
{
    EQASSERT( _connections.empty( ));
    ConnectionPtr connection = useMulticast ? node->getMulticast() : 0;
    if( !connection )
        connection = node->getConnection();
        
    _connections.push_back( connection );
}

void DataOStream::resend( const Nodes& receivers )
{
    _setupConnections( receivers );
    _resend();
}

void DataOStream::resend( NodePtr node, const bool useMulticast )
{
    _setupConnection( node, useMulticast );
    _resend( );
}

void DataOStream::_resend( )
{
    EQASSERT( !_enabled );
    EQASSERT( !_connections.empty( ));
    EQASSERT( _save );
    
    if( _compressorState != FULL_COMPRESSED )
    {
        _compress( _buffer.getData(), _buffer.getSize() );
        _compressorState = FULL_COMPRESSED;
    }
    _sendFooter( _buffer.getData(), _buffer.getSize() );
    _connections.clear();
}

void DataOStream::disable()
{
    if( !_enabled )
        return;

    if( _dataSent )
    {
        if( !_connections.empty( ))
        {
            const void* ptr = _buffer.getData() + _bufferStart;
            const uint64_t size = _buffer.getSize() - _bufferStart;
            if( _compressorState != PARTIAL_COMPRESSED )
            {
                _compress( ptr, size );
                _compressorState = PARTIAL_COMPRESSED;
            }
            _sendFooter( ptr, size );
        }
        _dataSent = true;
    }
    else if( _buffer.getSize() > 0 )
    {
        EQASSERT( _bufferStart == 0 );
        if( !_connections.empty( ))
        {
            if( _compressorState != FULL_COMPRESSED )
            {
                _compress( _buffer.getData(), _buffer.getSize() );
                _compressorState = FULL_COMPRESSED;
            }
            _sendFooter( _buffer.getData(), _buffer.getSize( ));
        }
        _dataSent = true;
    }

    reset();
    _enabled = false;
    _connections.clear();
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
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    if( (nBytes += size) > EQ_10MB )
        EQINFO << *this << std::endl;
#endif    

    if( _buffer.getSize() - _bufferStart > Global::getObjectBufferSize( ))
        _flush();

    EQASSERT( _enabled );
    _compressorState = NOT_COMPRESSED;
    _buffer.append( static_cast< const uint8_t* >( data ), size );
}

void DataOStream::_flush()
{
    EQASSERT( _enabled );
    const void* ptr = _buffer.getData() + _bufferStart;
    const uint64_t size = _buffer.getSize() - _bufferStart;

    EQASSERT( _compressorState == NOT_COMPRESSED );
    if( _compressorState != PARTIAL_COMPRESSED )
    {
        _compress( ptr, size );
        _compressorState = PARTIAL_COMPRESSED;
    }

    _sendData( ptr, size );
    _resetBuffer();
}

void DataOStream::reset()
{
    _resetBuffer();
}

void DataOStream::_resetBuffer()
{
    _compressorState = NOT_COMPRESSED;
    if( _save )
        _bufferStart = _buffer.getSize();
    else
    {
        _bufferStart = 0;
        _buffer.setSize( 0 );
    }
}

void DataOStream::_sendData( const void* data, const uint64_t size )
{
    EQASSERT( _enabled );
    if( size == 0 )
        return;
    _dataSent = true;

    if( _connections.empty( ))
        return;

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesSent += size;
#endif

    if( _compressorState != NOT_COMPRESSED )
    {
        const uint32_t nChunks = _compressor->getNumResults();
        uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                   alloca( nChunks * sizeof( uint64_t )));
        void** chunks = static_cast< void ** >( 
                            alloca( nChunks * sizeof( void* )));
        const uint64_t compressedSize = _getCompressedData( chunks, chunkSizes);

        if( compressedSize < size )
        {
#ifdef EQ_INSTRUMENT_DATAOSTREAM
            nBytesCompressedSent += compressedSize;
#endif
            sendData( _compressor->getName(), nChunks, chunks, chunkSizes,
                      size );
            return;
        }
    }
        
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesCompressedSent += size;
#endif
    sendData( EQ_COMPRESSOR_NONE, 1, &data, &size, size );
}

void DataOStream::_sendFooter( const void* buffer, const uint64_t size )
{
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesSent += size;
#endif

    if( _compressorState != NOT_COMPRESSED )
    {
        const uint32_t nChunks = _compressor->getNumResults( );
        uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                   alloca( nChunks * sizeof( uint64_t )));
        void** chunks = static_cast< void ** >( 
                            alloca( nChunks * sizeof( void* )));
        const uint64_t compressedSize = _getCompressedData( chunks, chunkSizes);

        if( compressedSize < size )
        {
#ifdef EQ_INSTRUMENT_DATAOSTREAM
            nBytesCompressedSent += compressedSize;
#endif
            sendFooter( _compressor->getName(), nChunks, chunks, chunkSizes,
                        size );
            return;
        }
    }

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesCompressedSent += size;
#endif
    sendFooter( EQ_COMPRESSOR_NONE, 1, &buffer, &size, size );
}

void DataOStream::_compress( const void* src, const uint64_t sizeSrc )
{
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesProcessed += sizeSrc;
#endif
    if( !_compressor->isValid( _compressor->getName( )))
    {
        _compressorState = NOT_COMPRESSED;
        return;
    }

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    co::base::Clock clock;
#endif
    const uint64_t inDims[2] = { 0, sizeSrc };

    _compressor->compress( const_cast< void* >( src ), inDims );

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    compressionTime += uint32_t( clock.getTimef() * 1000.f );
    const uint32_t nChunks = _compressor->getNumResults( );

    EQASSERT( nChunks > 0 );
    for( size_t i = 0; i < nChunks; ++i )
    {
        void* chunk;
        uint64_t chunkSize;

        _compressor->getResult(  i, &chunk, &chunkSize );
        nBytesCompressed += chunkSize;
    }
#endif
}

uint64_t DataOStream::_getCompressedData( void** chunks, uint64_t* chunkSizes )
    const
{    
    EQASSERT( _compressorState != NOT_COMPRESSED );
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
       << ": " << nBytes << " bytes total, " << nBytesCompressed  << "/"
       << nBytesProcessed << " compressed in " << compressionTime/1000
       << "ms, sent " << nBytesCompressedSent << "/" << nBytesSent << " bytes";
       
    nBytes = 0;
    nBytesProcessed = 0;
    nBytesCompressed = 0;
    nBytesCompressedSent = 0;
    nBytesSent = 0;
    compressionTime = 0;
#else
       << "@" << (void*)&dataOStream;
#endif
    return os;
}

}
