
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "global.h"
#include "log.h"
#include "node.h"
#include "types.h"

#include <eq/base/compressor.h>

#define EQ_INSTRUMENT_DATAOSTREAM
#ifdef EQ_INSTRUMENT_DATAOSTREAM
#   include <eq/base/clock.h>
#endif
using namespace eq::base;

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
        : _compressor( 0 )
        , _bufferType( BUFFER_NONE )
        , _bufferStart( 0 )
        , _enabled( false )
        , _dataSent( false )
        , _buffered( true )
        , _save( false )
{
#ifdef EQ_COMPRESS_STREAM
    uint32_t name = _chooseCompressor( EQ_COMPRESSOR_DATATYPE_BYTE );
    _initPlugin( name );
    _initCompressor();
#endif

}

DataOStream::~DataOStream()
{
    // Can't call disable() from destructor since it uses virtual functions
    EQASSERT( !_enabled );

    if( getPlugin() && _compressor )
        getPlugin()->deleteCompressor( _compressor );
    
    _compressor = 0;
}

void DataOStream::enable( const NodeVector& receivers )
{
#ifdef NDEBUG
    const bool useMulticast = receivers.size() > 1;
#else
    const bool useMulticast = true;
#endif

    ConnectionDescriptionVector mcSet;

    for( NodeVector::const_iterator i = receivers.begin(); 
         i != receivers.end(); ++i )
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
        
        connection->lockSend();
        _connections.push_back( connection );
    }

    EQLOG( LOG_OBJECTS )
        << "Enabled " << typeid( *this ).name() << " with " << mcSet.size()
        << "/" << _connections.size() << " multicast connections" << std::endl;
    enable();
}

void DataOStream::enable( NodePtr node )
{
    ConnectionPtr connection = node->getMulticast();
    if( !connection )
        connection = node->getConnection();
        
    connection->lockSend();
    _connections.push_back( connection );
    enable();
}

void DataOStream::enable()
{
    EQASSERT( !_enabled );
    _bufferType = BUFFER_NONE;
    _bufferStart = 0;
    _dataSent    = false;
    _buffered    = true;
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
        
    connection->lockSend();
    _connections.push_back( connection );
    if ( _bufferType != BUFFER_ALL )
    {
        _bufferType = BUFFER_ALL;
        _compress(  _buffer.getData(), _buffer.getSize() );
    }
    _sendFooter( _buffer.getData(), _buffer.getSize() );

    _connections.clear();
    connection->unlockSend();
}

