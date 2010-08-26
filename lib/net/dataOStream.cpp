
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "../base/cpuCompressor.h" // internal header
#include <eq/base/global.h>

//#define EQ_INSTRUMENT_DATAOSTREAM
#ifdef EQ_INSTRUMENT_DATAOSTREAM
#  include <eq/base/clock.h>
#endif

namespace eq
{
namespace net
{

namespace
{
#ifdef EQ_INSTRUMENT_DATAOSTREAM
base::a_int32_t nBytes;
base::a_int32_t nBytesTryToCompress;
base::a_int32_t nBytesUncompressed;
base::a_int32_t nBytesCompressed;
base::a_int32_t timeToCompress;
base::a_int32_t nBytesCompressedSend;
#endif
}

DataOStream::DataOStream()
        : compressor( new base::CPUCompressor )
        , _bufferType( BUFFER_NONE )
        , _bufferStart( 0 )
        , _enabled( false )
        , _dataSent( false )
        , _save( false )
{
    compressor->initCompressor( EQ_COMPRESSOR_DATATYPE_BYTE );
}

DataOStream::~DataOStream()
{
    // Can't call disable() from destructor since it uses virtual functions
    EQASSERT( !_enabled );
    delete compressor;
}

void DataOStream::enable( const Nodes& receivers )
{
#ifdef NDEBUG
    const bool useMulticast = receivers.size() > 1;
#else
    const bool useMulticast = true;
#endif

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

#if 0
    EQLOG( LOG_OBJECTS )
        << "Enabled " << typeid( *this ).name() << " with " << mcSet.size()
        << "/" << _connections.size() << " multicast connections" << std::endl;
#endif
    enable();
}

void DataOStream::enable( NodePtr node, const bool useMulticast )
{
    ConnectionPtr connection = useMulticast ? node->getMulticast() : 0;
    if( !connection )
        connection = node->getConnection();
        
    _connections.push_back( connection );
    enable();
}

void DataOStream::enable()
{
    EQASSERT( !_enabled );
    EQASSERT( _save || !_connections.empty( ));
    _bufferType = BUFFER_NONE;
    _bufferStart = 0;
    _dataSent    = false;
    _enabled     = true;
    _buffer.setSize( 0 );
}

void DataOStream::resend( NodePtr node )
{
    EQASSERT( !_enabled );
    EQASSERT( _connections.empty( ));
    EQASSERT( _save );
    
    ConnectionPtr connection = node->getMulticast();
    if( !connection )
        connection = node->getConnection();
        
    _connections.push_back( connection );
    if ( _bufferType != BUFFER_ALL )
    {
        _bufferType = BUFFER_ALL;
        _compress(  _buffer.getData(), _buffer.getSize() );
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
            if( _bufferType != BUFFER_PARTIAL )
            {
                _bufferType = BUFFER_PARTIAL;
                _compress( ptr, size );
                           
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
            if ( _bufferType != BUFFER_ALL )
            {
                _bufferType = BUFFER_ALL;
                _compress(  _buffer.getData(), _buffer.getSize() );
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
    nBytes += size;
#endif    

    if( _buffer.getSize() - _bufferStart > Global::getObjectBufferSize( ))
        _flush();

    EQASSERT( _enabled );
    _bufferType = BUFFER_NONE;
    _buffer.append( static_cast< const uint8_t* >( data ), size );
}

void DataOStream::_flush()
{
    EQASSERT( _enabled );
    const void* ptr = _buffer.getData() + _bufferStart;
    const uint64_t size = _buffer.getSize() - _bufferStart;

    if( _bufferType != BUFFER_PARTIAL )
    {
        _bufferType = BUFFER_PARTIAL;
        _compress( ptr, size );
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
    _bufferType = BUFFER_NONE;
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

    if( !_connections.empty( ))
    {
        if( _bufferType != BUFFER_NONE )
        {
            const uint32_t nChunks = compressor->getNumResults( );

            uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                    alloca( nChunks * sizeof( uint64_t )));
            void** chunks = static_cast< void ** >( 
                                    alloca( nChunks * sizeof( void* )));

            if( _getCompressedData( chunks, chunkSizes ) < size )
            {
#ifdef EQ_INSTRUMENT_DATAOSTREAM
                nBytesCompressedSend += dataSize;
#endif
                sendData( compressor->getName(), nChunks, chunks, 
                          chunkSizes, size );
                _dataSent = true;
                return;
            }
        }
        
        sendData( EQ_COMPRESSOR_NONE, 1, &data, &size, size );
    }

    _dataSent = true;
}

void DataOStream::_sendFooter( const void* buffer, const uint64_t size )
{
    if( _bufferType != BUFFER_NONE )
    {
        const uint32_t nChunks = compressor->getNumResults( );

        uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                alloca( nChunks * sizeof( uint64_t )));
        void** chunks = static_cast< void ** >( 
                                alloca( nChunks * sizeof( void* )));

        if( _getCompressedData( chunks, chunkSizes ) < size )
        {
#ifdef EQ_INSTRUMENT_DATAOSTREAM
            nBytesCompressedSend += dataSize;
#endif
            sendFooter( compressor->getName(), nChunks, chunks, chunkSizes, size );
            return;
        }
    }
    
    sendFooter( EQ_COMPRESSOR_NONE, 1, &buffer, &size, size );
}

void DataOStream::_compress( const void* src, const uint64_t  sizeSrc )
{
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesTryToCompress += sizeSrc;
#endif
    if ( !compressor->isValid() )
    {
        _bufferType = BUFFER_NONE;
        return;
    }

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesUncompressed += sizeSrc;
    eq::base::Clock clock;
#endif
    const uint64_t inDims[2] = { 0, sizeSrc };

    compressor->compress( const_cast< void* >( src ), inDims );

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    timeToCompress += clock.getTime64();
    const uint32_t nChunks = compressor->getNumResults( );

    EQASSERT( nChunks > 0 );
    for ( size_t i = 0; i < nChunks; i++ )
    {
        void* chunk;
        uint64_t chunkSize;

        compressor->getResult(  i, &chunk, &chunkSize );
        nBytesCompressed += chunkSize;
    }
#endif
}

uint64_t DataOStream::_getCompressedData( void** chunks, uint64_t* chunkSizes )
    const
{    
    EQASSERT( _bufferType != BUFFER_NONE );
    const uint32_t nChunks = compressor->getNumResults( );
    EQASSERT( nChunks > 0 );

    uint64_t dataSize = 0;
    for ( uint32_t i = 0; i < nChunks; i++ )
    {
        compressor->getResult( i, &chunks[i], &chunkSizes[i] );
        dataSize += chunkSizes[i];
    }

    return dataSize;
}

std::ostream& operator << ( std::ostream& os,
                            const DataOStream& dataOStream )
{
    os << base::disableFlush << base::disableHeader << "DataOStream "
#ifdef EQ_INSTRUMENT_DATAOSTREAM
       << ": write data " << nBytes << " bytes was treated. " 
       << nBytesTryToCompress << " was tried to compress but only "
       << nBytesUncompressed << " bytes has been compressed for a size of "  
       << nBytesCompressed  << " bytes after compression and the time compression was "
       << timeToCompress << "[ms]. " 
       << nBytesCompressedSend << " bytes compressed have been send." << std::endl;
       
    nBytes = 0;
    nBytesTryToCompress = 0;
    nBytesCompressed = 0;
    nBytesCompressedSend = 0;
    timeToCompress = 0;
    nBytesCompressedSend = 0;
#else
       << "@" << (void*)&dataOStream;
#endif
    os << base::enableHeader << base::enableFlush;

    return os;
}

}
}