void DataOStream::disable()
{
    if( !_enabled )
        return;

    if( _dataSent )
    {
        if( !_connections.empty( ))
        {
            if( _bufferType != BUFFER_PARTIAL )
            {
                _bufferType = BUFFER_PARTIAL;
                _compress( _buffer.getData() + _bufferStart, 
                           _buffer.getSize() - _bufferStart );
            }
            _sendFooter( _buffer.getData() + _bufferStart, 
                         _buffer.getSize() - _bufferStart );
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
    _unlockConnections();
}

void DataOStream::_unlockConnections()
{
    for( ConnectionVector::const_iterator i = _connections.begin(); 
         i != _connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        connection->unlockSend();
    }
    _connections.clear();
}

void DataOStream::enableBuffering()
{
    _buffered = true;
}

void DataOStream::disableBuffering()
{
    if( !_buffered )
        return;

    _buffered = false;
    _flush();
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

    EQASSERT( _enabled );
    if( _buffered || _save )
    {
        _bufferType = BUFFER_NONE;
        _buffer.append( static_cast< const uint8_t* >( data ), size );
    }
    
    if( !_buffered )
    {
        _bufferType = BUFFER_PARTIAL;
        _compress( data, size );   
        _sendBuffer( data, size );
        return;
    }

    if( _buffer.getSize() - _bufferStart > Global::getObjectBufferSize( ))
        _flush();
}

void DataOStream::writeOnce( const void* data, uint64_t size )
{
    EQASSERT( _enabled );
    EQASSERT( !_dataSent );
    EQASSERT( _bufferStart == 0 );
    EQASSERT( _buffer.isEmpty( ));
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytes += size;
#endif    
    
    if( _save )
    {
        _bufferType = BUFFER_NONE;
        _buffer.append( static_cast< const uint8_t* >( data ), size );
    }

    if( !_connections.empty( ))
    {
        _bufferType = BUFFER_ALL;
        _compress( data, size );
        _sendFooter( data, size );
    }
    reset();
    _enabled = false;
    _dataSent = true;
    _unlockConnections();
}

void DataOStream::_flush()
{
    EQASSERT( _enabled );
    if( _bufferType != BUFFER_PARTIAL )
    {
        _bufferType = BUFFER_PARTIAL;
        _compress(  _buffer.getData() + _bufferStart, 
                    _buffer.getSize() - _bufferStart  );
    }
    _sendBuffer( _buffer.getData() + _bufferStart, 
                 _buffer.getSize() - _bufferStart );
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

void DataOStream::_sendBuffer( const void* data, const uint64_t size )
{
    EQASSERT( _enabled );
    if( size == 0 )
        return;

    if( !_connections.empty( ))
    {
        if( _bufferType != BUFFER_NONE && _compressor )
        {
            const uint32_t nChunks = getPlugin()->getNumResults( _compressor, 
                                                          getCompressorName() );

            uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                    alloca( nChunks * sizeof( uint64_t )));
            void** chunks = static_cast< void ** >( 
                                    alloca( nChunks * sizeof( void* )));
            if( _getCompressedData( size, chunks, chunkSizes ) )
            {
                sendBuffer( getCompressorName(), nChunks, 
                            chunks, chunkSizes, size );
                _dataSent = true;
                return;
            }
        }
        
        sendBuffer( EQ_COMPRESSOR_NONE, 1, &data, &size, size );
    }
    _dataSent = true;
}

void DataOStream::_sendFooter( const void* buffer, const uint64_t size )
{
    if( _bufferType != BUFFER_NONE && _compressor )
    {
        const uint32_t nChunks = getPlugin()->getNumResults( _compressor, 
                                                         getCompressorName() );

        uint64_t* chunkSizes = static_cast< uint64_t* >( 
                                alloca( nChunks * sizeof( uint64_t )));
        void** chunks = static_cast< void ** >( 
                                alloca( nChunks * sizeof( void* )));
        if ( _getCompressedData( size, chunks, chunkSizes ) )
        {
            sendFooter( getCompressorName(), nChunks, chunks, 
                        chunkSizes, size );
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
    if( !_compressor )
    {
        _bufferType = BUFFER_NONE;
        return;
    }

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesUncompressed += sizeSrc;
    eq::base::Clock clock;
#endif
    const uint64_t inDims[2] = { 0, sizeSrc };
    getPlugin()->compress( _compressor, getCompressorName(), 
                       const_cast<void*>( src ), 
                       inDims, EQ_COMPRESSOR_DATA_1D );

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    timeToCompress += clock.getTime64();
    const uint32_t nChunks = getPlugin()->getNumResults( _compressor, 
                                                     getCompressorName() );

    EQASSERT( nChunks > 0 );
    for ( size_t i = 0; i < nChunks; i++ )
    {
        void* chunk;
        uint64_t chunkSize;
        getPlugin()->getResult( _compressor, getCompressorName(), i, 
                                &chunk, &chunkSize );
        nBytesCompressed += chunkSize;
    }
#endif
}

bool DataOStream::_getCompressedData( uint64_t sizeUncompressed, 
                                      void** chunks, 
                                      uint64_t* chunkSizes )
{    
    const uint32_t nChunks = getPlugin()->getNumResults( _compressor, 
                                                     getCompressorName() );
    EQASSERT( nChunks > 0 );

    uint64_t dataSize = 0;
    for ( uint32_t i = 0; i < nChunks; i++ )
    {
        getPlugin()->getResult( _compressor, getCompressorName(), i,  
                                &chunks[i], &chunkSizes[i] );
        dataSize += chunkSizes[i];
    }

    if( dataSize >= sizeUncompressed )
        return false;

#ifdef EQ_INSTRUMENT_DATAOSTREAM
    nBytesCompressedSend += sizeUncompressed;
#endif

    return true;
}

uint32_t DataOStream::_chooseCompressor( const uint32_t tokenType )
{
    uint32_t name = EQ_COMPRESSOR_NONE;
    float ratio = 1.0f;
    
    EQINFO << "Searching compressor for token type " << tokenType << std::endl;
    
    const PluginRegistry& registry = eq::base::Global::getPluginRegistry();
    const CompressorVector& compressors = registry.getCompressors();

    for( CompressorVector::const_iterator i = compressors.begin();
        i != compressors.end(); ++i )
    {
        const Compressor* compressor = *i;
        const CompressorInfoVector& infos = compressor->getInfos();
        
        EQINFO << "Searching in DSO " << (void*)compressor << std::endl;
        
        for( CompressorInfoVector::const_iterator j = infos.begin();
            j != infos.end(); ++j )
        {
            const EqCompressorInfo& info = *j;
            if( info.tokenType != tokenType )
                continue;
            
            if( ratio > info.ratio ) // TODO: be smarter
            {
                name  = info.name;
                ratio = info.ratio;
            }
        }
    }

    EQINFO << "Selected compressor " << name << std::endl;

    return name;
}

void DataOStream::_initCompressor()
{
    if( getCompressorName() != EQ_COMPRESSOR_NONE )
    {
        EQASSERT( !_compressor );
        _compressor = getPlugin()->newCompressor( getCompressorName() );
    }
}

std::ostream& operator << ( std::ostream& os,
                            const DataOStream& dataOStream )
{
    os << base::disableFlush << base::disableHeader << "DataOStream ";
#ifdef EQ_INSTRUMENT_DATAOSTREAM
    os << ": write data " << nBytes << " bytes was treated. " 
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
#endif
    os << base::enableHeader << base::enableFlush;

    return os;
}
}
}
